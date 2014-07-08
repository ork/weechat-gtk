/* See COPYING file for license and copyright information */

#include <glib.h>
#include <glib/gprintf.h>
#include <gio/gio.h>
#include <gio/gunixinputstream.h>
#include <gtk/gtk.h>

#include "weechat-protocol.h"
#include "weechat-commands.h"

struct test_s {
    weechat_t* weechat;
    struct {
        GObject* window;
        GObject* view;
        GObject* entry;
        GtkTextBuffer* buffer;
    } ui;
};

void recv_thread(gpointer data)
{
    struct test_s* test = data;

    while (TRUE) {
        answer_t* answer = weechat_receive(test->weechat);
        gtk_text_buffer_set_text(test->ui.buffer, g_variant_print(answer->data.object, TRUE), -1);
        g_free(answer);
    }
}

static void cb_send(GtkEntry* widget, gpointer data)
{
    weechat_t* weechat = data;

    if (gtk_entry_get_text_length(widget) > 0) {
        weechat_send(weechat, gtk_entry_get_text(widget));
    }
    gtk_entry_set_text(widget, "");
}

int main(int argc, char* argv[])
{
    struct test_s* test = g_malloc0(sizeof(struct test_s));
    GtkBuilder* builder;

    test->weechat = weechat_create();
    if (test->weechat == NULL) {
        return -1;
    }

    if (weechat_init(test->weechat, "localhost", 1234) == FALSE) {
        g_critical("Could not initialize weechat.");
        return -1;
    }

    gtk_init(&argc, &argv);

    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "window.glade", NULL);

    test->ui.window = gtk_builder_get_object(builder, "window");
    g_signal_connect(test->ui.window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    test->ui.view = gtk_builder_get_object(builder, "view");
    gtk_widget_override_font(test->ui.view, pango_font_description_from_string("Monospace Regular 10"));
    test->ui.buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(test->ui.view));

    test->ui.entry = gtk_builder_get_object(builder, "entry");
    g_signal_connect(test->ui.entry, "activate", G_CALLBACK(cb_send), test->weechat);

    g_thread_new("wc-recv", (GThreadFunc) & recv_thread, test);

    gtk_main();

    return 0;
}
