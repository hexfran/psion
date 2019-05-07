#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "json_helper.h"

// NOTE: val has to be provided by the callee and has to be an allocated
// buffer.
// "size" is the size of the allocated string which will hold the value
// (if found)
// MUST BE freed by the callee
void get_value_from_json(char *json, char *val, size_t size)
{
    const cJSON *value = NULL;

    cJSON *parsed = cJSON_Parse(json);

    value = cJSON_GetObjectItemCaseSensitive(parsed, "value");

    if (cJSON_IsString(value) && (value->valuestring != NULL))
    {
        strncpy(val, value->valuestring, size);
    }
    else
    {
        memset(val, 0, size);
        value = NULL;
    }

    cJSON_Delete(parsed);
}

// NOTE: value has to be provided by the callee and has to be an allocated
// buffer.
// MUST BE freed by the callee
char *serialize_delim_str_to_json(char *delim_str, char *delim)
{
    char *json = NULL;

    char *t = strtok(delim_str, delim);

    cJSON *obj = NULL;
    cJSON *res = NULL;
    cJSON *tmp = NULL;

    obj = cJSON_CreateObject();

    if(obj == NULL)
    {
        cJSON_Delete(obj);
        goto clean;
    }

    res = cJSON_CreateArray();

    if(res == NULL)
    {
        cJSON_Delete(obj);
        goto clean;
    }

    cJSON_AddItemToObject(obj, "results", res);

    while(t != NULL)
    {
        tmp = cJSON_CreateString(t);

        if(tmp == NULL)
        {
            cJSON_Delete(obj);
            goto clean;
        }

        cJSON_AddItemToArray(res, tmp);

        t = strtok(NULL, delim);
    }

    json = cJSON_Print(obj);

clean:
    cJSON_Delete(obj);

    return json;
}
