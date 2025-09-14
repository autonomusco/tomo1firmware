#ifndef BUTTON_H
#define BUTTON_H

#include <stdbool.h>

/**
 * Initialize the emergency button GPIO and ISR.
 * Returns true if successful, false otherwise.
 */
bool button_init(int gpio_num);

#endif // BUTTON_H
