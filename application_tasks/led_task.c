#include "../drivers/mss_gpio/mss_gpio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>
#include <math.h>

static const int MAX_ANALOG_VALUE = 3800;
static const int MIN_ANALOG_VALUE = 600;

/**
 * This task reads analog input values from the queue, then updates the LEDs
 * to display the value like a bar graph.
 */
extern xQueueHandle queue_h;

void led_initialization()
{
    /* Configuration of GPIO's */
    MSS_GPIO_config(MSS_GPIO_0, MSS_GPIO_OUTPUT_MODE );
    MSS_GPIO_config(MSS_GPIO_1, MSS_GPIO_OUTPUT_MODE );
    MSS_GPIO_config(MSS_GPIO_2, MSS_GPIO_OUTPUT_MODE );
    MSS_GPIO_config(MSS_GPIO_3, MSS_GPIO_OUTPUT_MODE );
    MSS_GPIO_config(MSS_GPIO_4, MSS_GPIO_OUTPUT_MODE );
    MSS_GPIO_config(MSS_GPIO_5, MSS_GPIO_OUTPUT_MODE );
    MSS_GPIO_config(MSS_GPIO_6 , MSS_GPIO_OUTPUT_MODE );
    MSS_GPIO_config(MSS_GPIO_7, MSS_GPIO_OUTPUT_MODE );
}

void led_task(void *para)
{
    while (1) {
        while (uxQueueMessagesWaiting(queue_h) == 0) {
            taskYIELD();
        }
        // queue should now definitely have at least one value in it
        long lReceivedValue;
        const int xStatus = xQueueReceive(queue_h, &lReceivedValue, 0);
        if (xStatus == pdPASS) {
            // value received should be will be 600 to 3800, so threshold and scale it as 0-255 now
            if (lReceivedValue < MIN_ANALOG_VALUE) {
                lReceivedValue = MIN_ANALOG_VALUE;
            }
            if (lReceivedValue > MAX_ANALOG_VALUE) {
                lReceivedValue = MAX_ANALOG_VALUE;
            }
            uint16_t scaled_value = round((lReceivedValue - MIN_ANALOG_VALUE) * 255.0 /
                                          (MAX_ANALOG_VALUE - MIN_ANALOG_VALUE));
            // now determine which of the 8 LEDs to light, remembering they
            // are opposite polarity.
            uint8_t v = 255;
            int x;
            for (x = 7;x >= 0;x--) {
                if (scaled_value >= (1 << x)) {
                    v = v & ~(1 << x);
                    scaled_value -= (1 << x);
                }
            }
            MSS_GPIO_set_outputs(v);
        }
        else {
            printf("\r\nError %d reading from queue\r\n", xStatus);
            break;
        }
    }
}
