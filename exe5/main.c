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

QueueHandle_t xQueueBtn;
SemaphoreHandle_t xSemaphoreLedR;
SemaphoreHandle_t xSemaphoreLedY;

volatile int led_r = 0;
volatile int led_y = 3;
void btn_callback(uint gpio, uint32_t events){
    if (events == 0x4 && gpio == BTN_PIN_R){
        int led = 1;
        xQueueSendFromISR(xQueueBtn, &led, 0);
    }
    if (events == 0x4 && gpio == BTN_PIN_Y){
        int led = 0;

        xQueueSendFromISR(xQueueBtn, &led, 0);

    }
}


void btn_task(void* p) {
    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);
    gpio_set_irq_enabled_with_callback(BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true,
                                       &btn_callback);

    gpio_init(BTN_PIN_Y);
    gpio_set_dir(BTN_PIN_Y, GPIO_IN);
    gpio_pull_up(BTN_PIN_Y);
    gpio_set_irq_enabled(BTN_PIN_Y, GPIO_IRQ_EDGE_FALL, true);

  
    
    int led = 2;
    int r_on = 0;
    int y_on = 0;
    while (true) {
        if (xQueueReceive(xQueueBtn, &led, 50) == pdTRUE) {
            printf("LED %d", led);
            if (led == 1) {
                r_on = !r_on;
                xSemaphoreGive(xSemaphoreLedR);
                
            }
            else if (led == 0) {
                y_on = !y_on;
                xSemaphoreGive(xSemaphoreLedY);
            }

            

        }

        
    
        

    }
}

void led_1_task(void *p) {
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);

    int delay = 100;
    int pisca = 0;

    while (true) {

        if (xSemaphoreTake(xSemaphoreLedR, pdMS_TO_TICKS(50)) == pdTRUE) {
            pisca = !pisca;
        }
        if (pisca){
            gpio_put(LED_PIN_R, 1);
            vTaskDelay(pdMS_TO_TICKS(delay));
            gpio_put(LED_PIN_R, 0);
            vTaskDelay(pdMS_TO_TICKS(delay));
        }
        
    }
}



void led_2_task(void *p) {
    gpio_init(LED_PIN_Y);
    gpio_set_dir(LED_PIN_Y, GPIO_OUT);

    int delay = 100;
    int pisca = 0;

    while (true) {

        if (xSemaphoreTake(xSemaphoreLedY, pdMS_TO_TICKS(50)) == pdTRUE) {
            pisca = !pisca;
        }
        if (pisca){
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

    xQueueBtn = xQueueCreate(32, sizeof(int));
    xSemaphoreLedR = xSemaphoreCreateBinary();
    xSemaphoreLedY = xSemaphoreCreateBinary();

    xTaskCreate(btn_task, "BTN_Task 1", 256, NULL, 1, NULL);
    xTaskCreate(led_1_task, "LED_Task 1", 256, NULL, 1, NULL);

    //xTaskCreate(btn2_task, "BTN_Task 1", 256, NULL, 1, NULL);
    xTaskCreate(led_2_task, "LED_Task 2", 256, NULL, 1, NULL);

    vTaskStartScheduler();

    while(1){}

    return 0;
}