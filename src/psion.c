#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

#include "http_helper.h"
#include "error_helper.h"
#include "picohttpparser.h"
#include "trie.h"


int visitor_print(const char *key, void *data, void *arg)
{
    (void) data;

    // printf("%s\n", key);
    // written = write(*(int *) arg, key, strlen(key)); 
    strcat((char *) arg, key); 
    strcat((char *) arg, "\n");

    return 0;
}

int main()
{
    int port = 2091;

    int server_fd, client_fd, err;
    struct sockaddr_in server, client;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) on_error_exit("Could not create socket\n");

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    int opt_val = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof opt_val);

    err = bind(server_fd, (struct sockaddr *) &server, sizeof(server));
    if (err < 0) on_error_exit("Could not bind socket\n");

    err = listen(server_fd, 128);
    if (err < 0) on_error_exit("Could not listen on socket\n");

    printf("Server is listening on %d\n", port);

    struct trie *t = trie_create();

    (void) trie_insert(t, (const char *) "key1", (void *) "data1");
    (void) trie_insert(t, (const char *) "key2", (void *) "data2");
    (void) trie_insert(t, (const char *) "key3", (void *) "data3");
    (void) trie_insert(t, (const char *) "key4", (void *) "data4");
    (void) trie_insert(t, (const char *) "key5", (void *) "data5");
    (void) trie_insert(t, (const char *) "key6", (void *) "data6");
    (void) trie_insert(t, (const char *) "key7", (void *) "data7");
    (void) trie_insert(t, (const char *) "key8", (void *) "data8");
    (void) trie_insert(t, (const char *) "key9", (void *) "data9");

    char buf[4096], *method, *path;
    int pret, minor_version;
    struct phr_header headers[100];
    size_t buflen = 0, prevbuflen = 0, method_len, path_len, num_headers;
    ssize_t rret;

    char resp[4096];
    memset(resp, 0, sizeof(resp));

    char fail_resp[256] = "HTTP/1.1 400 Bad Request\nContent-Type: text/plain\nContent-Length: 4\n\nUBAD\0";

    char _path[1024];
    
    char *http_resp = NULL;

    while (1)
    {
        socklen_t client_len = sizeof(client);
        client_fd = accept(server_fd, (struct sockaddr *) &client, &client_len);

        if (client_fd < 0) on_error_exit("Could not establish new connection\n");

        while (1)
        {
            /* read the request */
            while ((rret = read(client_fd, buf + buflen, sizeof(buf) - buflen)) == -1 && errno == EINTR) {}
            if (rret <= 0)
                return -127;

            prevbuflen = buflen;
            buflen += rret;
            /* parse the request */
            num_headers = sizeof(headers) / sizeof(headers[0]);
            pret = phr_parse_request(buf, buflen, (const char **) &method, &method_len, (const char **) &path, &path_len,
                    &minor_version, headers, &num_headers, prevbuflen);
            if (pret > 0)
                break; /* successfully parsed the request */
            else if (pret == -1)
                return -128;
            /* request is incomplete, continue the loop */
            assert(pret == -2);
            if (buflen == sizeof(buf))
                return -129;
        }

        // reset variables to previous values
        buflen = 0;
        prevbuflen = 0;

        sprintf(_path, "%.*s", (int)path_len, path);

        printf("> %s\n", _path);

        if(strncmp(_path, "/retrieve", strlen("/retrieve")) == 0)
        {
            printf("Retrieve operation\n");
            trie_visit(t, "key", visitor_print, resp);
            http_resp = build_http_resp(200, "Content-Type: text/plain\nAccept: text/plain", resp);
        }
        else if(strncmp(_path, "/insert", strlen("/insert")) == 0)
        {
            printf("Insert operation\n");
            err = write(client_fd, fail_resp, strlen(fail_resp));
            continue;
        }
        else
        {
            printf("Operation not supported\n");
            err = write(client_fd, fail_resp, strlen(fail_resp));
            continue;
        }

        err = write(client_fd, http_resp, strlen(http_resp));
        if (err < 0) on_error_exit("Client write failed\n");
    }

    return 0;
}
