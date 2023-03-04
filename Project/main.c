/**
 * RP2040 FreeRTOS Project
 * 
 * @copyright 2022, Tony Smith (@smittytone)
 * @version   1.4.1
 * @licence   MIT
 *
 */
#include "main.h"


/*
 * GLOBALS
 */
// This is the inter-task queue
volatile QueueHandle_t queue = NULL;

// Set a delay time of exactly 500ms
const TickType_t ms_delay = 500 / portTICK_PERIOD_MS;

// FROM 1.0.1 Record references to the tasks
TaskHandle_t gpio_task_handle = NULL;
TaskHandle_t pico_task_handle = NULL;

/*
 * MULTICORE FUNCTIONS
 */


void core1_entry() {
    while (1) {
        // Function pointer is passed to us via the FIFO
        // We have one incoming int32_t as a parameter, and will provide an
        // int32_t return value by simply pushing it back on the FIFO
        // which also indicates the result is ready.
        int32_t (*func)() = (int32_t(*)()) multicore_fifo_pop_blocking();
        int32_t p = multicore_fifo_pop_blocking();
        int32_t result = (*func)(p);
        multicore_fifo_push_blocking(result);
    }
}

int32_t mutiply_by_two(int32_t n) {
    return n * 2;
}

/*
 * FUNCTIONS
 */

/**
 * @brief Repeatedly flash the Pico's built-in LED.
 */
void led_task_pico(void* unused_arg) {
    // Store the Pico LED state
    uint8_t pico_led_state = 0;
    int32_t ms_delay_led;
    
    // Configure the Pico's on-board LED
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    
    while (true) {
        ms_delay_led = ms_delay;
        multicore_fifo_push_blocking((uintptr_t) &mutiply_by_two);
        multicore_fifo_push_blocking(ms_delay);

        // Turn Pico LED on an add the LED state
        // to the FreeRTOS xQUEUE
        log_debug("PICO LED FLASH");
        pico_led_state = 1;
        gpio_put(PICO_DEFAULT_LED_PIN, pico_led_state);
        xQueueSendToBack(queue, &pico_led_state, 0);
        vTaskDelay(ms_delay_led);

        ms_delay_led = multicore_fifo_pop_blocking();
        
        // Turn Pico LED off an add the LED state
        // to the FreeRTOS xQUEUE
        pico_led_state = 0;
        gpio_put(PICO_DEFAULT_LED_PIN, pico_led_state);
        xQueueSendToBack(queue, &pico_led_state, 0);
        vTaskDelay(ms_delay_led);
    }
}


/**
 * @brief Repeatedly flash an LED connected to GPIO pin 20
 *        based on the value passed via the inter-task queue.
 */
void led_task_gpio(void* unused_arg) {
    // This variable will take a copy of the value
    // added to the FreeRTOS xQueue
    uint8_t passed_value_buffer = 0;
    
    // Configure the GPIO LED
    gpio_init(RED_LED_PIN);
    gpio_set_dir(RED_LED_PIN, GPIO_OUT);
    
    while (true) {
        // Check for an item in the FreeRTOS xQueue
        if (xQueueReceive(queue, &passed_value_buffer, portMAX_DELAY) == pdPASS) {
            // Received a value so flash the GPIO LED accordingly
            // (NOT the sent value)
            if (passed_value_buffer) log_debug("GPIO LED FLASH");
            gpio_put(RED_LED_PIN, passed_value_buffer == 1 ? 0 : 1);
        }
    }
}


/**
 * @brief Generate and print a debug message from a supplied string.
 *
 * @param msg: The base message to which `[DEBUG]` will be prefixed.
 */
void log_debug(const char* msg) {
    uint msg_length = 9 + strlen(msg);
    char* sprintf_buffer = malloc(msg_length);
    sprintf(sprintf_buffer, "[DEBUG] %s\n", msg);
    #ifdef DEBUG
    printf("%s", sprintf_buffer);
    #endif
    free(sprintf_buffer);
}


/**
 * @brief Show basic device info.
 */
void log_device_info(void) {
    printf("App: %s %s\n Build: %i\n", APP_NAME, APP_VERSION, BUILD_NUM);
}


/*
 * RUNTIME START
 */
int main() {
    // Enable STDIO
    #ifdef DEBUG
    stdio_usb_init();
    #endif

    multicore_launch_core1(core1_entry);
    
    // Set up two tasks
    // FROM 1.0.1 Store handles referencing the tasks; get return values
    // NOTE Arg 3 is the stack depth -- in words, not bytes
    BaseType_t pico_status = xTaskCreate(led_task_pico, 
                                         "PICO_LED_TASK", 
                                         128, 
                                         NULL, 
                                         1, 
                                         &pico_task_handle);
    BaseType_t gpio_status = xTaskCreate(led_task_gpio, 
                                         "GPIO_LED_TASK", 
                                         128, 
                                         NULL, 
                                         1, 
                                         &gpio_task_handle);
    
    // Set up the event queue
    queue = xQueueCreate(4, sizeof(uint8_t));
    
    // Log app info
    log_device_info();
    
    // Start the FreeRTOS scheduler
    // FROM 1.0.1: Only proceed with valid tasks
    if (pico_status == pdPASS || gpio_status == pdPASS) {
        vTaskStartScheduler();
    }
    
    // We should never get here, but just in case...
    while(true) {
        // NOP
    };
}