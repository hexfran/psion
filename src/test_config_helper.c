#include <stdio.h>
#include <string.h>

#include "config_helper.h"

int main5()
{
    char key[1024] = "port\0";
    char value[1024];

    get_config(key, value);        
    printf("Key: %s\nValue: %s\n\n", key, value);

    strcpy(key, "log");

    get_config(key, value);        
    printf("Key: %s\nValue: %s\n", key, value);

    return 0;
}
