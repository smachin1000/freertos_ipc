#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <freertos.h>
#include <task.h>
#include <queue.h>
#include <math.h>

#include "../drivers/mss_ace/mss_ace.h"

#include "FreeRTOS.h"
#include "median_filter.h"

/**
 * This task reads the analog input value from the potentiometer and sends it
 * to the IPC queue.
 */
extern int QUEUE_LENGTH;


void analog_read_task(xQueueHandle handle)
{
	const xQueueHandle queue_h = handle;

      while (1) {
        const ace_channel_handle_t current_channel = ACE_get_first_channel();
        const uint16_t adc_result = ACE_get_ppe_sample(current_channel);
        const uint16_t value_to_send = median_filter(adc_result);

        if (uxQueueMessagesWaiting(queue_h) < QUEUE_LENGTH) {
            const int xStatus = xQueueSendToBack(queue_h, &value_to_send, 0);
            if (xStatus != pdPASS) {
                /* The send operation could not complete because the queue was full -
                   this must be an error as the queue should never contain more than
                   one item! */
                printf( "Could not send to the queue, error code %d.\r\n", xStatus );
                break;
            }
        }
        else {
            // no space in transmit queue, so don't hog the CPU
            taskYIELD();
        }
    }
}
