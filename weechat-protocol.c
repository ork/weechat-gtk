/* See COPYING file for license and copyright information */

#include <glib/gprintf.h>
#include <string.h>
#include "weechat-protocol.h"

static const char* types[] = {
    "chr", "int", "lon", "str", "buf", "ptr", "tim", "htb", "hda", "inf", "inl", "arr"
};

static gchar* wtype_to_gvtype(const gchar* wtype)
{
    if (g_strcmp0(wtype, "int") == 0) {
        return "i";
    } else if (g_strcmp0(wtype, "str") == 0) {
        return "s";
    } else if (g_strcmp0(wtype, "chr") == 0) {
        return "y";
    } else if (g_strcmp0(wtype, "ptr") == 0) {
        return "s";
    }
}

weechat_t* weechat_create()
{
    weechat_t* weechat = g_try_malloc0(sizeof(weechat_t));

    if (weechat == NULL) {
        return NULL;
    }

    weechat->socket.client = g_socket_client_new();

    return weechat;
}

gboolean weechat_init(weechat_t* weechat, const gchar* host_and_port,
                      guint16 default_port)
{
    g_return_val_if_fail(weechat != NULL, FALSE);

    /* Socket */
    weechat->socket.connection = g_socket_client_connect_to_host(
        weechat->socket.client, host_and_port, default_port, NULL,
        &weechat->error);

    if (weechat->error != NULL) {
        g_critical(weechat->error->message);
        goto error_free;
    }

    /* I/O streams */
    weechat->stream.input = g_io_stream_get_input_stream(
        G_IO_STREAM(weechat->socket.connection));
    weechat->stream.output = g_io_stream_get_output_stream(
        G_IO_STREAM(weechat->socket.connection));

    /* Data stream */
    weechat->incoming = g_data_input_stream_new(weechat->stream.input);
    g_data_input_stream_set_byte_order(weechat->incoming,
                                       G_DATA_STREAM_BYTE_ORDER_BIG_ENDIAN);

    return TRUE;

error_free:
    // TODO: Do weechat_free() here
    return FALSE;
}

gboolean weechat_send(weechat_t* weechat, const gchar* msg)
{
    gchar* str_on_wire = g_strdup_printf("%s\n", msg);
    gboolean ret = TRUE;

    g_output_stream_write(weechat->stream.output, str_on_wire,
                          strlen(str_on_wire), NULL, &weechat->error);
    if (weechat->error != NULL) {
        g_warning(weechat->error->message);
        ret = FALSE;
        goto error_free;
    }

    g_output_stream_flush(weechat->stream.output, NULL, &weechat->error);
    if (weechat->error != NULL) {
        g_warning(weechat->error->message);
        ret = FALSE;
        goto error_free;
    }

error_free:
    g_free(str_on_wire);
    return ret;
}

void weechat_receive(weechat_t* weechat)
{
    answer_t* answer = weechat_parse_header(weechat);
    gsize remaining = answer->length - 5;
    GDataInputStream* mem = g_data_input_stream_new(
        g_memory_input_stream_new_from_data(answer->body, remaining, NULL));

    /* Identifier */
    gchar* id = weechat_decode_str(mem, &remaining);
    g_printf("Id: %s\n", id);

    /* As long as there is still data */
    while (remaining > 0) {
        type_t type = weechat_decode_type(mem, &remaining);
        weechat_unmarshal(mem, type, &remaining);
        g_printf("Remain: %zu\n", remaining);
    }
}

answer_t* weechat_parse_header(weechat_t* weechat)
{
    answer_t* answer = g_try_malloc0(sizeof(answer_t));
    gssize body_read;

    if (answer == NULL) {
        return NULL;
    }
    g_printf("\n\n-> Incoming packet\n");

    /* -- HEADER (5B) -- */

    /* Length (4B) */
    answer->length = g_data_input_stream_read_uint32(weechat->incoming, NULL,
                                                     &weechat->error);
    g_printf("Length: %d\n", answer->length);

    /* Compression (1B) */
    answer->compression = g_data_input_stream_read_byte(weechat->incoming,
                                                        NULL, &weechat->error);
    g_printf("Compression: %s\n", (answer->compression) ? "True" : "False");

    /* -- BODY -- */

    /* Payload (Length - HEADER) */
    answer->body = g_try_malloc0(answer->length - 5);
    body_read = g_input_stream_read(weechat->stream.input, answer->body,
                                    answer->length - 5, NULL, &weechat->error);
    g_printf("Body size: %zu\n", body_read);

    if (weechat->error != NULL) {
        g_printf(weechat->error->message);
    }

    // TODO: If compression, decompress

    return answer;
}

void weechat_unmarshal(GDataInputStream* stream, type_t type, gsize* remaining)
{
    gchar w_chr;
    gint32 w_int;
    gint64 w_lon;
    gchar* w_str;
    gchar* w_buf;
    gchar* w_ptr;
    gchar* w_tim;
    GVariant* w_arr;
    GVariant* w_inf;
    GVariant* w_inl;
    GVariant* w_hda;

    switch (type) {
    case CHR:
        w_chr = weechat_decode_chr(stream, remaining);
        g_printf("chr-> %c\n", w_chr);
        break;
    case INT:
        w_int = weechat_decode_int(stream, remaining);
        g_printf("int-> %d\n", w_int);
        break;
    case LON:
        w_lon = weechat_decode_lon(stream, remaining);
        g_printf("lon-> %ld\n", w_lon);
        break;
    case STR:
        w_str = weechat_decode_str(stream, remaining);
        g_printf("str-> %s\n", w_str);
        break;
    case BUF:
        w_buf = weechat_decode_str(stream, remaining);
        g_printf("buf-> %s\n", w_buf);
        break;
    case PTR:
        w_ptr = weechat_decode_ptr(stream, remaining);
        g_printf("ptr-> %s\n", w_ptr);
        break;
    case TIM:
        w_tim = weechat_decode_tim(stream, remaining);
        g_printf("tim-> %s\n", w_tim);
        break;
    case ARR:
        w_arr = weechat_decode_arr(stream, remaining);
        g_printf("arr -> %s\n", g_variant_print(w_arr, TRUE));
        break;
    case INF:
        w_inf = weechat_decode_inf(stream, remaining);
        g_printf("inf -> %s\n", g_variant_print(w_inf, TRUE));
        break;
    case INL:
        w_inl = weechat_decode_inl(stream, remaining);
        g_printf("inl -> %s\n", g_variant_print(w_inl, TRUE));
        break;
    case HTB:
        weechat_decode_htb(stream, remaining);
        break;
    case HDA:
        w_hda = weechat_decode_hda(stream, remaining);
        g_printf("hda -> %s\n", g_variant_print(w_hda, TRUE));
        break;
    default:
        g_printf("Type [%s] Not implemented.\n", types[type]);
        break;
    }
}

gchar* weechat_decode_str(GDataInputStream* stream, gsize* remaining)
{
    gint32 str_len = g_data_input_stream_read_int32(stream, NULL, NULL);
    *remaining -= 4;

    if (str_len == -1) {
        return NULL;
    }

    if (str_len == 0) {
        return "";
    }

    GString* msg = g_string_sized_new(str_len);

    for (int i = 0; i < str_len; ++i) {
        guchar c = g_data_input_stream_read_byte(stream, NULL, NULL);
        g_string_append_c(msg, c);
    }

    *remaining -= str_len;
    return g_string_free(msg, FALSE);
}

gchar weechat_decode_chr(GDataInputStream* stream, gsize* remaining)
{
    gchar c = g_data_input_stream_read_byte(stream, NULL, NULL);
    *remaining -= 1;

    return c;
}

gint32 weechat_decode_int(GDataInputStream* stream, gsize* remaining)
{
    gint32 i = g_data_input_stream_read_int32(stream, NULL, NULL);
    *remaining -= 4;

    return i;
}

gint64 weechat_decode_lon(GDataInputStream* stream, gsize* remaining)
{
    gchar length = g_data_input_stream_read_byte(stream, NULL, NULL);
    *remaining -= 1;

    GString* msg = g_string_sized_new(length);
    for (int i = 0; i < length; ++i) {
        guchar c = g_data_input_stream_read_byte(stream, NULL, NULL);
        g_string_append_c(msg, c);
    }
    gchar* lon = g_string_free(msg, FALSE);
    *remaining -= length;

    return g_ascii_strtoll(lon, NULL, 10);
}

gchar* weechat_decode_ptr(GDataInputStream* stream, gsize* remaining)
{
    gchar length = g_data_input_stream_read_byte(stream, NULL, NULL);
    *remaining -= 1;

    GString* msg = g_string_sized_new(length);
    for (int i = 0; i < length; ++i) {
        guchar c = g_data_input_stream_read_byte(stream, NULL, NULL);
        g_string_append_c(msg, c);
    }
    *remaining -= length;
    gchar* ptr = g_string_free(msg, FALSE);

    return g_strdup_printf("0x%s", ptr);
}

gchar* weechat_decode_tim(GDataInputStream* stream, gsize* remaining)
{
    gchar length = g_data_input_stream_read_byte(stream, NULL, NULL);
    *remaining -= 1;

    GString* msg = g_string_sized_new(length);
    for (int i = 0; i < length; ++i) {
        guchar c = g_data_input_stream_read_byte(stream, NULL, NULL);
        g_string_append_c(msg, c);
    }
    *remaining -= length;

    return g_string_free(msg, FALSE);
}

GVariant* weechat_decode_arr(GDataInputStream* stream, gsize* remaining)
{
    type_t arr_t = weechat_decode_type(stream, remaining);
    gint32 arr_l = weechat_decode_int(stream, remaining);
    GVariantBuilder* builder;
    GVariant* value;
    gchar* var_at;

    if (arr_t == INT) {
        var_at = g_strdup("ai");
    } else if (arr_t == STR) {
        var_at = g_strdup("as");
    } else {
        g_critical("Type [%s] cant exist in array\n", types[arr_t]);
        return NULL;
    }
    builder = g_variant_builder_new(G_VARIANT_TYPE(var_at));

    for (int i = 0; i < arr_l; ++i) {
        if (arr_t == INT) {
            gint32 arr_i = weechat_decode_int(stream, remaining);
            g_variant_builder_add(builder, "i", arr_i);
        } else if (arr_t == STR) {
            gchar* arr_s = weechat_decode_str(stream, remaining);
            g_variant_builder_add(builder, "s", arr_s);
        }
    }
    value = g_variant_new(var_at, builder);
    g_variant_builder_unref(builder);

    return value;
}

GVariant* weechat_decode_inf(GDataInputStream* stream, gsize* remaining)
{
    GVariant* pair;

    /* Key */
    gchar* key = weechat_decode_str(stream, remaining);

    /* Value */
    gchar* val = weechat_decode_str(stream, remaining);

    /* K-V pair */
    pair = g_variant_new("{ss}", key, val);

    return pair;
}

/*
 * {
 *   "name"    => "buffer/window/bar/...",
 *   "objects" => [
 *     {
 *       "foo" => "abc",
 *       "bar" => 12345
 *     },
 *     {
 *       "baz" => '0xaabbcc'
 *     }
 *   ]
 * }
 */
GVariant* weechat_decode_inl(GDataInputStream* stream, gsize* remaining)
{
    GVariantDict* inl = g_variant_dict_new(NULL);

    gchar* name = weechat_decode_str(stream, remaining);
    gsize count = weechat_decode_int(stream, remaining);
    g_variant_dict_insert(inl, "name", "s", name);

    GVariantBuilder* builder = g_variant_builder_new(g_variant_type_new_array(G_VARIANT_TYPE_VARDICT));
    for (gsize n = 0; n < count; ++n) {

        gint32 count_n = weechat_decode_int(stream, remaining);

        GVariantDict* item = g_variant_dict_new(NULL);
        for (gint32 i = 0; i < count_n; ++i) {

            gchar* name_i = weechat_decode_str(stream, remaining);
            type_t type_i = weechat_decode_type(stream, remaining);

            // FIXME: Ugly
            if (g_strcmp0(types[type_i], "str") == 0) {
                gchar* lol = weechat_decode_str(stream, remaining);
                g_variant_dict_insert(item, name_i, "ms", lol);
            } else if (g_strcmp0(types[type_i], "int") == 0) {
                gint32 lol = weechat_decode_int(stream, remaining);
                g_variant_dict_insert(item, name_i, "i", lol);
            } else if (g_strcmp0(types[type_i], "chr") == 0) {
                gchar lol = weechat_decode_chr(stream, remaining);
                g_variant_dict_insert(item, name_i, "y", lol);
            } else if (g_strcmp0(types[type_i], "ptr") == 0) {
                gchar* lol = weechat_decode_ptr(stream, remaining);
                g_variant_dict_insert(item, name_i, "s", lol);
            } else {
                g_critical("Type [%s] is not handled in inl\n", types[type_i]);
            }
        }
        g_variant_builder_add_value(builder, g_variant_dict_end(item));
    }
    g_variant_dict_insert_value(inl, "objects", g_variant_builder_end(builder));

    return g_variant_dict_end(inl);
}

// WIP
GVariant* weechat_decode_htb(GDataInputStream* stream, gsize* remaining)
{
    type_t k, v;
    gchar* type_k, *type_v;
    gint32 count;

    k = weechat_decode_type(stream, remaining);
    v = weechat_decode_type(stream, remaining);
    count = weechat_decode_int(stream, remaining);
    type_k = g_strdup(types[k]);
    type_v = g_strdup(types[v]);

    g_printf("->dict of %d x {%s,%s}", count, type_k, type_v);
}

GVariant* weechat_decode_hda(GDataInputStream* stream, gsize* remaining)
{
    GVariantBuilder* builder;
    gchar* path, *keys;
    gint32 count;
    gsize list_path_length = 0;

    path = weechat_decode_str(stream, remaining);
    keys = weechat_decode_str(stream, remaining);
    count = weechat_decode_int(stream, remaining);

    /* Construction of the object needs to be generic enough
     *
     * [
     *   {
     *     pointers => [0xaaaaaa, 0xbbbbbb, ...],
     *     "foo"    => (int) 12345,
     *     "bar"    => (str) "abc",
     *     "baz"    => (chr) 'z',
     *     ...
     *   },
     *   {
     *     pointers => [0xaaabbb, 0xbbbccc, ...],
     *     "foo"    => (int) 67890,
     *     "bar"    => (str) "def",
     *     "bar"    => (chr) 'x',
     *     ...
     *   },
     *   ...
     * ]
     *
     * In C++ it would be std::vector<std::map<std::string, void*>> with
     * pointer always being first.
     *
     */
    builder = g_variant_builder_new(g_variant_type_new_array(G_VARIANT_TYPE_VARDICT));

    gchar** list_path = g_strsplit(path, "/", -1);

    for (int j = 0; list_path[j] != NULL; ++j) {
        ++list_path_length;
    }

    gchar** list_keys = g_strsplit(keys, ",", -1);

    /* Construct and add dicts to the array */
    for (int buffer_n = 0; buffer_n < count; ++buffer_n) {
        /* Create an empty dict */
        GVariantDict* dict = g_variant_dict_new(NULL);

        /* Create a builder for the pointer array */
        GVariantBuilder* ptr_array = g_variant_builder_new(
            g_variant_type_new_array(G_VARIANT_TYPE_STRING));

        /* Add each pointer to it */
        for (gsize ptr_n = 0; ptr_n < list_path_length; ++ptr_n) {
            gchar* pptr = weechat_decode_ptr(stream, remaining);
            g_variant_builder_add(ptr_array, "s", pptr);
        }

        /* Add the constructed pointer array to the dict */
        g_variant_dict_insert_value(dict, "__path", g_variant_builder_end(ptr_array));

        /* For each object */
        for (int object_n = 0; list_keys[object_n] != NULL; ++object_n) {
            gchar** name_and_type = g_strsplit(list_keys[object_n], ":", -1);

            // FIXME: Ugly
            if (g_strcmp0(name_and_type[1], "str") == 0) {
                gchar* lol = weechat_decode_str(stream, remaining);

                /* String can be empty so we use a maybe type */
                g_variant_dict_insert(dict, name_and_type[0], "ms", lol);
            } else if (g_strcmp0(name_and_type[1], "int") == 0) {
                gint32 lol = weechat_decode_int(stream, remaining);

                g_variant_dict_insert(dict, name_and_type[0], "i", lol);
            } else if (g_strcmp0(name_and_type[1], "chr") == 0) {
                gchar lol = weechat_decode_chr(stream, remaining);

                /* We use a guchar with the "y" format, with maybe */
                g_variant_dict_insert(dict, name_and_type[0], "my", lol);
            } else {
                g_critical("Type [%s] is not handled in hdata\n", name_and_type[1]);
            }
            g_strfreev(name_and_type);
        }

        /* Add the dict to the builder */
        g_variant_builder_add_value(builder, g_variant_dict_end(dict));
    }

    /* Finish the build and return the constructed object */
    return g_variant_builder_end(builder);
}

type_t weechat_decode_type(GDataInputStream* stream, gsize* remaining)
{
    gchar type[4];

    type[0] = g_data_input_stream_read_byte(stream, NULL, NULL);
    type[1] = g_data_input_stream_read_byte(stream, NULL, NULL);
    type[2] = g_data_input_stream_read_byte(stream, NULL, NULL);
    type[3] = '\0';

    *remaining -= 3;

    for (int t = CHR; t <= ARR; ++t) {
        if (g_strcmp0(type, types[t]) == 0) {
            return t;
        }
    }

    g_error("Unknown type : [%s]\n", type);
}
