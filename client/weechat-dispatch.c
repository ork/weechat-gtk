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
    } else if (g_strcmp0(answer->id, "_buffer_renamed") == 0) {
        client_dispatch_buffer_renamed(client, answer->data.object);
    } else {
        g_printf("Dispatcher: '%s' not handled\n", answer->id);
        g_printf("%s\n", g_variant_print(answer->data.object, TRUE));
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

    /* Extract from ([]) */
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
    /* Extract from ([]) */
    GVariant* gvline = g_variant_get_child_value(
        g_variant_get_child_value(gv, 0), 0);

    client_buffer_add(client, gvline);
    gtk_widget_show_all(GTK_WIDGET(client->ui.window));
}

void client_dispatch_buffer_renamed(client_t* client, GVariant* gv)
{
    /* Extract from ([]) */
    GVariant* gvline = g_variant_get_child_value(
        g_variant_get_child_value(gv, 0), 0);

    /* Init dict parser */
    GVariantDict* dict = g_variant_dict_new(gvline);

    /* Parse the pointer array */
    gchar** ptrs = g_variant_dup_strv(
        g_variant_dict_lookup_value(dict, "__path", NULL), NULL);

    /* Retrieve the buffer pointed at */
    gchar* buf_name = g_hash_table_lookup(client->buf_ptrs, ptrs[0]);
    buffer_t* buf = g_hash_table_lookup(client->buffers, buf_name);

    /* Extract new names */
    g_variant_dict_lookup(dict, "full_name", "ms", &buf->full_name);
    g_variant_dict_lookup(dict, "short_name", "ms", &buf->short_name);

    /* Rename tab */
    gtk_label_set_text(GTK_LABEL(buf->ui.label), buffer_get_canonical_name(buf));

    g_variant_dict_unref(dict);
    g_strfreev(ptrs);
}