#include <stdio.h>
#include "sensor_bme280.h"

int main(int argc, char* argv[]) {
    SensorBME280 bme280;
    SensorBME280::Config config;
    int ret = bme280.Open(config);
    if( ret < 0 ) {
        fprintf(stderr, "bme280 open failed.\n");
        return -1;
    }
    ret = bme280.Measure();
    if( ret < 0 ) {
        fprintf(stderr, "bme280 measure failed.\n");
        return -1;
    }

    float t = bme280.GetTemperature();
    float p = bme280.GetPressure();
    float h = bme280.GetHumidity();
    int32_t it = bme280.GetTemperatureAsInt();
    int32_t ip = bme280.GetPressureAsInt();
    int32_t ih = bme280.GetHumidityAsInt();

    printf("BME280 Result:\n");
    printf("  Temperature: %.2f (%d)\n", t, it);
    printf("  Pressure: %.2f (%d)\n", p, ip);
    printf("  Humidity: %.2f (%d)\n", h, ih);

    bme280.Close();
    return 0;
}
