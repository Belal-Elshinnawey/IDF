set(component_srcs "src/ota_util.c")

idf_component_register(
                       SRCS "${component_srcs}"
                       INCLUDE_DIRS "include"
                       REQUIRES nvs_flash app_update esp_https_ota esp_http_client) 



