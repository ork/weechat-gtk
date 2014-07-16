/* See COPYING file for license and copyright information */

#include "../lib/weechat-commands.h"
#include "weechat-callbacks.h"

void cb_focusentry(G_GNUC_UNUSED GtkNotebook* notebook,
                   GtkWidget* page,
                   G_GNUC_UNUSED guint page_num,
                   G_GNUC_UNUSED gpointer user_data)
{
    GList* list = gtk_container_get_children(GTK_CONTAINER(page));

    for (GList* l = list; l != NULL; l = l->next) {
        GtkWidget* widget = l->data;

        if (GTK_IS_ENTRY(widget)) {
            gtk_widget_grab_focus(widget);
            if (gtk_widget_is_focus(widget)) {
                g_printf("Should be focused\n");
            }
        }
    }
}

void cb_tabswitch(GtkNotebook* notebook,
                  GtkWidget* page,
                  G_GNUC_UNUSED guint page_num,
                  G_GNUC_UNUSED gpointer user_data)
{
    const gchar* tab_title = gtk_widget_get_name(page);

    /* Set window title */
    GtkWidget* toplevel = gtk_widget_get_toplevel(GTK_WIDGET(notebook));
    if (gtk_widget_is_toplevel(toplevel)) {
        gchar* win_title = g_strdup_printf("Weechat - %s", tab_title);
        gtk_window_set_title(GTK_WINDOW(toplevel), win_title);
        g_free(win_title);
    }
}

void cb_input(GtkWidget* widget, gpointer data)
{
    weechat_t* weechat = data;

    if (gtk_entry_get_text_length(GTK_ENTRY(widget)) > 0) {
        weechat_cmd_input(weechat,
                          gtk_widget_get_name(widget),
                          gtk_entry_get_text(GTK_ENTRY(widget)));
    }
    gtk_entry_set_text(GTK_ENTRY(widget), "");
}