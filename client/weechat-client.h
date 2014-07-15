/* See COPYING file for license and copyright information */

#pragma once

#include <gtk/gtk.h>
#include "../lib/weechat-protocol.h"

struct client_s {
    weechat_t* weechat;
    struct {
        GObject* window;
        GObject* notebook;
    } ui;
    GHashTable* buffers;
    GHashTable* buf_ptrs;
};
typedef struct client_s client_t;

client_t* client_create();

gboolean client_init(client_t* client, const gchar* host_and_port,
                     guint16 default_port, const gchar* password);

gboolean client_build_ui(client_t* client);

void client_buffer_add(client_t* client, GVariant* received);

void client_load_existing_buffers(client_t* client);