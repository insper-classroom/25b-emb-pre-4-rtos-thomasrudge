/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>


#include <stdio.h>
#include <string.h> 
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/irq.h"

const int BTN_PIN_R = 28;
const int BTN_PIN_Y = 21;

const int LED_PIN_R = 5;
const int LED_PIN_Y = 10;

volatile int flag_r = 0;

QueueHandle_t xQueueButId;
QueueHandle_t xQueueButId2;
SemaphoreHandle_t xSemaphore_r;
SemaphoreHandle_t xSemaphore_y;


void btn_callback(uint gpio, uint32_t events){
    if (events == 0x4 && gpio == BTN_PIN_R){
        xSemaphoreGiveFromISR(xSemaphore_r, 0);
    }
    if (events == 0x4 && gpio == BTN_PIN_Y){
        xSemaphoreGiveFromISR(xSemaphore_y, 0);
    }
}


void btn_task(void* p) {
    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);
    gpio_set_irq_enabled_with_callback(BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true,
                                       &btn_callback);

    int delay = 0;
    while (true) {
        if (xSemaphoreTake(xSemaphore_r, pdMS_TO_TICKS(500)) == pdTRUE) {
            if (delay == 0){
                delay = 100;
            }
            else{
                delay = 0;
            }
            
            printf("delay btn %d \n", delay);
            xQueueSend(xQueueButId, &delay, 0);
        }

    }
}

void led_1_task(void *p) {
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);

    int delay = 0;

    while (true) {
        if (xQueueReceive(xQueueButId, &delay, 0)) {
            printf("%d\n", delay);
        }

        if (delay > 0) {
            gpio_put(LED_PIN_R, 1);
            vTaskDelay(pdMS_TO_TICKS(delay));
            gpio_put(LED_PIN_R, 0);
            vTaskDelay(pdMS_TO_TICKS(delay));
        }
    }
}

void btn2_task(void* p) {
    gpio_init(BTN_PIN_Y);
    gpio_set_dir(BTN_PIN_Y, GPIO_IN);
    gpio_pull_up(BTN_PIN_Y);
    gpio_set_irq_enabled(BTN_PIN_Y, GPIO_IRQ_EDGE_FALL, true);

    int delay = 0;
    while (true) {
        if (xSemaphoreTake(xSemaphore_y, pdMS_TO_TICKS(500)) == pdTRUE) {
            if (delay == 0){
                delay = 100;
            }
            else{
                delay = 0;
            }
            
            printf("delay btn %d \n", delay);
            xQueueSend(xQueueButId2, &delay, 0);
        }

    }
}

void led_2_task(void *p) {
    gpio_init(LED_PIN_Y);
    gpio_set_dir(LED_PIN_Y, GPIO_OUT);

    int delay = 0;

    while (true) {
        if (xQueueReceive(xQueueButId2, &delay, 0)) {
            printf("%d\n", delay);
        }

        if (delay > 0) {
            gpio_put(LED_PIN_Y, 1);
            vTaskDelay(pdMS_TO_TICKS(delay));
            gpio_put(LED_PIN_Y, 0);
            vTaskDelay(pdMS_TO_TICKS(delay));
        }
    }
}



int main() {
    stdio_init_all();

    printf("Start RTOS \n");

    xQueueButId = xQueueCreate(32, sizeof(int));
    xQueueButId2 = xQueueCreate(32, sizeof(int));
    xSemaphore_r = xSemaphoreCreateBinary();
    xSemaphore_y = xSemaphoreCreateBinary();

    xTaskCreate(btn_task, "BTN_Task 1", 256, NULL, 1, NULL);
    xTaskCreate(led_1_task, "LED_Task 1", 256, NULL, 1, NULL);

    xTaskCreate(btn2_task, "BTN_Task 1", 256, NULL, 1, NULL);
    xTaskCreate(led_2_task, "LED_Task 1", 256, NULL, 1, NULL);

    vTaskStartScheduler();

    while(1){}

    return 0;
}