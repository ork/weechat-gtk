/* See COPYING file for license and copyright information */

#include <glib.h>
#include <gio/gio.h>
#include <gio/gunixinputstream.h>
#include "weechat-protocol.h"
#include "weechat-commands.h"

void repl_thread(gpointer data)
{
    weechat_t* weechat = data;

    GDataInputStream* stdin = g_data_input_stream_new(
        g_unix_input_stream_new(0, TRUE));

    g_printf("Interactive client for weechat-glib.\n");

    G_LOCK_DEFINE(m);
    while (TRUE) {
        gchar* in = g_data_input_stream_read_line(stdin, NULL, NULL, NULL);
        G_LOCK(m);
        weechat_send(weechat, in);
        G_UNLOCK(m);
        g_free(in);
    }
}

int main(int argc, char* argv[])
{
    weechat_t* weechat = weechat_create();
    if (weechat == NULL) {
        return -1;
    }

    if (weechat_init(weechat, "localhost", 1234) == FALSE) {
        g_critical("Could not initialize weechat.");
        return -1;
    }

    g_thread_new("wc-repl", (GThreadFunc) & repl_thread, weechat);

    while (TRUE) {
        weechat_receive(weechat);
    }

    return 0;
}
