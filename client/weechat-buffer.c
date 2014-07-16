/* See COPYING file for license and copyright information */

#include <glib/gprintf.h>
#include "weechat-buffer.h"

buffer_t* buffer_create(GVariant* buf)
{
    buffer_t* buffer = g_try_malloc0(sizeof(buffer_t));

    if (buffer == NULL) {
        return NULL;
    }
    GVariantDict* dict = g_variant_dict_new(buf);

    /* Extract GVariant dict to C struct */
    g_variant_dict_lookup(dict, "full_name", "ms", &buffer->full_name);
    g_variant_dict_lookup(dict, "short_name", "ms", &buffer->short_name);
    g_variant_dict_lookup(dict, "title", "ms", &buffer->title);
    g_variant_dict_lookup(dict, "notify", "i", &buffer->notify);
    g_variant_dict_lookup(dict, "number", "i", &buffer->number);
    buffer->pointers = g_variant_dup_strv(
        g_variant_dict_lookup_value(dict, "__path", NULL), NULL);

    g_variant_dict_unref(dict);

    return buffer;
}

void buffer_delete(buffer_t* buffer)
{
    g_free(buffer->full_name);
    g_free(buffer->short_name);
    g_free(buffer->title);
    g_strfreev(buffer->pointers);
    g_free(buffer);
}

const gchar* buffer_get_canonical_name(buffer_t* buffer)
{
    if (buffer->short_name != NULL) {
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
    gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(buffer->ui.textview), mark, 0, FALSE, 0, 0);

    g_free(str);
}