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
    gtk_builder_add_from_file(builder, "window.glade", NULL);

    client->ui.window = gtk_builder_get_object(builder, "window");
    g_signal_connect(client->ui.window, "destroy",
                     G_CALLBACK(gtk_main_quit), NULL);

    client->ui.notebook = gtk_builder_get_object(builder, "notebook");
    g_signal_connect(client->ui.notebook, "switch-page",
                     G_CALLBACK(cb_tabswitch), NULL);

    return TRUE;
}

void client_buffer_add(client_t* client, GVariant* received)
{
    PangoFontDescription* font_desc = pango_font_description_from_string("Monospace 10");

    buffer_t* buf = buffer_create(received);
    if (buf == NULL) {
        g_error("Could not add buffer\n");
    }

    /* Create map entries */
    g_hash_table_insert(client->buffers, buf->full_name, buf);
    g_hash_table_insert(client->buf_ptrs, buf->pointers[0], buf->full_name);

    /* Create widgets */
    buf->ui.label = gtk_label_new(buffer_get_canonical_name(buf));
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget* scro = gtk_scrolled_window_new(0, 0);
    GtkWidget* tv = gtk_text_view_new();
    buf->ui.entry = gtk_entry_new();

    /* Create the text view */
    buf->text_buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(tv));
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(tv), GTK_WRAP_WORD);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(tv), FALSE);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(tv), FALSE);
    gtk_widget_override_font(tv, font_desc);
    gtk_widget_set_can_focus(tv, FALSE);

    /* Add the text view to the scrolling window */
    gtk_container_add(GTK_CONTAINER(scro), tv);

    /* Add the scrolling window to the vertical box */
    gtk_box_pack_start(GTK_BOX(vbox), scro, TRUE, TRUE, 0);

    /* Create the text entry */
    gtk_entry_set_has_frame(GTK_ENTRY(buf->ui.entry), FALSE);
    gtk_widget_set_can_default(buf->ui.entry, TRUE);
    g_object_set(buf->ui.entry, "activates-default", TRUE, NULL);
    g_signal_connect(buf->ui.entry, "activate", G_CALLBACK(cb_input), client->weechat);

    /* Add the text entry to the vertical box */
    gtk_box_pack_end(GTK_BOX(vbox), buf->ui.entry, FALSE, FALSE, 0);

    /* Set the widget name to the full_name to help the callback */
    gtk_widget_set_name(GTK_WIDGET(buf->ui.entry), buf->full_name);

    gtk_label_set_width_chars(GTK_LABEL(buf->ui.label), 20);
    gtk_label_set_ellipsize(GTK_LABEL(buf->ui.label), PANGO_ELLIPSIZE_END);
    //gtk_misc_set_alignment(GTK_MISC(buf->ui.label), 1, 0);

    gtk_notebook_insert_page(GTK_NOTEBOOK(client->ui.notebook),
                             GTK_WIDGET(vbox), GTK_WIDGET(buf->ui.label), -1);
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

    /* Request buffer sync */
    weechat_send(client->weechat, "sync");

    /* Start the reception thread */
    g_thread_new("wc-recv", (GThreadFunc) & recv_thread, client);

    return TRUE;
}
