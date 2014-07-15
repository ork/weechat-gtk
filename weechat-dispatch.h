/* See COPYING file for license and copyright information */

#pragma once

#include "weechat-client.h"

struct dispatch_s {
    client_t** client;
    answer_t** answer;
};
typedef struct dispatch_s dispatch_t;

gboolean dispatcher(gpointer user_data);

void client_dispatch_buffer_line_added(client_t* client, GVariant* gv);

void client_dispatch_buffer_closing(client_t* client, GVariant* gv);