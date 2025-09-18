idf_component_register(
  SRCS
    "app_main.c"
    "i2c_bus.c"
    "max17048.c"
    "mpu6050.c"
    "button.c"
    "ble.c"
  INCLUDE_DIRS "."
  REQUIRES driver esp_timer nvs_flash bt
)
