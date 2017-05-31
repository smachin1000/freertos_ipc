#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <freertos.h>
#include <queue.h>

#include "../drivers/mss_ace/mss_ace.h"
#include "mss_ace_configurator.h"
#include "../../CMSIS/a2fxxxm3.h"
#include "../../CMSIS/mss_assert.h"
#include "../../drivers_config/mss_ace/ace_handles.h"
#include "../../drivers_config/mss_ace/ace_config.h"

#include "FreeRTOS.h"

/**
 * This task reads the analog input value from the potentiometer and sends it
 * to the IPC queue.
 */

extern xQueueHandle queue_h;
extern int QUEUE_LENGTH;

void analog_read_loop(void *para)
{
  	// analog potentiometer in ranges from about 600 to 3800.
  	const double FILTER_BETA = 0.01;
  	static uint16_t prev_result = USHRT_MAX;
  	long lValueToSend = 0;

  	while (1) {
        const ace_channel_handle_t current_channel = ACE_get_first_channel();
        const uint16_t adc_result = ACE_get_ppe_sample( current_channel );

        // analog reads from the pot are a bit noisy, so filter them with an LPF
        double filtered_result;
        if (prev_result != USHRT_MAX) {
          	filtered_result = FILTER_BETA * adc_result + (1 - FILTER_BETA) * prev_result;
        }
        else {
          	filtered_result = adc_result;
        }

        lValueToSend = round(filtered_result);
        if (uxQueueMessagesWaiting(queue_h) < QUEUE_LENGTH) {
			      const int xStatus = xQueueSendToBack(queue_h, &lValueToSend, 0);
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
        prev_result = filtered_result;
    }
}
