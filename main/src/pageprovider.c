#include "pageprovider.h"
#include <esp_system.h>
#include <string.h>
#include <esp_log.h>
#include <sys/param.h>
#include "esp_spiffs.h"
#include <stdbool.h>

_Bool READY = false;

static const char *TAG="PAGEPROVIDER";

//Will use NULL as partition label -> First SPIFFS to be found will be used. 
const esp_vfs_spiffs_conf_t CONF = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = true
    };

esp_err_t mountSPIFFS(){
    ESP_LOGI(TAG, "Mounting SPIFFS...");

    //use CONF to initialize and mount SPIFFS filesystem.
    // Note: esp_vfs_spiffs_register is an all-in-one convenience function.
    esp_err_t ret = esp_vfs_spiffs_register(&CONF);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }

    if(ret == ESP_OK){
        READY = true;
        ESP_LOGI(TAG, "SPIFFS mounted successfully.");
    }

    return ret;
}

esp_err_t unmountSPIFFS(){
    esp_err_t ret = esp_vfs_spiffs_unregister(NULL);
    if(ret == ESP_OK){
        ESP_LOGI(TAG, "SPIFFS unmounted successfully.");
        READY = false;
    } else{
        ESP_LOGW(TAG, "SPIFFS coundn't be unmounted");
    }
    return ret;
}

//provides a pointer to the page using malloc
char * getPage(char* path) 
{
    // If it hasn't been mounted yet, do it
    if(!READY && mountSPIFFS() != ESP_OK){
        ESP_LOGE(TAG, "Couldn't mount SPIFFS partition. Returning ERROR to Website");
        return "<h1>ERROR: Couldn't mount SPIFFS partition.<h1>";
    }
    
    //Opening the File
    ESP_LOGI(TAG, "Reading file");
    FILE* f = fopen(path, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return 0;
    }
    
    fseek(f, 0, SEEK_END);
    long unsigned fsize = ftell(f);
    fseek(f, 0, SEEK_SET);  /* same as rewind(f); */

    char* string = malloc (fsize + 1);
    fread(string, 1, fsize, f);
    fclose(f);

    string[fsize] = 0;
    ESP_LOGI(TAG, "Read file \"%s\", length:%lu", path, fsize);    
    return string;
}