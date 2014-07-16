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

    /* Create local variables hash table */
    buffer->local_variables = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);

    /* Create widgets */
    buffer->ui.label = gtk_label_new(buffer_get_canonical_name(buffer));
    buffer->ui.vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    buffer->ui.scroll = gtk_scrolled_window_new(0, 0);
    buffer->ui.textview = gtk_text_view_new();
    buffer->ui.entry = gtk_entry_new();

    return buffer;
}

void buffer_ui_init(buffer_t* buf)
{
    PangoFontDescription* font_desc = pango_font_description_from_string("Monospace 10");

    /* Create the text view */
    buf->ui.textbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(buf->ui.textview));
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(buf->ui.textview), GTK_WRAP_WORD);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(buf->ui.textview), FALSE);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(buf->ui.textview), FALSE);
    gtk_widget_override_font(buf->ui.textview, font_desc);
    gtk_widget_set_can_focus(buf->ui.textview, FALSE);

    /* Add the text view to the scrolling window */
    gtk_container_add(GTK_CONTAINER(buf->ui.scroll), buf->ui.textview);

    /* Add the scrolling window to the vertical box */
    gtk_box_pack_start(GTK_BOX(buf->ui.vbox), buf->ui.scroll, TRUE, TRUE, 0);

    /* Create the text entry */
    gtk_entry_set_has_frame(GTK_ENTRY(buf->ui.entry), FALSE);
    gtk_widget_set_can_default(buf->ui.entry, TRUE);
    g_object_set(buf->ui.entry, "activates-default", TRUE, NULL);

    /* Add the text entry to the vertical box */
    gtk_box_pack_end(GTK_BOX(buf->ui.vbox), buf->ui.entry, FALSE, FALSE, 0);

    /* Set the widget name to the full_name to help the callback */
    gtk_widget_set_name(GTK_WIDGET(buf->ui.entry), buf->full_name);

    gtk_label_set_width_chars(GTK_LABEL(buf->ui.label), 20);
    gtk_label_set_ellipsize(GTK_LABEL(buf->ui.label), PANGO_ELLIPSIZE_END);
    //gtk_misc_set_alignment(GTK_MISC(buf->ui.label), 1, 0);
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