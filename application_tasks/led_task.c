#include "../drivers/mss_gpio/mss_gpio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>
#include <math.h>

#include "../main.h"

// Define some upper and lower thresholds for the analog potentiometer read
static const int MAX_ANALOG_VALUE = 3800;
static const int MIN_ANALOG_VALUE = 650;

/**
 * This task reads analog input values from the queue, then updates the LEDs
 * to display the value like a bar graph.
 */
void led_initialization()
{
    /* Configuration of GPIOs */
    MSS_GPIO_config(MSS_GPIO_0, MSS_GPIO_OUTPUT_MODE );
    MSS_GPIO_config(MSS_GPIO_1, MSS_GPIO_OUTPUT_MODE );
    MSS_GPIO_config(MSS_GPIO_2, MSS_GPIO_OUTPUT_MODE );
    MSS_GPIO_config(MSS_GPIO_3, MSS_GPIO_OUTPUT_MODE );
    MSS_GPIO_config(MSS_GPIO_4, MSS_GPIO_OUTPUT_MODE );
    MSS_GPIO_config(MSS_GPIO_5, MSS_GPIO_OUTPUT_MODE );
    MSS_GPIO_config(MSS_GPIO_6 , MSS_GPIO_OUTPUT_MODE );
    MSS_GPIO_config(MSS_GPIO_7, MSS_GPIO_OUTPUT_MODE );
}

void led_task(task_arg_t* ta)
{
	const xQueueHandle queue_h = ta->queue_h;

    while (1) {
    	// wait until there's at least one message in the queue
        while (uxQueueMessagesWaiting(queue_h) == 0) {
            taskYIELD();
        }
        // queue should now definitely have at least one value in it, so read the value
        // from the queue
        uint16_t received_value;
        const int xStatus = xQueueReceive(queue_h, &received_value, 0);
        if (xStatus == pdPASS) {
            // value received should be will be 650 to 3800, so threshold and scale it as 0-255 now
            if (received_value < MIN_ANALOG_VALUE) {
                received_value = MIN_ANALOG_VALUE;
            }
            if (received_value > MAX_ANALOG_VALUE) {
                received_value = MAX_ANALOG_VALUE;
            }
            uint32_t scaled_value = round((received_value - MIN_ANALOG_VALUE) * 255.0 /
                                          (MAX_ANALOG_VALUE - MIN_ANALOG_VALUE));
            // now determine which of the 8 LEDs to light, remembering they
            // are opposite polarity.
            uint8_t v = 0;
            int x;
            for (x = 7;x >= 0;x--) {
                if (scaled_value >= (1 << x)) {
                    v |= (1 << x);
                    scaled_value = scaled_value / 2;
                }
            }
            MSS_GPIO_set_outputs(0xffffffff - v);
        }
        else {
            printf("\r\nError %d reading from queue\r\n", xStatus);
            break;
        }
    }
}
