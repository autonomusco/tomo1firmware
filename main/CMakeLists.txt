idf_component_register(SRCS
    "app_main.c"
    "ble.c"
    "ble_pairing.c"
    "button.c"
    "emergency_button.c"
    "fall_detection.c"
    "i2c_bus.c"
    "max17048.c"
    "mpu6050.c"
    "ota_update.c"
    "power_mgmt.c"
    "diagnostics.c"
    "cloud_api.c"
    "security.c"
    "voice_trigger.c"
    "health_sensors.c"   # <-- added this
    INCLUDE_DIRS ".")
