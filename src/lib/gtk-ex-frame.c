/*******************************************************************************
                          gtk-ex-frame.c  -  description
                             -------------------
    begin             : 28.02.2017
    copyright         : (C) 2017 by MHS-Elektronik GmbH & Co. KG, Germany
                               http://www.mhs-elektronik.de
                        (C) 2007-2010 Nick Schermer <nick@xfce.org>
    autho             : Klaus Demlehner, klaus@mhs-elektronik.de
 *******************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software, you can redistribute it and/or modify  *
 *   it under the terms of the MIT License <LICENSE.TXT or                 *
 *   http://opensource.org/licenses/MIT>                                   *
 *                                                                         *
 ***************************************************************************/
#include "gtk-ex-frame.h"
#include <math.h>

///////////////////////////////////////// frame ///////////////////////////////////////////////


GtkWidget *gtk_ex_frame_new(void)
{
GtkWidget *widget;
//GtkExFrame *self;

widget = GTK_WIDGET( g_object_new (GTK_TYPE_EX_FRAME, NULL));
//self = GTK_EX_FRAME(widget);
return(widget);
}


static gboolean gtk_ex_frame_expose (GtkWidget *widget, GdkEventExpose *event)
{
int ox, oy, sx, sy;
float rad, r, g, b, alpha;;
double pad, m;
GtkWidget *child;
GtkStateType state;
GtkStyle *style;

if (gtk_widget_is_drawable (widget))
  {
  style = gtk_widget_get_style(widget);
  state = gtk_widget_get_state(widget);
  GdkWindow *window = widget->window;
  cairo_t *c = gdk_cairo_create(GDK_DRAWABLE(window));

  ox = widget->allocation.x;
  oy = widget->allocation.y;
  sx = widget->allocation.width;
  sy = widget->allocation.height;
  r = (double)style->bg[state].red / 65535;
  g = (double)style->bg[state].green / 65535;
  b = (double)style->bg[state].blue / 65535;

  gtk_widget_style_get(widget, "border-radius", &rad, "background-alpha", &alpha, NULL);
  pad  = widget->style->xthickness;
  m    = 0.5;

  cairo_rectangle(c, ox, oy, sx, sy);
  cairo_clip(c);
        //cairo_set_line_width(c, 1.);
  cairo_set_source_rgba(c, r, g, b, alpha);
  // top left
  cairo_move_to(c, ox + m, oy + pad + rad + m);
  cairo_arc (c, ox + rad + m, oy + rad + pad + m, rad, 1 * M_PI, 1.5 * M_PI);
  // top
  cairo_move_to(c, ox + rad + m, oy + pad + m);
  cairo_line_to(c, ox + sx - rad - m, oy + pad + m);
  // top right
  cairo_arc (c, ox + sx - rad - m, oy + rad + pad + m, rad, 1.5 * M_PI, 2 * M_PI);
  // right
  cairo_line_to(c, ox + sx - m, oy + sy - rad - m);
  // bottom right
  cairo_arc (c, ox + sx - rad - m, oy + sy - rad - m, rad, 0 * M_PI, 0.5 * M_PI);
  // bottom
  cairo_line_to(c, ox + rad + m, oy + sy - m);
  // bottom left
  cairo_arc (c, ox + rad + m, oy + sy - rad - m, rad, 0.5 * M_PI, 1 * M_PI);
  // left
  cairo_line_to(c, ox + m, oy + rad + pad + m);
  cairo_fill(c);
  cairo_stroke(c);

  cairo_destroy(c);
  }
if ((child = gtk_bin_get_child(GTK_BIN(widget))))
  gtk_container_propagate_expose(GTK_CONTAINER(widget), child, event);

return(FALSE);
}


static void gtk_ex_frame_class_init(GtkExFrameClass *klass)
{
GtkWidgetClass *widget_class;

widget_class = GTK_WIDGET_CLASS(klass);
widget_class->expose_event = gtk_ex_frame_expose;
gtk_widget_class_install_style_property(
        widget_class, g_param_spec_float("border-radius", "Border Radius", "Generate round edges", 0, 24, 15, G_PARAM_READWRITE));
gtk_widget_class_install_style_property(
        widget_class, g_param_spec_float("background-alpha", "Alpha Background", "Alpha of background", 0.0, 1.0, 0.6, G_PARAM_READWRITE));
}


static void gtk_ex_frame_init (GtkExFrame *self)
{
GtkWidget *widget;

widget = GTK_WIDGET(self);
widget->requisition.width = 40;
widget->requisition.height = 40;
}


GType gtk_ex_frame_get_type (void)
{
    static GType type = 0;
    if (!type) {
        static const GTypeInfo type_info = {
            sizeof(GtkExFrameClass),
            NULL, /* base_init */
            NULL, /* base_finalize */
            (GClassInitFunc)gtk_ex_frame_class_init,
            NULL, /* class_finalize */
            NULL, /* class_data */
            sizeof(GtkExFrame),
            0,    /* n_preallocs */
            (GInstanceInitFunc)gtk_ex_frame_init
        };
            type = g_type_register_static(GTK_TYPE_FRAME,
                                          "GtkExFrame",
                                          &type_info,
                                          (GTypeFlags)0);
    }
    return type;
}
