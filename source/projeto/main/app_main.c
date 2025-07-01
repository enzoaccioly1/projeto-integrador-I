#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <nvs_flash.h>

#include <esp_rmaker_core.h>
#include <esp_rmaker_standard_params.h>
#include <esp_rmaker_standard_devices.h>
#include <esp_rmaker_standard_types.h>
#include <portmacro.h>

#include <app_network.h>
#include <app_insights.h>

#include "app_priv.h"

static const char *TAG = "app_main";

// DISPOSITIVOS
esp_rmaker_device_t *temp_sensor_device;
esp_rmaker_device_t *humidity_sensor_device;
esp_rmaker_device_t *pressure_sensor_device;
esp_rmaker_device_t *relay_device;
esp_rmaker_device_t *LDR_device;

esp_rmaker_device_t *esp_rmaker_graph_device_create(const char *dev_name, const char *param_name, const char *param_type, float val);
esp_rmaker_param_t *esp_rmaker_graph_param_create(const char *param_name, const char *param_type, float val);

// função callback para lidar com os comandos da nuvem
static esp_err_t write_cb(const esp_rmaker_device_t *device, const esp_rmaker_param_t *param,
                          const esp_rmaker_param_val_t val, void *priv_data, esp_rmaker_write_ctx_t *ctx)
{
    if (ctx)
    {
        ESP_LOGI(TAG, "Received write request via : %s", esp_rmaker_device_cb_src_to_str(ctx->src));
    }

    if (strcmp(esp_rmaker_param_get_name(param), ESP_RMAKER_DEF_POWER_NAME) == 0)
    {
        ESP_LOGI(TAG, "Received value = %s for %s - %s",
                 val.val.b ? "true" : "false", esp_rmaker_device_get_name(device),
                 esp_rmaker_param_get_name(param));

        app_driver_set_state(val.val.b);
        esp_rmaker_param_update(param, val);
    }
    return ESP_OK;
}

void app_main()
{
    app_driver_init();

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    app_network_init();

    esp_rmaker_config_t rainmaker_cfg = {
        .enable_time_sync = false,
    };

    esp_rmaker_node_t *node = esp_rmaker_node_init(&rainmaker_cfg, "Dispositivos", "Sensores: DHT11, BMP280, LDR; Outros (relê): G3MB-202P");
    if (!node)
    {
        ESP_LOGE(TAG, "Could not initialise node. Aborting!!!");
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        abort();
    }

    temp_sensor_device = esp_rmaker_temp_sensor_device_create("Temperatura (°C)", NULL, app_get_current_temperature());
    esp_rmaker_node_add_device(node, temp_sensor_device);

    humidity_sensor_device = esp_rmaker_graph_device_create("Umidade (%)", "Umidade", ESP_RMAKER_PARAM_HUMIDITY, app_get_current_humidity());
    esp_rmaker_node_add_device(node, humidity_sensor_device);

    pressure_sensor_device = esp_rmaker_graph_device_create("Pressão (Pa)", "Pressão", ESP_RMAKER_PARAM_PRESSURE, app_get_current_pressure());
    esp_rmaker_node_add_device(node, pressure_sensor_device);

    LDR_device = esp_rmaker_graph_device_create("Luminosidade (mV)", "Luminosidade", ESP_RMAKER_PARAM_LUMINOSITY, app_get_current_luminosity());
    esp_rmaker_node_add_device(node, LDR_device);

    relay_device = esp_rmaker_switch_device_create("Relê", NULL, 0);
    esp_rmaker_device_add_cb(relay_device, write_cb, NULL);
    esp_rmaker_node_add_device(node, relay_device);

    esp_rmaker_ota_enable_default();

    app_insights_enable();

    esp_rmaker_start();

    app_network_set_custom_pop("12345678"); // senha BLE

    err = app_network_start(POP_TYPE_CUSTOM);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Could not start Wifi. Aborting!!!");
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        abort();
    }
}

esp_rmaker_device_t *esp_rmaker_graph_device_create(const char *dev_name, const char *param_name, const char *param_type, float val)
{
    esp_rmaker_device_t *device = esp_rmaker_device_create(dev_name, NULL, NULL);
    if (device)
    {
        esp_rmaker_device_add_param(device, esp_rmaker_name_param_create(ESP_RMAKER_DEF_NAME_PARAM, dev_name));
        esp_rmaker_param_t *primary = esp_rmaker_graph_param_create(param_name, param_type, val);
        esp_rmaker_device_add_param(device, primary);
        esp_rmaker_device_assign_primary_param(device, primary);
    }
    return device;
}

esp_rmaker_param_t *esp_rmaker_graph_param_create(const char *param_name, const char *param_type, float val)
{
    esp_rmaker_param_t *param = esp_rmaker_param_create(param_name, param_type,
                                                        esp_rmaker_float(val), PROP_FLAG_READ | PROP_FLAG_TIME_SERIES);
    if (param)
    {
        esp_rmaker_param_add_ui_type(param, ESP_RMAKER_UI_TEXT);
    }
    return param;
}