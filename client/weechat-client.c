/* See COPYING file for license and copyright information */

#include "../lib/weechat-commands.h"

#include "weechat-client.h"
#include "weechat-callbacks.h"
#include "weechat-buffer.h"
#include "weechat-dispatch.h"

void recv_thread(gpointer data)
{
    dispatch_t* d = g_try_malloc0(sizeof(dispatch_t));
    client_t* client = data;
    d->client = &client;

    while (TRUE) {
        answer_t* answer = weechat_receive(client->weechat);
        d->answer = &answer;

        g_idle_add(dispatcher, d);
    }
}

client_t* client_create()
{
    client_t* client = g_try_malloc0(sizeof(client_t));

    if (client == NULL) {
        return NULL;
    }

    client->weechat = weechat_create();
    if (client->weechat == NULL) {
        return NULL;
    }

    return client;
}

gboolean client_build_ui(client_t* client)
{
    /* Load base layout from the Glade XML template */
    GtkBuilder* builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "ui/window.ui", NULL);

    client->ui.window = gtk_builder_get_object(builder, "window");
    g_signal_connect(client->ui.window, "destroy",
                     G_CALLBACK(gtk_main_quit), NULL);

    client->ui.notebook = gtk_builder_get_object(builder, "notebook");
    g_signal_connect(client->ui.notebook, "switch-page",
                     G_CALLBACK(cb_tabswitch), NULL);
    g_signal_connect(client->ui.notebook, "scroll-event",
                     G_CALLBACK(scroll_tab), NULL);
    gtk_widget_add_events(GTK_WIDGET(client->ui.notebook),
                          GDK_SCROLL_MASK | GDK_SMOOTH_SCROLL_MASK);

    /* Load the CSS */
    GtkCssProvider* provider = gtk_css_provider_new();
    GdkDisplay* display = gdk_display_get_default();
    GdkScreen* screen = gdk_display_get_default_screen(display);

    gtk_style_context_add_provider_for_screen(screen,
                                              GTK_STYLE_PROVIDER(provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    gtk_css_provider_load_from_path(GTK_CSS_PROVIDER(provider),
                                    "ui/custom.css", NULL);
    g_object_unref(provider);

    return TRUE;
}

void client_buffer_add(client_t* client, GVariant* received)
{
    buffer_t* buf = buffer_create(received);
    if (buf == NULL) {
        g_error("Could not add buffer\n");
    }

    /* Create map entries */
    g_hash_table_insert(client->buffers, buf->full_name, buf);
    g_hash_table_insert(client->buf_ptrs, buf->pointers[0], buf->full_name);

    /* Init the tab UI */
    buffer_ui_init(buf);

    /* Connect enter key with sending action */
    g_signal_connect(buf->ui.entry, "activate", G_CALLBACK(cb_input), client->weechat);

    /* Add the tab to the tab bar */
    gtk_notebook_insert_page(GTK_NOTEBOOK(client->ui.notebook),
                             GTK_WIDGET(buf->ui.buffer_layout), GTK_WIDGET(buf->ui.label), buf->number);
}

void client_load_existing_buffers(client_t* client)
{
    /* Request all existing buffers with useful data */
    GVariant* remote_bufs = weechat_cmd_hdata(client->weechat, NULL, "buffer:gui_buffers(*)",
                                              "local_variables,notify,number,full_name,short_name,title");
    GVariantIter iter;
    GVariant* child;

    /* For each buffer, load it */
    g_variant_iter_init(&iter, remote_bufs);
    while ((child = g_variant_iter_next_value(&iter))) {
        client_buffer_add(client, child);
        g_variant_unref(child);
    }
    g_variant_unref(remote_bufs);
}

gboolean client_init(client_t* client, const gchar* host_and_port,
                     guint16 default_port, const gchar* password)
{
    if (weechat_init(client->weechat, host_and_port, default_port) == FALSE) {
        g_critical("Could not initialize weechat.");
        return FALSE;
    }

    if (client_build_ui(client) == FALSE) {
        g_critical("Could not initialize GUI.");
        return FALSE;
    }

    /* Send password to initiate the connection */
    weechat_cmd_init(client->weechat, password, TRUE);

    g_info("Running Weechat version %s",
           weechat_cmd_info(client->weechat, NULL, "version"));

    /* Create (full_name -> buffer) map */
    client->buffers = g_hash_table_new_full(g_str_hash, g_str_equal, g_free,
                                            (GDestroyNotify)buffer_delete);
    /* Create (pointer -> full_name) map */
    client->buf_ptrs = g_hash_table_new(g_str_hash, g_str_equal);

    /* Load already openend weechat buffers */
    client_load_existing_buffers(client);

    /* Make all widgets visible */
    gtk_widget_show_all(GTK_WIDGET(client->ui.window));

    /* Request current nick list */
    weechat_send(client->weechat, "(_nicklist) nicklist");

    /* Request buffer sync */
    weechat_send(client->weechat, "sync");

    /* Start the reception thread */
    g_thread_new("wc-recv", (GThreadFunc) & recv_thread, client);

    return TRUE;
}

void client_update_nicklists(G_GNUC_UNUSED gpointer key,
                             gpointer value,
                             G_GNUC_UNUSED gpointer user_data)
{
    buffer_t* buffer = value;

    GHashTableIter iter;
    gpointer k, v;

    /* For each nick */
    g_hash_table_iter_init(&iter, buffer->nicklist.nicks);
    while (g_hash_table_iter_next(&iter, &k, &v)) {
        nicklist_item_t* nicklist_item = v;

        /* If it should be shown */
        if (nicklist_item->visible) {

            /* Construct nick label */
            GtkWidget* row = gtk_widget_new(GTK_TYPE_LABEL, "xalign", 0.,
                                                            "yalign", 0.,
                                                            "margin-left", 5.,
                                                            "margin-right", 3.,
                                                            NULL);
            gchar* str = g_markup_printf_escaped("<b><tt>%s</tt></b> %s",
                                                 nicklist_item->prefix,
                                                 nicklist_item->name);
            gtk_label_set_markup(GTK_LABEL(row), str);
            g_free(str);

            /* Add nick label */
            gtk_container_add(GTK_CONTAINER(buffer->ui.nick_list), row);
        }
    }

    /* Show all added labels */
    gtk_widget_show_all(GTK_WIDGET(buffer->ui.nick_list));
}
