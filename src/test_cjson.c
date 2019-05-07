#include <stdio.h>
#include <stdlib.h>

#include "cJSON.h"

char *gen_json();

int main7()
{
    char *json = gen_json();
 
    if(json != NULL) printf("%s\n", json);

    free(json);

    char parse[64] = "{\"value\": \"insert me\"}";

    printf("%s\n", parse);

    const cJSON *value = NULL;
    cJSON *parsed = cJSON_Parse(parse);

    value = cJSON_GetObjectItemCaseSensitive(parsed, "value");
    if (cJSON_IsString(value) && (value->valuestring != NULL))
    {
        printf("Value is \"%s\"\n", value->valuestring);
    }

    cJSON_Delete(parsed);

    return 0;
}

char *gen_json()
{
    cJSON *obj = NULL;
    cJSON *res = NULL;
    cJSON *tmp = NULL;

    char val[32];

    char *str = NULL;

    int i = 0;

    obj = cJSON_CreateObject();

    if(obj == NULL)
    {
        cJSON_Delete(obj);
        return NULL;
    }

    res = cJSON_CreateArray();

    if(res == NULL)
    {
        cJSON_Delete(obj);
        return NULL;
    }

    cJSON_AddItemToObject(obj, "values", res);

    for(i = 0; i < 7; i++)
    {
        sprintf(val, "value %d", i);
        tmp = cJSON_CreateString(val);

        if(tmp == NULL)
        {
            cJSON_Delete(obj);
            return NULL;
        }

        cJSON_AddItemToArray(res, tmp);
    }

    str = cJSON_Print(obj);

    cJSON_Delete(obj);
    
    return str;
}
