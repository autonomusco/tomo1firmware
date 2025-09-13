message(STATUS ">>> Entering main/CMakeLists.txt <<<")

idf_component_register(
    SRCS
        app_main.c
        ble_pairing.c
        cloud_api.c
        diagnostics.c
        emergency_button.c
        fall_detection.c
        health_sensors.c
        i2c_bus.c
        ota_update.c
        power_mgmt.c
        security.c
        voice_trigger.c

    INCLUDE_DIRS
        .
)
