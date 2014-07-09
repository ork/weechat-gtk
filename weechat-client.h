/* See COPYING file for license and copyright information */

#include <gtk/gtk.h>
#include "weechat-protocol.h"

struct client_s {
    weechat_t* weechat;
    struct {
        GObject* window;
        GObject* view;
        GObject* entry;
        GtkTextBuffer* buffer;
    } ui;
};
typedef struct client_s client_t;

client_t* client_create();

gboolean client_init(client_t* client, const gchar* host_and_port,
                     guint16 default_port, const gchar* password);
