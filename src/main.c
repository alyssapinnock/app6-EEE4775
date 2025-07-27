/* ------------------------------------------------------------
   Application: 06 - Rev1
   Release Type: Use of Memory Based Task Communication
   Class: Real Time Systems - Su 2025
   Author: Alyssa Pinnock
   Email: al310186@ucf.edu
   Company: [University of Central Florida]
   Website: theDRACOlab.com
   AI Use: Please commented inline where you use(d) AI
---------------------------------------------------------------
*/

/*
MY CHOSEN COMPANY/PROJECT INFO:
- Healthcare - Insulin Pump

*/

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_log.h"

// Initializing connections and ports
#define active_led GPIO_NUM_5 // green LED that indicates that insulin pump is active
#define LED_RED   GPIO_NUM_4
#define BUTTON_PIN GPIO_NUM_18
#define POT_ADC_CHANNEL ADC1_CHANNEL_6 // GPIO34

#define MAX_COUNT_SEM 10

#define THRESHOLD_RISING_SENSOR 0 // set to emulate behavior of sensor toggling quickly

// previously "SENSOR_THRESHOLD":
#define MAX_HEARTRATE 2407 // I set my desired max heartrate to around ~120bpm, but in analog

// To set a new heart rate for someone:
// Do [(your_max_heartrate) / 204] * 4095, then change MAX_HEARTRATE to that value
// (this just provides a more realistic reading of heartrate that isn't in analog)


// Handles for semaphores and mutex - you'll initialize these in the main program
SemaphoreHandle_t sem_button;
SemaphoreHandle_t sem_sensor;
SemaphoreHandle_t print_mutex;

volatile int SEMCNT = 0; //You may not use this value in your logic -- but you can print it if you wish

//TODO 0b: Set heartbeat to cycle once per second (on for one second, off for one second)
//Find TODO 0c
void heartbeat_task(void *pvParameters) {
  bool led_on = false;
    while (1) {
        gpio_set_level(LED_GREEN, 1);
        vTaskDelay(pdMS_TO_TICKS(1000));
        gpio_set_level(LED_GREEN, 0);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

volatile int prevState = 0; 

void sensor_task(void *pvParameters) {
    while (1) {
        int val = adc1_get_raw(POT_ADC_CHANNEL); // reads current sensor value
        static int previousVal = 0; // initializes prev value to 0 (for first iteration)
        int analog_to_bpm = val/20;

        //TODO 2: Add serial print to log the raw sensor value (mutex protected)
        //Hint: use xSemaphoreTake( ... which semaphore ...) and printf
        xSemaphoreTake(print_mutex, portMAX_DELAY);
        printf("BPM: %d\n", val); // BPM printed as analog values
        //printf("BPM (actual): %d\n", analog_to_bpm); // created to see heartrate that isn't in analog (un-comment to print)
        xSemaphoreGive(print_mutex);

        if (val > MAX_HEARTRATE) { // if ADC value exceeds the threshold
          if (prevState == 0) {
            prevState = THRESHOLD_RISING_SENSOR;
            if(SEMCNT < MAX_COUNT_SEM+1) SEMCNT++; // DO NOT REMOVE THIS LINE
            xSemaphoreGive(sem_sensor); // Signal sensor event
            
          }
            
        }

        else {
          prevState = 0;
        }

        //previousVal = val; // prev value is updated to current value (which will be "previous" in next iteration)

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}


#define DEBOUNCE_MS        50               // adjust as needed
#define POLL_PERIOD_MS     10               // must stay at 10 ms


void button_task(void *pvParameters)
{
    const TickType_t debounceTicks = pdMS_TO_TICKS(DEBOUNCE_MS);

    int             prevState    = 1;       // idle = HIGH (pull-up)
    TickType_t      lastPress    = 0;       // last accepted press time

    while (true) {
        int state = gpio_get_level(BUTTON_PIN);

        /* ---- 4a: debounce & rising-edge detection --------------------- */
        if (state == 0 && prevState == 1) {             // HIGH→LOW edge
            TickType_t now = xTaskGetTickCount();

            if (now - lastPress > debounceTicks) {      // outside window
                lastPress = now;

                /* ---- signal the event to whoever needs it ------------ */
                xSemaphoreGive(sem_button);

                /* ---- 4b: mutex-protected console message ------------- */
                //Handled in event_handler_task
            }
        }
        prevState = state;                              // remember level
        vTaskDelay(pdMS_TO_TICKS(10));      // **don’t change**
    }
}


void event_handler_task(void *pvParameters) {
    while (1) {
        if (xSemaphoreTake(sem_sensor, 0)) {
            SEMCNT--;  // DO NOT MODIFY THIS LINE

            xSemaphoreTake(print_mutex, portMAX_DELAY);
            printf("WARNING: Patient max heartrate exceeded the max %d times!\n", SEMCNT);
            if(SEMCNT == MAX_COUNT_SEM)
            {
              printf("\n***CARDIAC ARREST!*** Patient heart rate was too high for too long...\n");
            }
            xSemaphoreGive(print_mutex);

            gpio_set_level(LED_RED, 1);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_set_level(LED_RED, 0);
        }

        if (xSemaphoreTake(sem_button, 0)) {
            xSemaphoreTake(print_mutex, portMAX_DELAY);
            printf("EMERGENCY BUTTON PRESSED: Administering IV medication to regulate heart rate!\n");
            xSemaphoreGive(print_mutex);

            gpio_set_level(LED_RED, 1);
            vTaskDelay(pdMS_TO_TICKS(300));
            gpio_set_level(LED_RED, 0);
        }

        vTaskDelay(pdMS_TO_TICKS(10)); // Idle delay to yield CPU
    }
}

void app_main(void) {
    // Configure output LEDs
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LED_GREEN) | (1ULL << LED_RED),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&io_conf);

    // Configure input button
    gpio_config_t btn_conf = {
        .pin_bit_mask = (1ULL << BUTTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE
    };
    gpio_config(&btn_conf);

    // Configure ADC
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(POT_ADC_CHANNEL, ADC_ATTEN_DB_11);

    // Creating the semaphores and mutex
    sem_button = xSemaphoreCreateBinary(); // Binary semaphore for button
    sem_sensor = xSemaphoreCreateCounting(MAX_COUNT_SEM, 0); // Counting semaphore for sensor events
    print_mutex = xSemaphoreCreateMutex(); // Mutex for console output

    // Create tasks
    xTaskCreate(heartbeat_task, "heartbeat", 2048, NULL, 1, NULL);
    xTaskCreate(sensor_task, "sensor", 2048, NULL, 2, NULL);
    xTaskCreate(button_task, "button", 2048, NULL, 3, NULL);
    xTaskCreate(event_handler_task, "event_handler", 2048, NULL, 2, NULL);
}
