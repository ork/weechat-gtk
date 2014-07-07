/* See COPYING file for license and copyright information */

#pragma once

#include <gio/gio.h>

typedef enum type_e {
    CHR,
    INT,
    LON,
    STR,
    BUF,
    PTR,
    TIM,
    HTB,
    HDA,
    INF,
    INL,
    ARR
} type_t;

struct weechat_s {
    GError* error;
    struct {
        GSocketClient* client;
        GSocketConnection* connection;
    } socket;
    struct {
        GInputStream* input;
        GOutputStream* output;
    } stream;
    GDataInputStream* incoming;
};
typedef struct weechat_s weechat_t;

struct answer_s {
    gsize length;
    gboolean compression;
    gchar* body;
};
typedef struct answer_s answer_t;

weechat_t* weechat_create();

gboolean weechat_init(weechat_t* weechat, const gchar* host_and_port, guint16 default_port);

gboolean weechat_send(weechat_t* weechat, const gchar* msg);

void weechat_receive(weechat_t* weechat);

answer_t* weechat_parse_header(weechat_t* weechat);

gchar* weechat_decode_str(GDataInputStream* stream, gsize* remaining);

gchar weechat_decode_chr(GDataInputStream* stream, gsize* remaining);

gint32 weechat_decode_int(GDataInputStream* stream, gsize* remaining);

gint64 weechat_decode_lon(GDataInputStream* stream, gsize* remaining);

gchar* weechat_decode_ptr(GDataInputStream* stream, gsize* remaining);

gchar* weechat_decode_tim(GDataInputStream* stream, gsize* remaining);

GVariant* weechat_decode_arr(GDataInputStream* stream, gsize* remaining);

GVariant* weechat_decode_inf(GDataInputStream* stream, gsize* remaining);

GVariant* weechat_decode_inl(GDataInputStream* stream, gsize* remaining);

GVariant* weechat_decode_htb(GDataInputStream* stream, gsize* remaining);

GVariant* weechat_decode_hda(GDataInputStream* stream, gsize* remaining);

void weechat_unmarshal(GDataInputStream* stream, type_t type, gsize* remaining);

type_t weechat_decode_type(GDataInputStream* stream, gsize* remaining);
