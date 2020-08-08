#ifndef BME280_H
#define BME280_H

#include <stdio.h>
#include <stdint.h>

class SensorBME280 {
public:
    static const int DEFAULT_ADDRESS = 0x76;
    struct Config {
        uint8_t osrsT;   /* Temperature oversampling x 1*/
        uint8_t osrsP;   /* Pressure oversampling x 1 */
        uint8_t osrsH;   /* Humidity oversampling x 1 */
        uint8_t mode;    /* Normal mode */
        uint8_t tSb;     /* Tstandby 1000ms */
        uint8_t filter;  /* Filter off */
        uint8_t spi3wEn; /* 3-wire SPI Disable */

        Config() : 
            osrsT(1), osrsP(1), osrsH(1), 
            mode(3), tSb(5), filter(0), spi3wEn(0) {
        }
    };

    SensorBME280(int address = DEFAULT_ADDRESS);

    // センサーとの接続を開く.
    //  正常終了: 0
    //  pigpiod との接続失敗: -1
    //  校正データ取得に失敗 : -2
    int Open(const Config& config);

    // センサーとの接続を終了.
    //  正常終了: 0
    int Close();

    // 計測を行う.
    //  正常終了: 0
    int Measure();

    // 温度の取得. 単位は℃.
    float GetTemperature() const;

    // 気圧の取得. 単位は hPa.
    float GetPressure() const;

    // 湿度の取得. 単位は %.
    float GetHumidity() const;

    // 温度の取得.
    //   温度 = retVal / 100.0
    int GetTemperatureAsInt() const;

    // 気圧の取得.
    //   気圧 = retVal / 100.0
    int GetPressureAsInt() const;

    // 湿度の取得.
    //   湿度 = retVal / 1024.0
    int GetHumidityAsInt() const;

private:
    int32_t CompensateT(int32_t adcTemp);
    int32_t CompensateP(int32_t adcPress);
    int32_t CompensateH(int32_t adcHum);
    int ReadTrimmingData();

    uint64_t _pressure;
    int32_t  _temperature;
    int32_t  _humidity;
    int _address;
    int _t_fine;

    // pigpio
    int _handle;
    int _pi;

    // BST-BME280_DS001-10.pdf  4.2.2 Triming parameter readout より.
    struct TrimmingParameters {
        uint16_t dig_T1;
        int16_t dig_T2;
        int16_t dig_T3;

        uint16_t dig_P1;
        int16_t dig_P2;
        int16_t dig_P3;
        int16_t dig_P4;
        int16_t dig_P5;
        int16_t dig_P6;
        int16_t dig_P7;
        int16_t dig_P8;
        int16_t dig_P9;

        uint8_t dig_H1;
        int16_t dig_H2;
        uint8_t dig_H3;
        int16_t dig_H4;
        int16_t dig_H5;
        int8_t  dig_H6;
    };
    TrimmingParameters _trimParam;
};

#endif
