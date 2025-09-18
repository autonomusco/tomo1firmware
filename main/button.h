#pragma once

#include "driver/gpio.h"   // for gpio_num_t

typedef enum {
    BUTTON_EVENT_PRESS,
    BUTTON_EVENT_LONG,
} button_event_t;

typedef void (*button_cb_t)(button_event_t ev);

void button_init(gpio_num_t gpio, button_cb_t cb);
