#pragma once
#include <stdbool.h>

typedef struct { float ax, ay, az; float gx, gy, gz; } mpu6050_reading_t;
bool mpu6050_init(void);
mpu6050_reading_t mpu6050_read(void);
bool mpu6050_fall_detected(void); // simple threshold/orientation
