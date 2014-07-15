/* See COPYING file for license and copyright information */

#include "weechat-dispatch.h"
#include "weechat-buffer.h"

gboolean dispatcher(gpointer user_data)
{
    dispatch_t* d = user_data;
    client_t* client = *(d->client);
    answer_t* answer = *(d->answer);

    /* Dispatch */
    if (g_strcmp0(answer->id, "_buffer_line_added") == 0) {
        client_dispatch_buffer_line_added(client, answer->data.object);
    } else if (g_strcmp0(answer->id, "_buffer_closing") == 0) {
        client_dispatch_buffer_closing(client, answer->data.object);
    } else if (g_strcmp0(answer->id, "_buffer_opened") == 0) {
        client_dispatch_buffer_opened(client, answer->data.object);
    } else {
        g_printf("Dispatcher: '%s' not handled\n", answer->id);
    }

    return G_SOURCE_REMOVE;
}

void client_dispatch_buffer_line_added(client_t* client, GVariant* gv)
{
    gchar* message;
    gchar* prefix;
    gchar* buf_ptr;

    GVariant* gvline = g_variant_get_child_value(
        g_variant_get_child_value(gv, 0), 0);

    /* Extract */
    GVariantDict* dict = g_variant_dict_new(gvline);
    g_variant_dict_lookup(dict, "message", "ms", &message);
    g_variant_dict_lookup(dict, "prefix", "ms", &prefix);
    g_variant_dict_lookup(dict, "buffer", "ms", &buf_ptr);
    g_variant_dict_unref(dict);

    /* Display */
    gchar* buf_name = g_hash_table_lookup(client->buf_ptrs, buf_ptr);
    buffer_t* buf = g_hash_table_lookup(client->buffers, buf_name);

    buffer_append_text(buf, prefix, message);

    g_free(message);
    g_free(prefix);
    g_free(buf_ptr);
}

void client_dispatch_buffer_closing(client_t* client, GVariant* gv)
{
    gchar* full_name;
    gchar* path;

    GVariant* gvline = g_variant_get_child_value(
        g_variant_get_child_value(gv, 0), 0);

    /* Extract */
    GVariantDict* dict = g_variant_dict_new(gvline);
    g_variant_dict_lookup(dict, "full_name", "ms", &full_name);
    g_variant_dict_lookup(dict, "__path", "ms", &path);
    g_variant_dict_unref(dict);

    g_hash_table_remove(client->buf_ptrs, path);
    g_hash_table_remove(client->buffers, full_name);

    g_free(full_name);
    g_free(path);

    g_error("GTK-side of buffer deletion not implemented\n");
}

void client_dispatch_buffer_opened(client_t* client, GVariant* gv)
{
    GVariant* gvline = g_variant_get_child_value(
        g_variant_get_child_value(gv, 0), 0);

    client_buffer_add(client, gvline);
    gtk_widget_show_all(GTK_WIDGET(client->ui.window));
}
