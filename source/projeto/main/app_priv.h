#pragma once
#include <stdint.h>
#include <stdbool.h>

#define ESP_RMAKER_PARAM_HUMIDITY       "esp.param.humidity"
#define ESP_RMAKER_PARAM_PRESSURE       "esp.param.pressure"
#define ESP_RMAKER_PARAM_LUMINOSITY     "esp.param.luminosity"

#define REPORTING_PERIOD 20 // segundos (4*60*24*4 = 23040 < 25000 (limite))
#define DEFAULT_POWER false

extern esp_rmaker_device_t *temp_sensor_device;
extern esp_rmaker_device_t *humidity_sensor_device;
extern esp_rmaker_device_t *pressure_sensor_device;
extern esp_rmaker_device_t *relay_device;
extern esp_rmaker_device_t *LDR_device;

void app_driver_init(void);
int app_driver_set_state(bool state);
float app_get_current_temperature();
float app_get_current_humidity();
float app_get_current_pressure();
int app_get_current_luminosity();
bool app_driver_get_state();
