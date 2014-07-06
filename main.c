/* See COPYING file for license and copyright information */

#include <glib.h>
#include "weechat-protocol.h"
#include "weechat-commands.h"

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

    weechat_cmd_init(weechat, "1234", FALSE);
    weechat_cmd_ping(weechat, "hello");
    weechat_cmd_test(weechat);
    weechat_cmd_info(weechat, NULL, "version");
    weechat_cmd_input(weechat, "core.weechat", "/print foobar");
    weechat_cmd_hdata(weechat, NULL, "buffer:gui_buffers(*)", "number,full_name");
    weechat_cmd_infolist(weechat, NULL, "buffer", NULL, NULL);
    weechat_cmd_nicklist(weechat, NULL, NULL);
    weechat_cmd_quit(weechat);

    return 0;
}
