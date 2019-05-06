#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>

#include "error_helper.h"
#include "http_helper.h"

// BE CAREFUL, this function allocates a buffer that needs to be freed by
// the callee once done with it
// NOTE: headers is all the headers in a string like the following:
// Accept: text/plain;\nContent-Type: text/plain

char *build_http_resp(int statcode, char *headers, char *body)
{
    char intro[] = "HTTP/1.1 ";
    char content_len[32] = "Content-Length: ";
    char status[64]; 
    char tmp[32];

    // there's an extra 3 characters after intro for the "\n" character after 
    // the status and the two "\n\n" that separate headers from body
    // add extra space for the Content-Length header
    char *resp = malloc(strlen(intro) + 3 + sizeof(content_len) + strlen(headers) + sizeof(status) + strlen(body) + 1);
    
    memset(status, 0, sizeof(status));
    memset(resp, 0, sizeof(*resp));

    if(resp == NULL)
    {
        on_error_exit("Error allocating memory for http response, aborting");
    }

    switch(statcode)
    {
        case 200:
            strcpy(status,"200 OK\0");
            break;
        case 400:
            strcpy(status, "400 Bad Request\0");
            break;
        default:
            on_error_exit("Provided status not valid [%d], aborting", statcode);
    }

    sprintf(tmp, "%zu", strlen(body));

    strcat(content_len, tmp);

    resp = strcat(resp, "HTTP/1.1 ");
    resp = strcat(resp, status); // for example: HTTP/1.1 200 OK
    resp = strcat(resp, "\n");
    resp = strcat(resp, headers);
    resp = strcat(resp, "\n");
    resp = strcat(resp, content_len);  
    resp = strcat(resp, "\n\n");
    resp = strcat(resp, body);
    resp = strcat(resp, "\0");

    // printf("<>\n%s\n<>\n", resp);

    return resp;
}


ssize_t find_body_index(char *str, char *body)
{
    char delimiter[4] = "\r\n\r\n";

    if(strstr(str, delimiter) != NULL)
    {
        strcpy(body, (strstr(str, delimiter) + 4));
        return ((strstr(str, delimiter) - str) + 4);
    }

    memset(body, 0, sizeof(*body));
    body = NULL;

    return -1;
}
