/* See COPYING file for license and copyright information */

#include <gtk/gtk.h>
#include "weechat-client.h"

int main(int argc, char* argv[])
{
    client_t* client = client_create();
    if (client == NULL) {
        return -1;
    }

    gtk_init(&argc, &argv);

    if (client_init(client, "localhost", 1234, "1234") == FALSE) {
        g_critical("Could not initialize client.");
        return -1;
    }

    gtk_main();

    return 0;
}
