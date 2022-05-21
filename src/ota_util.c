#include "ota_util.h"

void download_update(void){
    esp_http_client_config_t ota_client_config = {
      .url = OTA_URL_FW_BIN,
      .cert_pem = NULL,
  };
  esp_err_t ret = esp_https_ota(&ota_client_config);
  if (ret == ESP_OK){
    esp_restart();
  } else {
    OTA_VALID_IMG = true;
  }
}


static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
  {
    esp_wifi_connect();
  }
  else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
  {
    if (OTA_WIFI_RETRY < OTA_MAX_WIFI_RETRY) {
      esp_wifi_connect();
      OTA_WIFI_RETRY++;
    } else {
        OTA_VALID_IMG = false;
    }
  }
  else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
  {
    OTA_WIFI_RETRY = 0;
    xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
  }
}


void wifi_init_sta(void)
{
  s_wifi_event_group = xEventGroupCreate();
  esp_netif_create_default_wifi_sta();
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  esp_event_handler_instance_t instance_start;
  esp_event_handler_instance_t instance_disconnect;
  esp_event_handler_instance_t instance_got_ip;
  ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_STA_START, &wifi_event_handler, NULL, &instance_start));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &wifi_event_handler, NULL, &instance_disconnect));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &instance_got_ip));
  wifi_config_t wifi_config = {};
  strcpy((char *)wifi_config.sta.ssid, OTA_SSID);
  strcpy((char *)wifi_config.sta.password, OTA_PASSWORD);
  wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
  wifi_config.sta.pmf_cfg.capable = true;
  wifi_config.sta.pmf_cfg.required = false;
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());
  /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
  * number of re-tries (WIFI_FAIL_BIT). The bits are set by wifi_event_handler() (see above) */
  EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
  /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
  * happened. */
  if (bits & WIFI_CONNECTED_BIT) {
    download_update();
  } else if (bits & WIFI_FAIL_BIT) {
  } 
  /* The event will not be processed after unregister */
  ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, WIFI_EVENT_STA_START, instance_start));
  ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, instance_disconnect));
  ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
  vEventGroupDelete(s_wifi_event_group);
}

void validate_and_install(void){

  const esp_partition_t *running = esp_ota_get_running_partition();
  esp_ota_img_states_t ota_state;
    if (OTA_VALID_IMG && esp_ota_get_state_partition(running, &ota_state) == ESP_OK) {
        esp_ota_mark_app_valid_cancel_rollback();
    }
    else {
        esp_ota_mark_app_invalid_rollback_and_reboot();
    }
}
 
void ota_init_loop() {
  OTA_VALID_IMG = false;
  OTA_WIFI_RETRY = 0;
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);
  ESP_ERROR_CHECK(esp_netif_init());

  vTaskDelay(pdMS_TO_TICKS(1000));
  //Configure WiFi connection
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  wifi_init_sta();
  vTaskDelay(pdMS_TO_TICKS(1000));

  while (!OTA_VALID_IMG && OTA_WIFI_RETRY < OTA_MAX_WIFI_RETRY)
  {
      vTaskDelay(pdMS_TO_TICKS(1000));
  }

  validate_and_install();
  esp_wifi_disconnect();
  esp_wifi_stop();
  esp_wifi_deinit();
  nvs_flash_deinit();
}