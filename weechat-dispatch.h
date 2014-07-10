/* See COPYING file for license and copyright information */

#pragma once

#include "weechat-client.h"

void client_dispatch_buffer_line_added(client_t* client, GVariant* gv);

void client_dispatch_buffer_closing(client_t* client, GVariant* gv);