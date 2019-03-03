#ifndef __ICONCACHE_H__
#define __ICONCACHE_H__

#include <gtk/gtk.h>

GdkPixbuf	*mhs_get_pixbuf(const gchar *file);
GtkWidget	*mhs_get_image(const gchar *file);
void mhs_set_image(GtkWidget *widget, const gchar * file);

#endif
