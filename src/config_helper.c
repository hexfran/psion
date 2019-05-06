#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "error_helper.h"
#include "config_helper.h"

ssize_t char_pos(char *str, char ch);

// value will be filled by the application, MUST BE an already-malloc'd buffer
// the value will be NULL on error
void get_config(char *key, char *value)
{
    // try opening the default config path
    FILE *fp = fopen("../conf/config.txt", "r");
    ssize_t index = -1;

    char buf[1024];
    char tmp[1024];

    if(fp == NULL)
    {
        fp = fopen("conf/config.txt", "r");
        if(fp == NULL)
        {
            on_error_exit("Could not open config file, aborting\n");
        }
    }

    memset(value, 0, sizeof(*value));

    while(fgets(buf, sizeof(buf), fp) != NULL)
    {
        memset(tmp, 0, sizeof(tmp));
        // remove newline '\n' char
        buf[strlen(buf) - 1] = 0;
        
        if(buf[0] != '#'
                && buf[0] != ' '
                && strlen(buf) > 1)
        {
            if((index = char_pos(buf, '=')) >= 0)
            {
                strncpy(tmp, buf, index);

                if(strcmp(tmp, key) == 0)
                {
                    // extra 1 to exclude the '=' character
                    strcpy(value, (buf + index + 1));
                }     
            }
            else
            {
                value = "\0";
            } 
        }
    }

    fclose(fp);
}

ssize_t char_pos(char *str, char ch)
{
    ssize_t i = 0;
    char *copy = str;

    while(*copy)
    {
        if(*copy == ch) return i;
        
        ++copy;
        ++i;
    }

    // check last character just in case
    if(copy[i] != ch) return -1;

    return i;
}
