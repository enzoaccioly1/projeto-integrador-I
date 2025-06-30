#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>
#include <sdkconfig.h>
#include <esp_rmaker_core.h>
#include <esp_rmaker_standard_types.h>
#include <esp_rmaker_standard_params.h>
#include "math.h"

#include <app_reset.h>
#include <iot_button.h>

#include <dht.h>

#include <string.h>
#include <bmp280.h>

#include "esp_adc/adc_oneshot.h"
#include "hal/adc_types.h"

#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_cali.h"

#include "app_priv.h"

// RELÊ
#define BUTTON_GPIO 0
#define BUTTON_ACTIVE_LEVEL 0
#define RELAY_GPIO 2

// DHT11
#define SENSOR_TYPE 0
#define CONFIG_EXAMPLE_DHT11_GPIO 17 // TX2

// BMP280
#define CONFIG_EXAMPLE_I2C_MASTER_SDA 21
#define CONFIG_EXAMPLE_I2C_MASTER_SCL 22

#define WIFI_RESET_BUTTON_TIMEOUT 3
#define FACTORY_RESET_BUTTON_TIMEOUT 10

static TimerHandle_t sensor_timer;

bmp280_t dev;
bmp280_params_t params;

adc_cali_handle_t cali_handle = NULL;
adc_oneshot_unit_handle_t adc_handle = NULL;
int LDR_read, LDR_output;

static float g_temperature;
static float g_humidity;
static bool g_power_state = DEFAULT_POWER;
static float g_pressure;

// DHT11, BMP280

static void app_sensor_update(TimerHandle_t handle)
{

    if (bmp280_read_float(&dev, &g_temperature, &g_pressure, &g_humidity) == ESP_OK)
    {
        printf("Pressao:  %.1f Pa\n", g_pressure);
    }
    else
    {
        printf("Nao foi possivel ler dados do sensor BMP280\n");
    }

    if (dht_read_float_data(SENSOR_TYPE, CONFIG_EXAMPLE_DHT11_GPIO, &g_humidity, NULL) == ESP_OK)
    {
        printf("Umidade: %.1f%% Temperatura: %.1fC\n", g_humidity, g_temperature);
    }
    else
    {
        printf("Nao foi possivel ler dados do sensor DHT11\n");
    }

    ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, ADC_CHANNEL_0, &LDR_read));
    printf("LDR (ADC): %d \n", LDR_read);

    adc_cali_raw_to_voltage(cali_handle, LDR_read, &LDR_output);
    printf("LDR (Tensao): %d mV\n", LDR_output);

    esp_rmaker_param_update_and_report(
        esp_rmaker_device_get_param_by_type(temp_sensor_device, ESP_RMAKER_PARAM_TEMPERATURE),
        esp_rmaker_float(g_temperature));

    esp_rmaker_param_update_and_report(
        esp_rmaker_device_get_param_by_type(humidity_sensor_device, ESP_RMAKER_PARAM_HUMIDITY),
        esp_rmaker_float(g_humidity));

    esp_rmaker_param_update_and_report(
        esp_rmaker_device_get_param_by_type(pressure_sensor_device, ESP_RMAKER_PARAM_PRESSURE),
        esp_rmaker_float(g_pressure));

    esp_rmaker_param_update_and_report(
        esp_rmaker_device_get_param_by_type(LDR_device, ESP_RMAKER_PARAM_LUMINOSITY),
        esp_rmaker_float(LDR_output));
}

esp_err_t app_sensor_init()
{
    sensor_timer = xTimerCreate("app_sensor_update_tm", (REPORTING_PERIOD * 1000) / portTICK_PERIOD_MS,
                                pdTRUE, NULL, app_sensor_update);
    if (sensor_timer)
    {
        xTimerStart(sensor_timer, 0);
        return ESP_OK;
    }
    return ESP_FAIL;
}

// RELÊ

static void set_power_state(bool target)
{
    gpio_set_level(RELAY_GPIO, target);
}

int app_driver_set_state(bool state)
{
    if (g_power_state != state)
    {
        g_power_state = state;
        set_power_state(!g_power_state); // relê é ativo em 0
    }
    return ESP_OK;
}

static void push_btn_cb()
{
    bool new_state = !g_power_state;
    app_driver_set_state(new_state);

    esp_rmaker_param_update_and_report(
        esp_rmaker_device_get_param_by_name(relay_device, ESP_RMAKER_DEF_POWER_NAME),
        esp_rmaker_bool(new_state));
}

void app_driver_init()
{
    ESP_ERROR_CHECK(i2cdev_init());

    // LDR
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };

    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };

    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, ADC_CHANNEL_0, &config)); // GPIO36 (VP)

    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_2,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
        .default_vref = 1100,
    };

    ESP_ERROR_CHECK(adc_cali_create_scheme_line_fitting(&cali_config, &cali_handle));

    // RELÊ
    gpio_set_direction(RELAY_GPIO, GPIO_MODE_OUTPUT);

    button_handle_t btn_handle = iot_button_create(BUTTON_GPIO, BUTTON_ACTIVE_LEVEL);

    if (btn_handle)
    {
        iot_button_set_evt_cb(btn_handle, BUTTON_CB_TAP, push_btn_cb, NULL);

        app_reset_button_register(btn_handle, WIFI_RESET_BUTTON_TIMEOUT, FACTORY_RESET_BUTTON_TIMEOUT);
    }

    // SENSORES
    bmp280_init_default_params(&params);
    memset(&dev, 0, sizeof(bmp280_t));

    ESP_ERROR_CHECK(bmp280_init_desc(&dev, BMP280_I2C_ADDRESS_0, 0, CONFIG_EXAMPLE_I2C_MASTER_SDA, CONFIG_EXAMPLE_I2C_MASTER_SCL));
    ESP_ERROR_CHECK(bmp280_init(&dev, &params));

    app_sensor_init();
}

float app_get_current_temperature()
{
    return g_temperature;
}

float app_get_current_humidity()
{
    return g_humidity;
}

float app_get_current_pressure()
{
    return g_pressure;
}

int app_get_current_luminosity()
{
    return LDR_output;
}

bool app_driver_get_state()
{
    return g_power_state;
}
