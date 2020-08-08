#include "sensor_bme280.h"

#include <pigpiod_if2.h>

SensorBME280::SensorBME280(int address) : _address(address) {
    _pi = -1;
    _handle = -1;
}

int SensorBME280::Open(const Config& config) {
    uint32_t flags = 0;
    const uint32_t i2cBus =1;

    _pi = pigpio_start(NULL, NULL);
    _handle = i2c_open(_pi, i2cBus, _address, flags);
    if(_handle < 0 ) {
        Close();
        return -1;
    }

    const uint8_t ctrlMeasReg = (config.osrsT << 5) | (config.osrsP << 2) | config.mode;
    const uint8_t configReg = (config.tSb << 5) | (config.filter << 2) | config.spi3wEn;
    const uint8_t ctrlHumReg = config.osrsH;

    i2c_write_byte_data(_pi, _handle, 0xF2, ctrlHumReg);
    i2c_write_byte_data(_pi, _handle, 0xF4, ctrlMeasReg);
    i2c_write_byte_data(_pi, _handle, 0xF5, configReg);

    if ( ReadTrimmingData() < 0 ) {
        Close();
        return -2;
    }

    return 0;
}

int SensorBME280::Close() {
    if( _handle >= 0) {
        i2c_close(_pi, _handle);
        _handle = -1;
    }
    if( _pi >= 0 ) {
        pigpio_stop(_pi);
        _pi = -1;
    }
    return 0;
}

// 計測を行う.
int SensorBME280::Measure() {
    uint8_t data[8] = { 0 };
    uint32_t press = 0;
    int32_t temp = 0;
    int32_t hum = 0;
   
    for(int i=0; i<8; i++) {
        int value = i2c_read_byte_data(_pi, _handle, 0xF7 + i );
        if( value < 0 ) {
            return -1;
        }
        data[i] = value;
    }

    press = (data[0] << 12) | (data[1] << 4) | (data[2] >> 4);
    temp = (data[3] << 12) | (data[4] << 4) | (data[5] >> 4);
    hum = (data[6] << 8) | (data[7]);

    // 各値の補正.
    _temperature = CompensateT(temp);
    _pressure = CompensateP(press);
    _humidity = CompensateH(hum);

    return 0;
}

float SensorBME280::GetTemperature() const {
    return _temperature / 100.0f;
}

float SensorBME280::GetPressure() const {
    return _pressure / 100.0f;
}

float SensorBME280::GetHumidity() const {
    return _humidity / 1024.0f;
}

int SensorBME280::GetTemperatureAsInt() const {
    return _temperature;
}

int SensorBME280::GetPressureAsInt() const {
    return _pressure;
}

int SensorBME280::GetHumidityAsInt() const {
    return _humidity;
}

int32_t SensorBME280::CompensateT(int32_t adcTemp) {
    const auto t1 = (int32_t)_trimParam.dig_T1;
    const auto t2 = (int32_t)_trimParam.dig_T2;
    const auto t3 = (int32_t)_trimParam.dig_T3;
    int32_t var1, var2, T;
    int32_t w = (adcTemp>>4) - t1;

    var1 = ((((adcTemp >> 3) - (t1<<1))) * t2) >> 11;
    var2 = (((w * w) >> 12) * t3) >> 14;
    _t_fine = var1 + var2;
    T = (_t_fine * 5 + 128) >> 8;
    return T;
}

// 8.2 Pressure compensation in 32 bit fixed point.
int32_t SensorBME280::CompensateP(int32_t adcPress) {
    const auto p1 = (int32_t)_trimParam.dig_P1;
    const auto p2 = (int32_t)_trimParam.dig_P2;
    const auto p3 = (int32_t)_trimParam.dig_P3;
    const auto p4 = (int32_t)_trimParam.dig_P4;
    const auto p5 = (int32_t)_trimParam.dig_P5;
    const auto p6 = (int32_t)_trimParam.dig_P6;
    const auto p7 = (int32_t)_trimParam.dig_P7;
    const auto p8 = (int32_t)_trimParam.dig_P8;
    const auto p9 = (int32_t)_trimParam.dig_P9;
    int32_t var1, var2;
    uint32_t p;

    var1= (_t_fine>>1) - 64000;
    var2 = ((var1 >>2) * (var1 >>2) >> 11) * p6;
    var2 = var2 + ((var1*p5)<<1);
    var2 = (var2>>2)+(p4<<16);
    var1 = (((p3 * (((var1>>2) * (var1>>2)) >> 13 )) >> 3) + ((p2 * var1)>>1))>>18;
    var1 =((((32768+var1))*p1)>>15);
    if (var1 == 0)
    {
        return 0; // avoid exception caused by division by zero
    }
    p = (((uint32_t)(1048576-adcPress)-(var2>>12)))*3125;
    if (p < 0x80000000)
    {
        p = (p << 1) / ((uint32_t)var1);
    }
    else
    {
        p = (p / (uint32_t)var1) * 2;
    }
    var1 = ((p9) * ((int32_t)(((p>>3) * (p>>3))>>13)))>>12;
    var2 = (((int32_t)(p>>2)) * (p8))>>13;
    p = (uint32_t)((int32_t)p + ((var1 + var2 + p7) >> 4));
    return p;
}

int32_t SensorBME280::CompensateH(int32_t adcHum) {
    const auto h1 = (int32_t)_trimParam.dig_H1;
    const auto h2 = (int32_t)_trimParam.dig_H2;
    const auto h3 = (int32_t)_trimParam.dig_H3;
    const auto h4 = (int32_t)_trimParam.dig_H4;
    const auto h5 = (int32_t)_trimParam.dig_H5;
    const auto h6 = (int32_t)_trimParam.dig_H6;

    int32_t h;
    h = _t_fine - 76800;
    h = (((((adcHum << 14) - (h4 << 20) - (h5 * h)) + 16384) >> 15) * (((((((h * h6) >> 10) * (((h * h3) >> 11) + 32768)) >> 10) + (2097152)) * h2 + 8192) >> 14));
    h = (h - (((((h >> 15) * (h >> 15)) >> 7) * h1) >> 4));
    h = (h < 0 ? 0 : h);
    h = (h > 419430400 ? 419430400 : h);
    return ((uint32_t)h) >> 12;
}

int SensorBME280::ReadTrimmingData() {
    uint8_t data[32] = { 0 };
    int ofs = 0;
    int value = 0;
    for(int  i=0; i<24; i++, ofs++) {
        value = i2c_read_byte_data(_pi, _handle, 0x88 + i);
        if( value < 0 ) {
            return -1;
        }
        data[ofs] = value;
    }
    value = i2c_read_byte_data(_pi, _handle, 0xA1);
    if( value < 0 ) {
        return -1;
    }
    data[ofs++] = value;

    for(int  i=0; i<7; i++, ofs++) {
        value = i2c_read_byte_data(_pi, _handle, 0xE1 + i);
        if( value < 0 ) {
            return -1;
        }
        data[ofs] = value;
    }

    _trimParam.dig_T1 = (data[1] << 8) | data[0];
    _trimParam.dig_T2 = (data[3] << 8) | data[2];
    _trimParam.dig_T3 = (data[5] << 8) | data[4];

    _trimParam.dig_P1 = (data[7] << 8) | data[6];
    _trimParam.dig_P2 = (data[9] << 8) | data[8];
    _trimParam.dig_P3 = (data[11] << 8) | data[10];
    _trimParam.dig_P4 = (data[13] << 8) | data[12];
    _trimParam.dig_P5 = (data[15] << 8) | data[14];
    _trimParam.dig_P6 = (data[17] << 8) | data[16];
    _trimParam.dig_P7 = (data[19] << 8) | data[18];
    _trimParam.dig_P8 = (data[21] << 8) | data[20];
    _trimParam.dig_P9 = (data[23] << 8) | data[22];

    _trimParam.dig_H1 = data[24];
    _trimParam.dig_H2 = (data[26] << 8) | data[25];
    _trimParam.dig_H3 = data[27];
    _trimParam.dig_H4 = (data[28] << 4) | (data[29] & 0x0F);
    _trimParam.dig_H5 = (data[30] << 4) | ((data[29] >> 4) & 0x0F);
    _trimParam.dig_H6 = data[31];

    return 0;
}

