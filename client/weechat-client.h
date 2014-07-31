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

/* Create the client */
client_t* client_create();

/* Init the client */
gboolean client_init(client_t* client, const gchar* host_and_port,
                     guint16 default_port, const gchar* password);

/* Construct the base UI */
gboolean client_build_ui(client_t* client);

/* Add a buffer and tab to the client */
void client_buffer_add(client_t* client, GVariant* received);

/* Load existing remote buffers */
void client_load_existing_buffers(client_t* client);

void client_update_nicklists(gpointer key, gpointer value, gpointer user_data);