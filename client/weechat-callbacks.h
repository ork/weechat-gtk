/* See COPYING file for license and copyright information */

#pragma once

#include <gtk/gtk.h>
#include "weechat-client.h"

/* Set the keyboard focus to the widget */
void cb_focusentry(GtkWidget* widget,
                   G_GNUC_UNUSED gpointer data);

/* Call the keyboard focus callback on tab switch */
void cb_tabswitch(G_GNUC_UNUSED GtkNotebook* notebook,
                  GtkWidget* page,
                  G_GNUC_UNUSED guint page_num,
                  G_GNUC_UNUSED gpointer user_data);

/* Handle text entry input */
void cb_input(GtkWidget* widget, gpointer data);
