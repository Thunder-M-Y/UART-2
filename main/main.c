#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "string.h"
#include "esp_console.h"
#include "linenoise/linenoise.h"
#include "sdkconfig.h"

#define UART_TX_PIN 1
#define UART_RX_PIN 2
#define BUF_SIZE (1024) // 缓冲区大小
#define UART_PORT_NUM UART_NUM_2
const char *TAG = "UART";

void pc_interaction()
{
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, UART_TX_PIN, UART_RX_PIN, -1, -1));

    char *data = (char *)pvPortMalloc(BUF_SIZE);
    char msg[BUF_SIZE] = ""

        ESP_LOGI(TAG, "串口初始化成功，等待接收数据...");
    while (1)
    {
        int len = uart_read_bytes(UART_PORT_NUM, data, (BUF_SIZE - 1), pdMS_TO_TICKS(20)); // 读取对应长度的数据
        if (len)
        {
            if (strcmp(data, "quit") == 0)
            {
                ESP_LOGI(TAG, "退出串口...");
                break;
            }
            data[len] = 0;
            ESP_LOGI(TAG, "收到消息，长度 %d 内容: %s", len, data);
            memset(msg, 0, strlen(msg));
            strcat(msg, "RECV:");
            strcat(msg, data);
            strcat(msg, "\n");
            ESP_LOGI(TAG, "回复消息，长度 %d 内容 %s", strlen(msg), msg);
            uart_write_bytes(UART_PORT_NUM, msg, strlen(msg));
        }
    }
    vPortFree(data);
    uart_driver_delete(UART_PORT_NUM);
    vTaskDelete(NULL);
}

/**
 * @brief 聊天器接收线程
 */

static void recipient()
{
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, UART_TX_PIN, UART_RX_PIN, -1, -1));

    char data[BUF_SIZE + 1];
    while (1)
    {
        int len = uart_read_bytes(UART_PORT_NUM, data, (BUF_SIZE - 1), pdMS_TO_TICKS(20));
        if (len)
        {
            data[len] = 0;
            printf("RECV:%s\n", data);
        }
    }
    uart_driver_delete(UART_PORT_NUM);
    vTaskDelete(NULL);
}

static void chat()
{
    xTaskCreate(recipient, "recv", 1024 * 10, NULL, 10, NULL);
    while (1)
    {
        char line[BUF_SIZE];
        if (fgets(line, sizeof(line), stdin))
        {
            printf("SEND:%s\n", line);
            uart_write_bytes(UART_PORT_NUM, line, strlen(line));
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Chat room Opening up!");
    chat();
    vTaskDelete(NULL);
}
