/* GtkEx DSP Library
 * A frame widget
 *
 * Copyright (C) 2008-2015 Krzysztof Foltman, Torben Hohn, Markus
 * Schmidt and others
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02111-1307, USA.
 */

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif

#ifndef __GTK_EX_CTL_FRAME__
#define __GTK_EX_CTL_FRAME__

#include <cairo/cairo.h>
#include <gtk/gtk.h>
#include <gtk/gtkframe.h>

G_BEGIN_DECLS


typedef struct _GtkExFrameClass GtkExFrameClass;
typedef struct _GtkExFrame      GtkExFrame;


#define GTK_TYPE_EX_FRAME          (gtk_ex_frame_get_type())
#define GTK_EX_FRAME(obj)          (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_EX_FRAME, GtkExFrame))
#define GTK_EX_IS_FRAME(obj)       (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_EX_FRAME))
#define GTK_EX_FRAME_CLASS(klass)  (G_TYPE_CHECK_CLASS_CAST ((klass),  GTK_TYPE_EX_FRAME, GtkExFrameClass))
#define GTK_EX_IS_FRAME_CLASS(obj) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GTK_TYPE_EX_FRAME))

struct _GtkExFrame
{
    GtkFrame parent;
};

struct _GtkExFrameClass
{
    GtkFrameClass parent_class;
};

extern GtkWidget *gtk_ex_frame_new();
extern GType gtk_ex_frame_get_type();

G_END_DECLS

#endif
