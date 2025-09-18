#pragma once
#include <stdbool.h>
typedef enum { BUTTON_EVENT_NONE=0, BUTTON_EVENT_PRESS, BUTTON_EVENT_LONG } button_event_t;
typedef void (*button_cb_t)(button_event_t ev);
void button_init(gpio_num_t gpio, button_cb_t cb);
