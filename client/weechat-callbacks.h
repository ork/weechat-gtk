/* See COPYING file for license and copyright information */

#pragma once

#include <gtk/gtk.h>
#include "weechat-client.h"

/* Change buffers when scrolling over the tabs */
gboolean
scroll_tab(GtkWidget* widget,
           GdkEvent* event,
           gpointer user_data);

/* Call the keyboard focus callback on tab switch */
void cb_tabswitch(GtkNotebook* notebook,
                  GtkWidget* page,
                  guint page_num,
                  gpointer user_data);

/* Handle text entry input */
void cb_input(GtkWidget* widget, gpointer data);
