#ifndef FALL_DETECTION_H
#define FALL_DETECTION_H
#include <stdbool.h>
void fall_sensor_init(void);
bool fall_detected(void);
void fall_task_start(void);
#endif
