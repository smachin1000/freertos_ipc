#include "../drivers/mss_gpio/mss_gpio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>

/**
 * This task reads analog input values from the queue, then updates the LEDs
 * to display the value like a bar graph.
 */

extern xQueueHandle queue_h;

void led_initialization()
{
    /* Configuration of GPIO's */
    MSS_GPIO_config(MSS_GPIO_0 , MSS_GPIO_OUTPUT_MODE );
    MSS_GPIO_config(MSS_GPIO_1 , MSS_GPIO_OUTPUT_MODE );
    MSS_GPIO_config(MSS_GPIO_2 , MSS_GPIO_OUTPUT_MODE );
    MSS_GPIO_config(MSS_GPIO_3 , MSS_GPIO_OUTPUT_MODE );
    MSS_GPIO_config(MSS_GPIO_4 , MSS_GPIO_OUTPUT_MODE );
    MSS_GPIO_config(MSS_GPIO_5 , MSS_GPIO_OUTPUT_MODE );
    MSS_GPIO_config(MSS_GPIO_6 , MSS_GPIO_OUTPUT_MODE );
    MSS_GPIO_config(MSS_GPIO_7 , MSS_GPIO_OUTPUT_MODE );
}

void led_task(void *para)
{
    while (1) {
	      while (uxQueueMessagesWaiting(queue_h) == 0) {
		      taskYIELD();
	      }
        long lReceivedValue;
        const int xStatus = xQueueReceive(queue_h, &lReceivedValue, 0);
        if (xStatus == pdPASS) {
            // value received should be will be 600 to 3800, so threshold and scale it as 0-255 now
            if (lReceivedValue < 600) {
                lReceivedValue = 600;
		        }
            if (lReceivedValue > 3800) {
                lReceivedValue = 3800;
            }
        		const double scaled_value = (lReceivedValue - 600.0) * 255.0 / (3800.0 - 600.0);
        		MSS_GPIO_set_outputs(255 - round(scaled_value));
	      }
	      else {
            printf("\r\nError %d reading from queue\r\n", xStatus);
            break;
        }
    }
}
