#ifndef JSON_HELPER_H
#define JSON_HELPER_H

#include <stdio.h>
#include <stdlib.h>

#include "cJSON.h"

// NOTE: value has to be provided by the callee and has to be an allocated
// buffer.
// MUST BE freed by the callee
void get_value_from_json(char *json, char *value, size_t size);

// NOTE: value has to be provided by the callee and has to be an allocated
// buffer.
// MUST BE freed by the callee
char *serialize_delim_str_to_json(char *delim_str, char *delim);

#endif
