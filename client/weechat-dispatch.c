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
    } else if (g_strcmp0(answer->id, "_buffer_title_changed") == 0) {
        client_dispatch_buffer_title_changed(client, answer->data.object);
    } else if (g_strcmp0(answer->id, "_buffer_localvar_added") == 0) {
        client_dispatch_buffer_localvar_added(client, answer->data.object);
    } else if (g_strcmp0(answer->id, "_buffer_localvar_removed") == 0) {
        client_dispatch_buffer_localvar_removed(client, answer->data.object);
    } else if (g_strcmp0(answer->id, "_nicklist") == 0) {
        client_dispatch_nicklist(client, answer->data.object);
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
    g_variant_dict_lookup(dict, "message", "s", &message);
    g_variant_dict_lookup(dict, "prefix", "s", &prefix);
    g_variant_dict_lookup(dict, "buffer", "s", &buf_ptr);
    g_variant_dict_unref(dict);

    /* Display */
    gchar* buf_name = g_hash_table_lookup(client->buf_ptrs, buf_ptr);
    buffer_t* buf = g_hash_table_lookup(client->buffers, buf_name);

    buffer_append_text(buf, prefix, message);

    /* Hilight tab */
    GdkRGBA red = { 1.0, 0.0, 0.0, 1.0 };
    gint cur = gtk_notebook_get_current_page(GTK_NOTEBOOK(client->ui.notebook));
    gint added = gtk_notebook_page_num(GTK_NOTEBOOK(client->ui.notebook), buf->ui.tab_layout);
    if (cur != added) {
        gtk_widget_override_color(buf->ui.label, GTK_STATE_FLAG_NORMAL, &red);
    }

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
    g_variant_dict_lookup(dict, "full_name", "s", &full_name);
    g_variant_dict_lookup(dict, "__path", "s", &path);
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
    g_variant_dict_lookup(dict, "full_name", "s", &buf->full_name);
    g_variant_dict_lookup(dict, "short_name", "s", &buf->short_name);

    /* Rename tab */
    gtk_label_set_text(GTK_LABEL(buf->ui.label), buffer_get_canonical_name(buf));

    g_variant_dict_unref(dict);
    g_strfreev(ptrs);
}

void client_dispatch_buffer_title_changed(client_t* client, GVariant* gv)
{
    /* Extract from ([]) */
    GVariant* gvline = g_variant_get_child_value(
        g_variant_get_child_value(gv, 0), 0);

    /* Init dict parser */
    GVariantDict* dict = g_variant_dict_new(gvline);

    /* Extract the full name of the buffer */
    gchar* full_name;
    g_variant_dict_lookup(dict, "full_name", "s", &full_name);

    /* Retrieve the buffer */
    buffer_t* buf = g_hash_table_lookup(client->buffers, full_name);

    /* Extract new title */
    g_variant_dict_lookup(dict, "title", "s", &buf->title);

    g_variant_dict_unref(dict);
    g_free(full_name);
}

void client_dispatch_buffer_localvar_added(client_t* client, GVariant* gv)
{
    /* Extract from ([]) */
    GVariant* gvline = g_variant_get_child_value(
        g_variant_get_child_value(gv, 0), 0);

    /* Init dict parser */
    GVariantDict* dict = g_variant_dict_new(gvline);
    gchar* full_name;
    g_variant_dict_lookup(dict, "full_name", "s", &full_name);

    /* Retrieve the buffer */
    buffer_t* buf = g_hash_table_lookup(client->buffers, full_name);

    /* Retrieve the local variables */
    GVariant* localvars = g_variant_dict_lookup_value(dict, "local_variables", NULL);
    GVariantIter iter;
    gchar* k, *v;

    g_variant_iter_init(&iter, localvars);
    while (g_variant_iter_next(&iter, "{ss}", &k, &v)) {
        g_hash_table_insert(buf->local_variables, k, v);
    }

    g_variant_dict_unref(dict);
    g_free(full_name);
}

void client_dispatch_buffer_localvar_removed(client_t* client, GVariant* gv)
{
    /* Extract from ([]) */
    GVariant* gvline = g_variant_get_child_value(
        g_variant_get_child_value(gv, 0), 0);

    /* Init dict parser */
    GVariantDict* dict = g_variant_dict_new(gvline);
    gchar* full_name;
    g_variant_dict_lookup(dict, "full_name", "s", &full_name);

    /* Retrieve the buffer */
    buffer_t* buf = g_hash_table_lookup(client->buffers, full_name);

    /* Remove all local variables */
    g_hash_table_remove_all(buf->local_variables);

    /* Retrieve the local variables */
    GVariant* localvars = g_variant_dict_lookup_value(dict, "local_variables", NULL);
    GVariantIter iter;
    gchar* k, *v;

    g_variant_iter_init(&iter, localvars);
    while (g_variant_iter_next(&iter, "{ss}", &k, &v)) {
        g_hash_table_insert(buf->local_variables, k, v);
    }

    g_variant_dict_unref(dict);
    g_free(full_name);
}

void client_dispatch_nicklist(client_t* client, GVariant* gv)
{
    /* Extract from () */
    GVariant* gvline = g_variant_get_child_value(gv, 0);

    GVariantIter iter;
    GVariant* child;

    /* For each nick/group */
    g_variant_iter_init(&iter, gvline);
    while ((child = g_variant_iter_next_value(&iter))) {
        nicklist_item_t* nicklist_item = nicklist_item_create();

        gchar group, visible;
        GVariant* path;

        GVariantDict* dict = g_variant_dict_new(child);

        g_variant_dict_lookup(dict, "prefix", "s", &nicklist_item->prefix);
        g_variant_dict_lookup(dict, "name", "s", &nicklist_item->name);
        g_variant_dict_lookup(dict, "level", "i", &nicklist_item->level);
        g_variant_dict_lookup(dict, "visible", "y", &visible);
        nicklist_item->visible = (visible == 1);
        g_variant_dict_lookup(dict, "group", "y", &group);
        path = g_variant_dict_lookup_value(dict, "__path", NULL);

        const gchar** paths = g_variant_get_strv(path, NULL);

        buffer_t* buf = g_hash_table_lookup(client->buffers,
                                            g_hash_table_lookup(client->buf_ptrs, paths[0]));

        /* Add the nick/group to its buffer's list */
        if (group == 0) {
            g_hash_table_insert(buf->nicklist.nicks, nicklist_item->name, nicklist_item);
        } else {
            g_hash_table_insert(buf->nicklist.groups, nicklist_item->name, nicklist_item);
        }

        g_variant_dict_unref(dict);
        g_variant_unref(child);
    }

    /* Update UI */
    g_hash_table_foreach(client->buffers, client_update_nicklists, NULL);
}
