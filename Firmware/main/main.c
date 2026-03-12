#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "wifi.h"
#include "http.h"
#include "vSignalTask.h"

void app_main(void)
{
    start_wifi();

    EventGroupHandle_t wifiEventGroup = get_wifi_event_group();
    EventBits_t uxBits = xEventGroupWaitBits(wifiEventGroup, WIFI_CONNECTED_BIT, pdFALSE, pdTRUE, portMAX_DELAY);

    static ContextHttp httpContext;

    start_http(&httpContext);

    if (uxBits & WIFI_CONNECTED_BIT)
    {
        xTaskCreate(vSignalTask, "Task_Signal", 4096, (void *)&httpContext, 5, NULL);
    }
}