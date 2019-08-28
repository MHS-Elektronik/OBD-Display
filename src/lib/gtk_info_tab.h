#ifndef __GTK_UTIL_H__
#define __GTK_UTIL_H__

#include <glib.h>
#include <gtk/gtk.h>

#ifdef __cplusplus
  extern "C" {
#endif

typedef gchar *(*TInfoTableGetValueFunc)(guint idx, gpointer *data);

GtkWidget *CreateInfoTable(const gchar **description_list, TInfoTableGetValueFunc get_value_func, gpointer get_value_func_data);
void InfoTableUpdate(GtkWidget *widget);

GtkWidget *create_menue_button(const gchar *stock, const gchar *text, const gchar *secondary);
void UpdateGtk(void);

#ifdef __cplusplus
  }
#endif

#endif
