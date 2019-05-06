#ifndef CONFIG_HELPER_H
#define CONFIG_HELPER_H

#include <stdio.h>

// value will be filled by the application, MUST BE an already-malloc'd buffer
// the value will be NULL on error
void get_config(char *key, char *value);
#endif
