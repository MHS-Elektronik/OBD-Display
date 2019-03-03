/*******************************************************************************
                         iconcache.c  -  description
                             -------------------
    begin             : 27.05.2012
    copyright         : (C) 2012 by MHS-Elektronik GmbH & Co. KG, Germany
                               http://www.mhs-elektronik.de
    autho             : Klaus Demlehner, klaus@mhs-elektronik.de
 *******************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software, you can redistribute it and/or modify  *
 *   it under the terms of the MIT License <LICENSE.TXT or                 *
 *   http://opensource.org/licenses/MIT>                                   *
 *                                                                         *
 ***************************************************************************/
#include "support.h"
#include "main.h"
#include "iconcache.h"

static GHashTable *cache = NULL;


GdkPixbuf *mhs_get_pixbuf(const gchar * file)
{
GdkPixbuf *icon;
gchar *path;
GError *error;

error = NULL;
if (!cache)
  cache = g_hash_table_new(g_str_hash, g_str_equal);
icon = g_hash_table_lookup(cache, file);
if (!icon)
  {
  path = g_build_filename(BaseDir, file, NULL);
  icon = gdk_pixbuf_new_from_file(path, &error);
  if (error)
    {
    printf(_("Could not load icon: %s"), error->message);
    g_error_free (error);
    icon = NULL;
    }
  else
    g_hash_table_insert(cache, g_strdup(file), icon);
  g_free(path);
  }
if (icon)
  g_object_ref(icon);
return(icon);
}


GtkWidget *mhs_get_image(const gchar * file)
{
GdkPixbuf *icon;

icon = mhs_get_pixbuf(file);
return(gtk_image_new_from_pixbuf(icon));
}


void mhs_set_image(GtkWidget *widget, const gchar * file)
{
GdkPixbuf *icon;

icon = mhs_get_pixbuf(file);
gtk_image_set_from_pixbuf(GTK_IMAGE(widget), icon);
}
