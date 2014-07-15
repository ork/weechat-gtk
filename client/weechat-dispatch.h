/* See COPYING file for license and copyright information */

#pragma once

#include "weechat-client.h"

struct dispatch_s {
    client_t** client;
    answer_t** answer;
};
typedef struct dispatch_s dispatch_t;

/* Check identifier to dispatch function call */
gboolean dispatcher(gpointer user_data);

/* A line hash been added to a buffer */
void client_dispatch_buffer_line_added(client_t* client, GVariant* gv);

/* A buffer has been closed */
void client_dispatch_buffer_closing(client_t* client, GVariant* gv);

/* A buffer has been opened */
void client_dispatch_buffer_opened(client_t* client, GVariant* gv);