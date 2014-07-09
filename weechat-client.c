/* See COPYING file for license and copyright information */

#include "weechat-client.h"
#include "weechat-commands.h"
#include "weechat-buffer.h"

void recv_thread(gpointer data)
{
    client_t* client = data;

    while (TRUE) {
        answer_t* answer = weechat_receive(client->weechat);
        GString* str = g_string_new("");
        g_string_append_printf(str, "Length: %zu\n", answer->length);
        g_string_append_printf(str, "Compression: %s\n", (answer->compression) ? "True" : "False");
        g_string_append_printf(str, "ID: %s\n", answer->id);
        g_string_append_printf(str, "%s\n", g_variant_print(answer->data.object, TRUE));

        gtk_text_buffer_set_text(client->ui.buffer, g_string_free(str, FALSE), -1);
        g_free(answer);
    }
}

static void cb_send(GtkWidget* widget, gpointer data)
{
    weechat_t* weechat = data;

    if (gtk_entry_get_text_length(GTK_ENTRY(widget)) > 0) {
        weechat_send(weechat, gtk_entry_get_text(GTK_ENTRY(widget)));
    }
    gtk_entry_set_text(GTK_ENTRY(widget), "");
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

static gboolean client_build_ui(client_t* client)
{
    GtkBuilder* builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "window.glade", NULL);

    client->ui.window = gtk_builder_get_object(builder, "window");
    g_signal_connect(client->ui.window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    client->ui.view = gtk_builder_get_object(builder, "textview");
    gtk_widget_override_font(GTK_WIDGET(client->ui.view),
                             pango_font_description_from_string("Monospace Regular 10"));
    client->ui.buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(client->ui.view));

    client->ui.entry = gtk_builder_get_object(builder, "entry");
    g_signal_connect(client->ui.entry, "activate", G_CALLBACK(cb_send), client->weechat);

    return TRUE;
}

static void client_build_buffer_map(client_t* client)
{
    client->buffers = g_hash_table_new_full(g_str_hash, buffer_equal,
                                            g_free, (GDestroyNotify)buffer_delete);

    GVariant* remote_bufs = weechat_cmd_hdata(client->weechat, NULL, "buffer:gui_buffers(*)",
                                              "local_variables,notify,number,full_name,short_name,title");
    GVariantIter iter;
    GVariant* child;

    g_variant_iter_init(&iter, remote_bufs);
    while ((child = g_variant_iter_next_value(&iter))) {
        buffer_t* buf = buffer_create(child);
        if (buf != NULL) {
            g_hash_table_insert(client->buffers, buf->full_name, buf);
        }
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

    weechat_cmd_init(client->weechat, password, TRUE);

    g_info("Running Weechat version %s",
           weechat_cmd_info(client->weechat, NULL, "version"));

    client_build_buffer_map(client);

    g_thread_new("wc-recv", (GThreadFunc) & recv_thread, client);

    return TRUE;
}
