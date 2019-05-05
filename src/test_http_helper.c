#include <stdio.h>
#include <stdlib.h>

#include "http_helper.h"

int main3()
{
    char *resp = build_http_resp(200, "Content-Type: text/plain\nAccept: text/plain", "I AM OKAY"); 

    free(resp);
    return 0;
}
