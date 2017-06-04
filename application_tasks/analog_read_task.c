#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <freertos.h>
#include <task.h>
#include <queue.h>
#include <math.h>

#include "../drivers/mss_ace/mss_ace.h"

#include "FreeRTOS.h"

/**
 * This task reads the analog input value from the potentiometer and sends it
 * to the IPC queue.
 */
extern int QUEUE_LENGTH;


#define STOPPER 0                                      /* Smaller than any datum */
#define MEDIAN_FILTER_SIZE	(5)

uint16_t median_filter(uint16_t datum)
{
 struct pair
 {
   struct pair   *point;                              /* Pointers forming list linked in sorted order */
   uint16_t  value;                                   /* Values to sort */
 };
 static struct pair buffer[MEDIAN_FILTER_SIZE] = {};  /* Buffer of nwidth pairs */
 static struct pair *datpoint = buffer;               /* Pointer into circular buffer of data */
 static struct pair small = {NULL, STOPPER};          /* Chain stopper */
 static struct pair big = {&small, 0};                /* Pointer to head (largest) of linked list.*/

 struct pair *successor;                              /* Pointer to successor of replaced data item */
 struct pair *scan;                                   /* Pointer used to scan down the sorted list */
 struct pair *scanold;                                /* Previous value of scan */
 struct pair *median;                                 /* Pointer to median */
 uint16_t i;

 if (datum == STOPPER)
 {
   datum = STOPPER + 1;                             /* No stoppers allowed. */
 }

 if ((++datpoint - buffer) >= MEDIAN_FILTER_SIZE)
 {
   datpoint = buffer;                               /* Increment and wrap data in pointer.*/
 }

 datpoint->value = datum;                           /* Copy in new datum */
 successor = datpoint->point;                       /* Save pointer to old value's successor */
 median = &big;                                     /* Median initially to first in chain */
 scanold = NULL;                                    /* Scanold initially null. */
 scan = &big;                                       /* Points to pointer to first (largest) datum in chain */

 /* Handle chain-out of first item in chain as special case */
 if (scan->point == datpoint)
 {
   scan->point = successor;
 }
 scanold = scan;                                     /* Save this pointer and   */
 scan = scan->point ;                                /* step down chain */

 /* Loop through the chain, normal loop exit via break. */
 for (i = 0 ; i < MEDIAN_FILTER_SIZE; ++i)
 {
   /* Handle odd-numbered item in chain  */
   if (scan->point == datpoint)
   {
     scan->point = successor;                      /* Chain out the old datum.*/
   }

   if (scan->value < datum)                        /* If datum is larger than scanned value,*/
   {
     datpoint->point = scanold->point;             /* Chain it in here.  */
     scanold->point = datpoint;                    /* Mark it chained in. */
     datum = STOPPER;
   };

   /* Step median pointer down chain after doing odd-numbered element */
   median = median->point;                       /* Step median pointer.  */
   if (scan == &small)
   {
     break;                                      /* Break at end of chain  */
   }
   scanold = scan;                               /* Save this pointer and   */
   scan = scan->point;                           /* step down chain */

   /* Handle even-numbered item in chain.  */
   if (scan->point == datpoint)
   {
     scan->point = successor;
   }

   if (scan->value < datum)
   {
     datpoint->point = scanold->point;
     scanold->point = datpoint;
     datum = STOPPER;
   }

   if (scan == &small) {
     break;
   }

   scanold = scan;
   scan = scan->point;
 }
 return median->value;
}

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
