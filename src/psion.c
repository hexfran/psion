/* Compatibility for possible missing IPv6 declarations */
#include "../libs/libevent/util-internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <signal.h>

#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/keyvalq_struct.h>
#include <event2/http_struct.h>

#ifdef EVENT__HAVE_NETINET_IN_H
#include <netinet/in.h>
# ifdef _XOPEN_SOURCE_EXTENDED
#  include <arpa/inet.h>
# endif
#endif

#include "json_helper.h"
#include "error_helper.h"
#include "trie.h"

#define DEFAULT_SEPARATOR ">|"

struct options
{
    int port;
    int iocp;
    int verbose;
};

struct sized_buf
{
    char *buf;
    size_t occupied;
    size_t allocated;
};

int visitor_print(const char *key, void *data, void *arg)
{
    (void) data;
 
    struct sized_buf *bf = ((struct sized_buf *) arg);

    size_t key_size = strlen(key) + 1;
    size_t occupied = bf->occupied;
    size_t allocated = bf->allocated;

    if((key_size + occupied + strlen(DEFAULT_SEPARATOR)) >= allocated)
    {
        // we don't fit, reallocate and update the allocated size
        bf->buf = realloc(bf->buf, allocated * 2);  
       
        if(bf->buf == NULL)
        {
            printf("Error reallocating buffer\n");
            free(bf);

            return -129;
        }

        bf->allocated *= 2;
    }

    bf->occupied += key_size;
    bf->occupied += strlen(DEFAULT_SEPARATOR);

    strcat(bf->buf, key); 
    strcat(bf->buf, DEFAULT_SEPARATOR);

    return 0;
}

static void fronting_cb(struct evhttp_request *req, void *arg)
{
    const char *uri = evhttp_request_get_uri(req);

    if(strncmp(uri, "/retrieve/", strlen("/retrieve/")) == 0)
    {
        struct evbuffer *buf = evbuffer_new();

        struct sized_buf *resp = malloc(sizeof(struct sized_buf));

        // start with a basic dimension of 4K, expanded later if necessary
        resp->buf = calloc(1, 4096 * sizeof(char));
        resp->occupied = 0;
        resp->allocated = 4096;

        char *res_json = NULL;

        trie_visit((struct trie *) arg, (uri + strlen("/retrieve/")), visitor_print, resp);

        res_json = serialize_delim_str_to_json(resp->buf, DEFAULT_SEPARATOR); 

        evbuffer_add(buf, res_json, strlen(res_json));

        memset(res_json, 0, sizeof(*res_json));

        free(resp->buf);
        free(resp);
        free(res_json);

        res_json = NULL;
        resp = NULL;

        evhttp_send_reply(req, 200, "OK", buf);

        evbuffer_free(buf);
    }
    else
    {
        evhttp_send_reply(req, 404, "Not Found", NULL);
    }
}

static void insert_cb(struct evhttp_request *req, void *arg)
{
    char success_res[] = "{\"msg\": \"Ok\"}"; 

    if(evhttp_request_get_command(req) != EVHTTP_REQ_POST)
    {
        evhttp_send_reply(req, 405, "Method Not Allowed", NULL);
        return;
    }

    char *body = NULL;
    char deserialized[1024] = {0};

    struct evbuffer *buf;
    struct evbuffer *resp = evbuffer_new();

    evhttp_add_header(req->output_headers, "Content-Type", "application/json");  

    buf = evhttp_request_get_input_buffer(req);

    size_t bytes = evbuffer_get_length(buf);

    /*
     * NOTE IMPORTANT: Using malloc and later memset was giving problems with
     * Valgrind, complaining about "body" not being initialized correctly,
     * specifically reporting: "Conditional move or jump depdens on
     * uninitialized data"
     *
     * Not sure why this is the case, never gave problems before, but using
     * calloc solved the issue and that is good for now.
     * To be investigated later, definitely
     */
    // extra 1 char for '\0'
    body = calloc(1, bytes + 1);

    if(body == NULL)
    {
        evbuffer_free(resp);
        evhttp_send_reply(req, 500, "Internal Server Error", NULL);

        return;
    }

    if(evbuffer_remove(buf, body, bytes) == -1)
    {
        evbuffer_free(resp);
        evhttp_send_reply(req, 500, "Internal Server Error", NULL);

        return;
    }
    
    get_value_from_json(body, deserialized, sizeof(deserialized)); 

    if(deserialized == NULL || strlen(deserialized) < 1)
    {
        evbuffer_free(resp);

        evhttp_send_reply(req, 500, "Internal Server Error", NULL);

        free(body);

        return;
    }

    if(deserialized != NULL && strlen(deserialized) >= 1)
    {
        (void) trie_insert((struct trie *) arg, deserialized, (void *) "data");
        
        evbuffer_add(resp, success_res, strlen(success_res));

        evhttp_send_reply(req, 200, "OK", resp);

        evbuffer_free(resp);

        free(body);

        return;
    }

    evbuffer_free(resp);
    free(body);

    evhttp_send_reply(req, 500, "Internal Server Error", NULL);
}

static struct options
parse_opts(int argc, char **argv)
{
    struct options o;
    int opt;

    memset(&o, 0, sizeof(o));

    while ((opt = getopt(argc, argv, "p:Iv")) != -1) {
        switch (opt) {
            case 'p': o.port = atoi(optarg); break;
            case 'I': o.iocp = 1; break;
            case 'v': ++o.verbose; break;
            default : fprintf(stderr, "Unknown option %c\n", opt); break;
        }
    }

    return o;
}

static void
do_term(int sig, short events, void *arg)
{
    (void) events;
    struct event_base *base = arg;
    event_base_loopbreak(base);
    fprintf(stderr, "Got %i, Terminating\n", sig);
}

int
main(int argc, char **argv)
{
    struct event_config *cfg = NULL;
    struct event_base *base = NULL;
    struct evhttp *http = NULL;
    struct evhttp_bound_socket *handle = NULL;
    struct event *term = NULL;
    struct options o = parse_opts(argc, argv);
    int ret = 0;

    struct trie *t = NULL;

    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        ret = 1;
        goto err;
    }

    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    t = trie_create();

    /** Read env like in regress" */
    if (o.verbose || getenv("EVENT_DEBUG_LOGGING_ALL"))
        event_enable_debug_logging(EVENT_DBG_ALL);

    cfg = event_config_new();

    base = event_base_new_with_config(cfg);
    if (!base) {
        fprintf(stderr, "Couldn't create an event_base: exiting\n");
        ret = 1;
    }
    event_config_free(cfg);
    cfg = NULL;

    /* Create a new evhttp object to handle requests. */
    http = evhttp_new(base);
    if (!http) {
        fprintf(stderr, "couldn't create evhttp. Exiting.\n");
        ret = 1;
    }

    /*
     * Set the endpoints
     */ 
    // evhttp_set_cb(http, "/retrieve", retrieve_cb, NULL);

    evhttp_set_cb(http, "/insert", insert_cb, t);

    /* We want to accept arbitrary requests, so we need to set a "generic"
     * cb.  We can also add callbacks for specific paths. */
    evhttp_set_gencb(http, fronting_cb, t);

    /* Now we tell the evhttp what port to listen on */
    handle = evhttp_bind_socket_with_handle(http, "0.0.0.0", o.port);
    if (!handle) {
        fprintf(stderr, "couldn't bind to port %d. Exiting.\n", o.port);
        ret = 1;
        goto err;
    }

    {
        /* Extract and display the address we're listening on. */
        struct sockaddr_storage ss;
        evutil_socket_t fd;
        ev_socklen_t socklen = sizeof(ss);
        char addrbuf[128];
        void *inaddr;
        const char *addr;
        int got_port = -1;
        fd = evhttp_bound_socket_get_fd(handle);
        memset(&ss, 0, sizeof(ss));
        if (getsockname(fd, (struct sockaddr *)&ss, &socklen)) {
            perror("getsockname() failed");
            ret = 1;
        }
        if (ss.ss_family == AF_INET) {
            got_port = ntohs(((struct sockaddr_in*)&ss)->sin_port);
            inaddr = &((struct sockaddr_in*)&ss)->sin_addr;
        } else if (ss.ss_family == AF_INET6) {
            got_port = ntohs(((struct sockaddr_in6*)&ss)->sin6_port);
            inaddr = &((struct sockaddr_in6*)&ss)->sin6_addr;
        } else {
            fprintf(stderr, "Weird address family %d\n",
                    ss.ss_family);
            ret = 1;
            goto err;
        }
        addr = evutil_inet_ntop(ss.ss_family, inaddr, addrbuf,
                sizeof(addrbuf));
        if (addr) {
            printf("Listening on %s:%d\n", addr, got_port);
        } else {
            fprintf(stderr, "evutil_inet_ntop failed\n");
            ret = 1;
            goto err;
        }
    }

    term = evsignal_new(base, SIGINT, do_term, base);

    if (!term)
        goto err;
    if (event_add(term, NULL))
        goto err;

    event_base_dispatch(base);

err:
    if (cfg)
        event_config_free(cfg);
    if (http)
        evhttp_free(http);
    if (term)
        event_free(term);
    if (base)
        event_base_free(base);
    if(t)
        trie_free(t);
    

    return ret;
}
