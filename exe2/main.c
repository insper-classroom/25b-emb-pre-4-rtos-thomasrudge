#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include "pico/stdlib.h"
#include <stdio.h>

const int BTN_PIN_R = 28;
const int BTN_PIN_G = 26;

const int LED_PIN_R = 4;
const int LED_PIN_G = 6;

volatile int flag_r = 0;
volatile int flag_g = 0;

SemaphoreHandle_t xSemaphore_r;
SemaphoreHandle_t xSemaphore_g;

void btn_callback(uint gpio, uint32_t events) {
    if (events == 0x4) {  // fall edge
        if (gpio == BTN_PIN_R) {
            flag_r = 1;
        
        } 
    
      else if (gpio == BTN_PIN_G){
          flag_g = 1;
      }
    }
}


void led_2_task(void *p) {
  gpio_init(LED_PIN_G);
  gpio_set_dir(LED_PIN_G, GPIO_OUT);

  int delay = 250;

  while (true) {

    if (xSemaphoreTake(xSemaphore_g, pdMS_TO_TICKS(500)) == pdTRUE) {
      gpio_put(LED_PIN_G, 1);
      vTaskDelay(pdMS_TO_TICKS(delay));
      gpio_put(LED_PIN_G, 0);
      vTaskDelay(pdMS_TO_TICKS(delay));
    }
  }
}

void btn_2_task(void *p) {
  gpio_init(BTN_PIN_G);
  gpio_set_dir(BTN_PIN_G, GPIO_IN);
  gpio_pull_up(BTN_PIN_G);

  while (true) {
    if (flag_g) {
      xSemaphoreGive(xSemaphore_g);
      flag_g = 0;
      
    }
    else{
      vTaskDelay(pdMS_TO_TICKS(1));
    }
  }
}

void led_1_task(void *p) {
  gpio_init(LED_PIN_R);
  gpio_set_dir(LED_PIN_R, GPIO_OUT);

  int delay = 250;

  while (true) {

    if (xSemaphoreTake(xSemaphore_r, pdMS_TO_TICKS(500)) == pdTRUE) {
      gpio_put(LED_PIN_R, 1);
      vTaskDelay(pdMS_TO_TICKS(delay));
      gpio_put(LED_PIN_R, 0);
      vTaskDelay(pdMS_TO_TICKS(delay));
    }
  }
}

void btn_1_task(void *p) {
  gpio_init(BTN_PIN_R);
  gpio_set_dir(BTN_PIN_R, GPIO_IN);
  gpio_pull_up(BTN_PIN_R);

    while (true) {
    if (flag_r) {
      xSemaphoreGive(xSemaphore_r);
      flag_r = 0;
      
    }
    else{
      vTaskDelay(pdMS_TO_TICKS(1));
    }
  }
}


int main() {
  stdio_init_all();
  gpio_set_irq_enabled_with_callback(BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true, &btn_callback);
  gpio_set_irq_enabled(BTN_PIN_G, GPIO_IRQ_EDGE_FALL, true);
  printf("Start RTOS \n");

  xSemaphore_r = xSemaphoreCreateBinary();
  xSemaphore_g = xSemaphoreCreateBinary();

  xTaskCreate(led_1_task, "LED_Task 1", 256, NULL, 1, NULL);
  xTaskCreate(btn_1_task, "BTN_Task 1", 256, NULL, 1, NULL);
  xTaskCreate(led_2_task, "LED_Task 2", 256, NULL, 1, NULL);
  xTaskCreate(btn_2_task, "BTN_Task 2", 256, NULL, 1, NULL);

  vTaskStartScheduler();

  while (true)
    ;
}
