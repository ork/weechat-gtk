/* See COPYING file for license and copyright information */

#pragma once

#include "weechat-protocol.h"

/* Initialize connection with relay
 * 
 */
void weechat_cmd_init(weechat_t* weechat, const gchar* password, gboolean compression);

/* Request a hdata
 * 
 */
//hdata

/* Request an info
 * 
 */
//info

/* Request an infolist
 * 
 */
//infolist

/* Request a nicklist
 * 
 */
//nicklist

/* Send data to a buffer (text or command)
 * 
 */
//input

/* Synchronize buffer(s) (get updates for buffer(s))
 * 
 */
//sync

/* Desynchronize buffer(s) (stop updates for buffer(s))
 * 
 */
//desync

/* Test command: WeeChat will reply with various different objects.
 *
 */
void weechat_cmd_test(weechat_t* weechat);

/* Send a ping to WeeChat which will reply with a message "_pong" and same arguments.
 *
 */
void weechat_cmd_ping(weechat_t* weechat, const gchar* s);

/* Disconnect from relay
 * 
 */
void weechat_cmd_quit(weechat_t* weechat);
