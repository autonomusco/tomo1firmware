#pragma once
#include <stdbool.h>
void max17048_init(void);
float max17048_get_soc(void);     // 0..100
float max17048_get_voltage(void); // Volts
