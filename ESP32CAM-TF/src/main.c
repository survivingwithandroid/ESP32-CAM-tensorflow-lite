
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "app_camera.h"
#include "app_webserver.h"
#include "app_machine_learning.h"


#define SSID "your_wifi_ssid"
#define PWD "your_wifi_pwd"

const char* TAG ="esp32ML";

static EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;

void wifi_connect(){
  ESP_LOGI(TAG, "Wifi Connect2");
    wifi_config_t cfg = {
        .sta = {
            .ssid = SSID,
            .password = PWD,
        },
    };
    ESP_ERROR_CHECK( esp_wifi_disconnect() );
    ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &cfg) );
    ESP_ERROR_CHECK( esp_wifi_connect() );
}

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        break;
    default:
        break;
    }
    
    return ESP_OK;
}

static void init_wifi(void) {
  ESP_LOGI(TAG, "Init wifi...");
  tcpip_adapter_init();
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);
  esp_wifi_set_mode(WIFI_MODE_STA);
  esp_wifi_start();
  printf("Connecting to %s\n", SSID);
  //wifi_connect();
}

void main_task(void) {
  init_wifi();
  printf("Waiting for WiFi connection....");
  xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);
  printf("WiFi connected");

  tcpip_adapter_ip_info_t ip_info;
	ESP_ERROR_CHECK(tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ip_info));
	printf("IP Address:  %s\n", ip4addr_ntoa(&ip_info.ip));

init_camera();
startCameraServer();
//init_ml_module();
 
}

void app_main(){

  // Start Wifi Connection
  esp_log_level_set("wifi", ESP_LOG_NONE);

  // initialize NVS
  ESP_ERROR_CHECK(nvs_flash_init());

  wifi_event_group = xEventGroupCreate();

  ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
  main_task();
}

