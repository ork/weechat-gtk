/* See COPYING file for license and copyright information */

#include "weechat-commands.h"

void weechat_cmd_init(weechat_t* weechat, const gchar* password,
                      gboolean compression)
{
    gchar* msg = g_strdup_printf("init password=%s,compression=%s", password,
                                 (compression) ? "on" : "off");

    weechat_send(weechat, msg);
    g_free(msg);
}

void weechat_cmd_nicklist(weechat_t* weechat, const gchar* id, const gchar* buffer)
{
    GString* str = g_string_new("");
    gchar* msg;

    /* Construct */
    if (id != NULL) {
        g_string_append_printf(str, "(%s) ", id);
    }
    g_string_append(str, "nicklist");
    if (buffer != NULL) {
        g_string_append_printf(str, " %s", buffer);
    }
    msg = g_string_free(str, FALSE);

    /* Send */
    g_return_if_fail(weechat_send(weechat, msg));

    /* Process */
    weechat_receive(weechat);
}

void weechat_cmd_test(weechat_t* weechat)
{
    /* Send */
    g_return_if_fail(weechat_send(weechat, "test"));

    /* Process */
    weechat_receive(weechat);
}

void weechat_cmd_ping(weechat_t* weechat, const gchar* s)
{
    gchar* msg = g_strdup_printf("ping %s", s);

    g_return_if_fail(weechat_send(weechat, msg));
    weechat_receive(weechat);
    g_free(msg);
}

void weechat_cmd_quit(weechat_t* weechat)
{
    weechat_send(weechat, "quit");
}
