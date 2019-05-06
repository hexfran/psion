#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <signal.h>

#include "config_helper.h"
#include "http_helper.h"
#include "error_helper.h"
#include "picohttpparser.h"
#include "trie.h"

#define DEFAULT_PORT 2091

void handle_sigint();
int visitor_print(const char *key, void *data, void *arg);

volatile int do_cleanup = 0;

int main()
{
    signal(SIGINT, handle_sigint);

    int port = DEFAULT_PORT;
    char key[] = "port";
    char value[1024];

    get_config(key, value);

    if(strlen(value) > 0)
    {
        port = atoi(value);
    }

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

    while (!do_cleanup)
    {
        socklen_t client_len = sizeof(client);
        client_fd = accept(server_fd, (struct sockaddr *) &client, &client_len);

        if (client_fd < 0 && !do_cleanup) on_error_exit("Could not establish new connection\n");

        while (!do_cleanup)
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

        if(do_cleanup) break;

        // reset variables to previous values
        buflen = 0;
        prevbuflen = 0;

        sprintf(_path, "%.*s", (int)path_len, path);

        if(strncmp(_path, "/retrieve", strlen("/retrieve")) == 0)
        {
            printf("> %s\n", (_path + strlen("/retrieve/")));

            trie_visit(t, (_path + strlen("/retrieve/")), visitor_print, resp);
            http_resp = build_http_resp(200, "Content-Type: text/plain\nAccept: text/plain", resp);
        }
        else if(strncmp(_path, "/insert", strlen("/insert")) == 0)
        {
            ssize_t body_index = find_body_index(buf); 
            char body[1024];

            memset(body, 0, sizeof(body));

            if(body_index > 0)
            {
                strncpy(body, (buf + body_index), sizeof(body));
                (void) trie_insert(t, (const char *) body, (void *) "data");
                http_resp = build_http_resp(200, "Content-Type: text/plain\nAccept: text/plain", "Value inserted");
            }
            else
            {
                err = write(client_fd, fail_resp, strlen(fail_resp));
                continue;
            }
        }
        else
        {
            printf("Operation not supported\n");
            err = write(client_fd, fail_resp, strlen(fail_resp));
            continue;
        }

        err = write(client_fd, http_resp, strlen(http_resp));
        if (err < 0) on_error_exit("Client write failed\n");

        memset(resp, 0, sizeof(*resp));
        memset(http_resp, 0, sizeof(*http_resp));
    }

    trie_free(t);
    free(http_resp);
    // free(method);
    // free(path);

    printf("Cleaned, exiting\n");

    return 0;
}

void handle_sigint(int sig)
{
    if(sig != SIGINT) return;
    printf("+ Caught SIGINT, cleaning up and aborting\n");
    do_cleanup = 1;
}

int visitor_print(const char *key, void *data, void *arg)
{
    (void) data;

    strcat((char *) arg, key); 
    strcat((char *) arg, "\n");

    return 0;
}
