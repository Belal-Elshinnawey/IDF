#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_system.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_https_ota.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "nvs_flash.h"

#pragma once
#ifdef __cplusplus
extern "C" {
#endif
void ota_init_loop();
#define OTA_URL_FW_BIN "http://fileshare.wlan/get_ESP_data.php?file=aircontroller/current/IDF/ESP_OTA_Template.bin"
#define OTA_SSID "ESP_UPLOADER"
#define OTA_PASSWORD "EPLOAD19"
#define OTA_MAX_WIFI_RETRY 5
static EventGroupHandle_t s_wifi_event_group;
static bool OTA_VALID_IMG;
static int8_t OTA_WIFI_RETRY;
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
#ifdef __cplusplus
}
#endif