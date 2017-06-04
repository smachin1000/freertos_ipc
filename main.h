#ifndef MAIN_H_
#define MAIN_H_

#include <stdint.h>

typedef struct {
	xQueueHandle queue_h;
	uint16_t QUEUE_LENGTH;
} task_arg_t;

#endif /* MAIN_H_ */
