message(STATUS ">>> Entering main/CMakeLists.txt <<<")

# Automatically gather all .c files in this folder
file(GLOB SOURCES "*.c")

# Register the component with ESP-IDF
idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS "."
)
