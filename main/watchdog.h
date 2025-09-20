#pragma once
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void watchdog_init(void);
bool watchdog_enable_task(void);
bool watchdog_disable_task(void);
bool watchdog_feed(void);
void watchdog_self_test(void);

#ifdef __cplusplus
}
#endif
