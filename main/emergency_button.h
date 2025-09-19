#pragma once
#include "driver/gpio.h"
#include <stdbool.h>

typedef enum {
    EMERGENCY_EVENT_PRESS,
    EMERGENCY_EVENT_LONG
} emergency_event_t;

typedef void (*emergency_cb_t)(emergency_event_t ev);

/**
 * Initialize emergency button on given GPIO pin.
 * Returns true on success.
 */
bool emergency_button_init(gpio_num_t gpio, emergency_cb_t cb);
