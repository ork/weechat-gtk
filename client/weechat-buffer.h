/* See COPYING file for license and copyright information */

#pragma once

#include <glib.h>
#include <gtk/gtk.h>

struct buffer_s {
    gchar** pointers;
    gchar* full_name;
    gchar* short_name;
    gchar* title;
    gint32 notify;
    gint32 number;
    GHashTable* local_variables;
    struct {
        GtkWidget* label;
        GtkWidget* tab_layout;
        GtkWidget* tab_title;
        GtkWidget* log_and_nick;
        GtkWidget* log_scroll;
        GtkWidget* log_view;
        GtkWidget* sep;
        GtkWidget* nick_scroll;
        GtkWidget* nick_adapt;
        GtkWidget* nick_list;
        GtkWidget* entry;
        GtkTextBuffer* textbuf;
    } ui;
};
typedef struct buffer_s buffer_t;

/* Create a buffer */
buffer_t* buffer_create(GVariant* buf);

/* Init the UI of the buffer */
void buffer_ui_init(buffer_t* buf);

/* Delete a buffer */
void buffer_delete(buffer_t* buffer);

/* Get the canonical name of a buffer */
const gchar* buffer_get_canonical_name(buffer_t* buffer);

/* Append (optionally) prefixed text to a buffer */
void buffer_append_text(buffer_t* buffer, const gchar* prefix, const gchar* text);
