#include <glib.h>
#include <gtk/gtk.h>

struct buffer_s {
    gchar** pointers;

    gchar* full_name;
    gchar* short_name;
    gchar* title;
    gint32 notify;
    gint32 number;
    GtkTextBuffer* text_buf;
};
typedef struct buffer_s buffer_t;

gboolean buffer_equal(gconstpointer a, gconstpointer b);

buffer_t* buffer_create(GVariant* buf);

void buffer_delete(buffer_t* buffer);

const gchar* buffer_get_canonical_name(buffer_t* buffer);
