/* See COPYING file for license and copyright information */

#include <glib/gprintf.h>
#include "weechat-buffer.h"

/* Create a nicklist item */
nicklist_item_t* nicklist_item_create()
{
    nicklist_item_t* nicklist_item = g_try_malloc0(sizeof(nicklist_item_t));

    if (nicklist_item == NULL) {
        return NULL;
    }

    return nicklist_item;
}

void nicklist_item_delete(nicklist_item_t* nicklist_item)
{
    g_free(nicklist_item->name);
    g_free(nicklist_item->color);
    g_free(nicklist_item->prefix);
    g_free(nicklist_item->prefix_color);
    g_free(nicklist_item);
}

buffer_t* buffer_create(GVariant* buf)
{
    buffer_t* buffer = g_try_malloc0(sizeof(buffer_t));

    if (buffer == NULL) {
        return NULL;
    }
    GVariantDict* dict = g_variant_dict_new(buf);

    /* Extract GVariant dict to C struct */
    g_variant_dict_lookup(dict, "full_name", "s", &buffer->full_name);
    g_variant_dict_lookup(dict, "short_name", "s", &buffer->short_name);
    g_variant_dict_lookup(dict, "title", "s", &buffer->title);
    g_variant_dict_lookup(dict, "notify", "i", &buffer->notify);
    g_variant_dict_lookup(dict, "number", "i", &buffer->number);
    buffer->pointers = g_variant_dup_strv(
        g_variant_dict_lookup_value(dict, "__path", NULL), NULL);

    g_variant_dict_unref(dict);

    /* Create local variables hash table */
    buffer->local_variables = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);

    buffer->nicklist.groups = g_hash_table_new_full(g_str_hash, g_str_equal,
                                                    g_free, (GDestroyNotify)nicklist_item_delete);
    buffer->nicklist.nicks = g_hash_table_new_full(g_str_hash, g_str_equal,
                                                   g_free, (GDestroyNotify)nicklist_item_delete);

    return buffer;
}

void buffer_ui_init(buffer_t* buf)
{
    /* Load buffer layout from the Glade XML template */
    GtkBuilder* builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "ui/buffer.ui", NULL);

    buf->ui.buffer_layout = GTK_WIDGET(gtk_builder_get_object(builder, "buffer_layout"));
    buf->ui.tab_title = GTK_WIDGET(gtk_builder_get_object(builder, "buffer_title"));
    buf->ui.log_view = GTK_WIDGET(gtk_builder_get_object(builder, "log"));
    buf->ui.nick_adapt = GTK_WIDGET(gtk_builder_get_object(builder, "viewport"));
    buf->ui.nick_list = GTK_WIDGET(gtk_builder_get_object(builder, "nicklist"));
    buf->ui.entry = GTK_WIDGET(gtk_builder_get_object(builder, "entry"));

    /* Get the text buffer */
    buf->ui.textbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(buf->ui.log_view));

    /* Show the buffer title */
    gtk_label_set_text(GTK_LABEL(buf->ui.tab_title), buf->title);

    /* Set the widget name to the full_name to help the callback */
    gtk_widget_set_name(GTK_WIDGET(buf->ui.buffer_layout), buf->full_name);
    gtk_widget_set_name(GTK_WIDGET(buf->ui.entry), buf->full_name);

    /* Create the tab label */
    buf->ui.label = gtk_label_new(buffer_get_canonical_name(buf));
    gtk_label_set_width_chars(GTK_LABEL(buf->ui.label), 20);
    gtk_label_set_ellipsize(GTK_LABEL(buf->ui.label), PANGO_ELLIPSIZE_END);
}

void buffer_delete(buffer_t* buffer)
{
    g_free(buffer->full_name);
    g_free(buffer->short_name);
    g_free(buffer->title);
    g_strfreev(buffer->pointers);
    g_hash_table_unref(buffer->local_variables);
    g_free(buffer);
}

const gchar* buffer_get_canonical_name(buffer_t* buffer)
{
    if (buffer->short_name != NULL && g_strcmp0(buffer->short_name, "") != 0) {
        return buffer->short_name;
    } else {
        return buffer->full_name;
    }
}

void buffer_append_text(buffer_t* buffer, const gchar* prefix, const gchar* text)
{
    GtkTextMark* mark;
    GtkTextIter iter;

    gchar* str = g_strdup_printf("%s\t%s", prefix, text);

    /* Gtk buffer magic */
    mark = gtk_text_buffer_get_insert(buffer->ui.textbuf);
    gtk_text_buffer_get_iter_at_mark(buffer->ui.textbuf, &iter, mark);
    if (gtk_text_buffer_get_char_count(buffer->ui.textbuf))
        gtk_text_buffer_insert(buffer->ui.textbuf, &iter, "\n", 1);
    gtk_text_buffer_insert(buffer->ui.textbuf, &iter, str, -1);

    /* Scroll to the end of the text view */
    mark = gtk_text_buffer_create_mark(buffer->ui.textbuf, NULL, &iter, FALSE);
    gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(buffer->ui.log_view), mark, 0, FALSE, 0, 0);

    g_free(str);
}