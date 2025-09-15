#ifndef EMERGENCY_BUTTON_H
#define EMERGENCY_BUTTON_H

#include <stdbool.h>

// Initialize emergency button on given GPIO
bool emergency_button_init(int gpio);

#endif // EMERGENCY_BUTTON_H
