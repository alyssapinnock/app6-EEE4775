/* ------------------------------------------------------------
   Application: 06 - Rev1
   Release Type: Real-Time System for Dream Company
   Class: Real Time Systems - Su 2025
   Author: Alyssa Pinnock
   Email: al310186@ucf.edu
   Company: [University of Central Florida]
   Website: theDRACOlab.com
   AI Use: Please commented inline where you use(d) AI

   MY CHOSEN COMPANY/PROJECT INFO:
    - Autonomous Vehicles - Emergency Brake System
---------------------------------------------------------------
*/

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_log.h"

// Initializing connections and ports
#define LED_GREEN GPIO_NUM_5 // green LED that indicates vehicle is on
#define LED_RED   GPIO_NUM_4 // red LED for speeding alert
#define LED_BLUE  GPIO_NUM_15 // blue LED for emergency brake alerts
#define BUTTON_PIN GPIO_NUM_18
#define POT_ADC_CHANNEL  ADC1_CHANNEL_6 // GPIO34

#define MAX_SPEED_COUNTER 7
#define THRESHOLD_RISING_SENSOR 0 // set to emulate behavior of sensor toggling quickly
#define SPEED_LIMIT 2400 // The speed limit for the vehicle (estimated around ~85mph)


// Handles for semaphores, mutex, and queue
SemaphoreHandle_t sem_button;
SemaphoreHandle_t sem_sensor;
SemaphoreHandle_t print_mutex;
QueueHandle_t speed_queue;

// Global variable definitions
volatile int SEMCNT = 0; // semaphore counter to track speeding
volatile int prevState = 0; 
volatile bool speedingAlert = false; // global speeding alert status
volatile bool emergencyBrakePress = false; // global emergency brake press status

// Vehicle On - Green LED blinking to indicate vehicle is on
void heartbeat_task(void *pvParameters) { // Soft deadline, Time: 2s (or 1s on, 1s off)
    while (1) {
        gpio_set_level(LED_GREEN, 1);
        vTaskDelay(pdMS_TO_TICKS(1000));
        gpio_set_level(LED_GREEN, 0);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// Alert LED - Blinks LED during speeding alerts
void alert_led_task(void *pvParameters) { // Soft, Time: 400ms (or 200ms on, 200ms off)
  while (1) {
    if (speedingAlert == true && emergencyBrakePress == false){ // only will blink if vehicle is speeding AND emergency brake is not pressed
      gpio_set_level(LED_RED, 1);
      vTaskDelay(pdMS_TO_TICKS(200));
      gpio_set_level(LED_RED, 0);
      vTaskDelay(pdMS_TO_TICKS(200));
    }

    else { // speeding alert led is off otherwise
      gpio_set_level(LED_RED, 0);  // makes sure alert LED is off
      vTaskDelay(pdMS_TO_TICKS(100));
    }
  }

}

// Detect Speed - measures the current speed of the vehicle
void detect_speed(void *pvParameters) { // Hard deadline, Time: 100ms
    while (1) {
        int currentSpeed = adc1_get_raw(POT_ADC_CHANNEL); // reads current car speed from speedometer
        int analog_to_mph = currentSpeed/30; // "converting" the ADC value of speedometer to mph (estimated)

        if (currentSpeed > SPEED_LIMIT) { // if car's current speed exceeds speed limit
          if (prevState == 0) { // if previous state was not speeding
            prevState = THRESHOLD_RISING_SENSOR; 
            speedingAlert = true; // set global speeding alert to true

            if(SEMCNT < MAX_SPEED_COUNTER+1) SEMCNT++; // DO NOT REMOVE THIS LINE
            xSemaphoreGive(sem_sensor); // Signal sensor event

            xQueueSend(speed_queue, &analog_to_mph, 0); // Queue for speed over limit
            
            }  
        }

        else {
          speedingAlert = false; // in any other case, speeding alert is false
          prevState = 0;

          // Display current vehicle speed if there is no active speeding alert
          xSemaphoreTake(print_mutex, portMAX_DELAY);
          printf("Vehicle speed: %d mph\n", analog_to_mph); // current car speed printed
          xSemaphoreGive(print_mutex);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
        
    }
}

// ISR for button/emergency brake press
void IRAM_ATTR brake_isr_handler(void* arg) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(sem_button, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken) { 
        portYIELD_FROM_ISR(); 
    }
}

// Emergency Brake - Actions taken during emergency brake press
void emergency_brake_task(void *pvParameters) { // Hard deadline, time period based on push button/brake press instance
    while (true) {
        if (xSemaphoreTake(sem_button, portMAX_DELAY)) {
          emergencyBrakePress = true; // Setting global emergency brake status to true

          // Printing 5x that emergency brake was pressed (for additional visibility in serial montior)
          for (int i = 0; i <= 5; i++){
            xSemaphoreTake(print_mutex, portMAX_DELAY);
            printf("EMERGENCY BRAKE PRESSED! Vehicle will now stop to avoid collision!\n");
            xSemaphoreGive(print_mutex);
            vTaskDelay(pdMS_TO_TICKS(200)); 

          }

          // Keep blue emergency brake light on for 3 seconds
          gpio_set_level(LED_BLUE, 1);
          vTaskDelay(pdMS_TO_TICKS(3000));
          gpio_set_level(LED_BLUE, 0);

          emergencyBrakePress = false; // turning off global emergency brake press status to resume other tasks
        }
    }
}


// Main event handler - used to print collision alert and speed over the limit
void event_handler_task(void *pvParameters) { // Hard deadline
    while (1) {
        if (xSemaphoreTake(sem_sensor, 0)) { // sensor event 

          if (emergencyBrakePress == true) { // check if emergency brake is pressed to skip printing
          vTaskDelay(pdMS_TO_TICKS(100));
          continue; // continue to next iteration
          }

          SEMCNT--;  // DO NOT MODIFY THIS LINE

          xSemaphoreTake(print_mutex, portMAX_DELAY);
          printf("WARNING: Vehicle rapidly increasing speed for %d seconds. Reduce speed to avoid collision!\n", SEMCNT);

          if(SEMCNT == MAX_SPEED_COUNTER) {

            printf("\n***VEHICLE COLLISION ALERT!*** Vehicle was going too fast...\n");
            
          }

          // Queue for getting the over-limit speed of the vehicle
          int got_speed;
          if(xQueueReceive(speed_queue, &got_speed, 0)) {

            printf("CURRENT SPEED OVER LIMIT: %d mph\n", got_speed);

          }

          xSemaphoreGive(print_mutex);

          vTaskDelay(pdMS_TO_TICKS(100));

        }

        vTaskDelay(pdMS_TO_TICKS(10)); // Idle delay to yield CPU
    }
}

void app_main(void) {
    // Configure output LEDs
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LED_GREEN) | (1ULL << LED_RED) | (1ULL << LED_BLUE),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&io_conf);

    // Configure input button
    gpio_config_t btn_conf = {
        .pin_bit_mask = (1ULL << BUTTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .intr_type = GPIO_INTR_NEGEDGE
    };
    gpio_config(&btn_conf);

    // Configure button ISR
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_PIN, brake_isr_handler, NULL);

    // Configure ADC
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(POT_ADC_CHANNEL, ADC_ATTEN_DB_11);

    // Initializing semaphores & queue
    sem_button = xSemaphoreCreateBinary(); // Binary semaphore for button
    sem_sensor = xSemaphoreCreateCounting(MAX_SPEED_COUNTER, 0); // Counting semaphore for sensor events
    print_mutex = xSemaphoreCreateMutex(); // Mutex for console output
    speed_queue = xQueueCreate(10, sizeof(int)); // Queue for speed values

    // Create tasks
    xTaskCreate(heartbeat_task, "heartbeat", 2048, NULL, 1, NULL);
    xTaskCreate(detect_speed, "sensor", 2048, NULL, 2, NULL);
    xTaskCreate(emergency_brake_task, "brake", 2048, NULL, 3, NULL);
    xTaskCreate(event_handler_task, "event_handler", 2048, NULL, 2, NULL);
    xTaskCreate(alert_led_task, "led_alert", 2048, NULL, 2, NULL);

}