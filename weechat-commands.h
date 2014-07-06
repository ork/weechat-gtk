/* See COPYING file for license and copyright information */

#pragma once

#include "weechat-protocol.h"

/* Initialize connection with relay
 * 
 */
void weechat_cmd_init(weechat_t* weechat, const gchar* password, gboolean compression);

/* Request a hdata
 *
 * (id) hdata <path> [<keys>]
 *
 */
void weechat_cmd_hdata(weechat_t* weechat, const gchar* id, const gchar* path,
                       const gchar* keys);

/* Request an info
 * 
 */
void weechat_cmd_info(weechat_t* weechat, const gchar* id, const gchar* info);

/* Request an infolist
 *
 * (id) infolist <name> [<pointer> [<arguments>]]
 *
 */
void weechat_cmd_infolist(weechat_t* weechat, const gchar* id,
                          const gchar* name, const gchar* pointer, const gchar* arguments);

/* Request a nicklist
 * 
 */
void weechat_cmd_nicklist(weechat_t* weechat, const gchar* id, const gchar* buffer);

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
