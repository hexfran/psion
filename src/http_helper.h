#ifndef HTTP_HELPER_H
#define HTTP_HELPER_H

// BE CAREFUL, this function allocates a buffer that needs to be freed by
// the callee once done with it
// NOTE: headers is all the headers in a string like the following:
// Accept: text/plain;\nContent-Type: text/plain
char *build_http_resp(int statcode, char *headers, char *body);

#endif