/*******************************************************************************
                          gtk-lcd.c  -  description
                             -------------------
    begin             : 22.07.2015
    copyright         : (C) 2015 by MHS-Elektronik GmbH & Co. KG, Germany
                               http://www.mhs-elektronik.de
                        (C) 2007-2010 Nick Schermer <nick@xfce.org>
    autho             : Klaus Demlehner, klaus@mhs-elektronik.de
 *******************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software, you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   version 2.1 as published by the Free Software Foundation.             *
 *                                                                         *
 ***************************************************************************/
#include <math.h>
#include <gtk/gtk.h>
//#include <cairo/cairo.h>
#include <cairo.h>
#include "gtk-lcd.h"

#define RELATIVE_SPACE (0.10)
#define RELATIVE_DIGIT (5 * RELATIVE_SPACE)
#define RELATIVE_DOTS  (3 * RELATIVE_SPACE)


enum
  {
  PROP_0,
  PROP_NEGATIV,
  PROP_SIZE,
  PROP_DIGITS,
  PROP_VALUE,
  PROP_LCD_COLOR,
  PROP_SIZE_RATIO,
  PROP_ORIENTATION
  };


struct _GtkLcdClass
  {
  GtkImageClass __parent__;
  };


struct _GtkLcd
  {
  GtkImage __parent__;

  gboolean negativ;
  guint size;
  guint digits;
  gdouble value;
  GdkColor lcd_color;
  };


typedef struct
  {
  gdouble x;
  gdouble y;
  }
LcdPoint;


/*
 * number:
 *
 * ##1##
 * 6   2
 * ##7##
 * 5   3
 * ##4##
 */

// coordicates to draw for each segment
static const LcdPoint segment_points[][6] = {
    /* 1 */ { { 0, 0 }, { 0.5, 0 }, { 0.4, 0.1 }, { 0.1, 0.1 }, { -1, }, { -1, } },
    /* 2 */ { { 0.4, 0.1 }, { 0.5, 0.0 }, { 0.5, 0.5 }, { 0.4, 0.45 }, { -1, },  { -1, } },
    /* 3 */ { { 0.4, 0.55 }, { 0.5, 0.5 }, { 0.5, 1 }, { 0.4, 0.9 }, { -1, },  { -1, } },
    /* 4 */ { { 0.1, 0.9 }, { 0.4, 0.9 }, { 0.5, 1 }, { 0.0, 1 }, { -1, },  { -1, } },
    /* 5 */ { { 0.0, 0.5 }, { 0.1, 0.55 }, { 0.1, 0.90 }, { 0.0, 1}, { -1, },  { -1, } },
    /* 6 */ { { 0.0, 0.0 }, { 0.1, 0.1 }, { 0.1, 0.45 }, { 0.0, 0.5 }, { -1, },  { -1, } },
    /* 7 */ { { 0.0, 0.5 }, { 0.1, 0.45 }, { 0.4, 0.45 }, { 0.5, 0.5 }, { 0.4, 0.55 }, { 0.1, 0.55 } },
  };

// space line, mirrored to other side
static const LcdPoint clear_points[] = {
    { 0, 0 }, { 0.25, 0.25 }, { 0.25, 0.375 }, { 0, 0.5 },
    { 0.25, 0.625 }, { 0.25, 0.75 }, { 0, 1 }
  };

// segment to draw for each number: 0, 1, ..., 9, -
static const gint numbers[][8] = {
    { 0, 1, 2, 3, 4, 5, -1 },
    { 1, 2, -1 },
    { 0, 1, 6, 4, 3, -1 },
    { 0, 1, 6, 2, 3, -1 },
    { 5, 6, 1, 2, -1 },
    { 0, 5, 6, 2, 3, -1 },
    { 0, 5, 4, 3, 2, 6, -1 },
    { 0, 1, 2, -1 },
    { 0, 1, 2, 3, 4, 5, 6, -1 },
    { 3, 2, 1, 0, 5, 6, -1 },
    { 6, -1}
  };


static void gtk_lcd_init(GtkLcd *lcd);
static void gtk_lcd_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void gtk_lcd_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
static void gtk_lcd_finalize(GObject *object);
static gboolean gtk_lcd_expose_event(GtkWidget *widget, GdkEventExpose *event);
static gdouble gtk_lcd_get_ratio(GtkLcd *lcd);
static gdouble gtk_lcd_draw_dot(cairo_t *cr, gdouble size, gdouble offset_x, gdouble offset_y);
static gdouble gtk_lcd_draw_digit(cairo_t *cr, guint number, gdouble size, gdouble offset_x, gdouble offset_y);


G_DEFINE_TYPE(GtkLcd, gtk_lcd, GTK_TYPE_IMAGE)



static void gtk_lcd_class_init(GtkLcdClass *klass)
{
GObjectClass *gobject_class;
GtkWidgetClass *gtkwidget_class;

gobject_class = G_OBJECT_CLASS (klass);
gobject_class->set_property = gtk_lcd_set_property;
gobject_class->get_property = gtk_lcd_get_property;
gobject_class->finalize = gtk_lcd_finalize;

gtkwidget_class = GTK_WIDGET_CLASS (klass);
gtkwidget_class->expose_event = gtk_lcd_expose_event;

g_object_class_install_property(gobject_class, PROP_SIZE_RATIO,
                                g_param_spec_double("size-ratio", NULL, NULL,
                                                     -1, G_MAXDOUBLE, -1.0,
                                                     G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

g_object_class_install_property(gobject_class, PROP_ORIENTATION,
                                g_param_spec_enum("orientation", NULL, NULL,
                                                  GTK_TYPE_ORIENTATION, GTK_ORIENTATION_HORIZONTAL,
                                                  G_PARAM_WRITABLE | G_PARAM_STATIC_STRINGS));
g_object_class_install_property(gobject_class, PROP_NEGATIV,
                                g_param_spec_boolean("negativ", NULL, NULL, FALSE,
                                                     G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
g_object_class_install_property(gobject_class, PROP_VALUE,
                                g_param_spec_double("value", NULL, NULL,
                                                    -1000000, G_MAXDOUBLE, 0.0,
                                                    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

g_object_class_install_property(gobject_class, PROP_SIZE,
					            g_param_spec_uint ("size", NULL, NULL,
							                       1, 10, 1,
							                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

g_object_class_install_property(gobject_class, PROP_DIGITS,
					            g_param_spec_uint("digits", NULL, NULL,
							                       1, 10, 1,
							                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
g_object_class_install_property(gobject_class, PROP_LCD_COLOR,
					            g_param_spec_boxed("fg_color", NULL, NULL,
							       GDK_TYPE_COLOR, G_PARAM_READWRITE));
}


static void gtk_lcd_init(GtkLcd *lcd)
{
lcd->negativ = FALSE;
lcd->size = 1;
lcd->digits = 1;
lcd->value = 1;
lcd->lcd_color.red = 255;
lcd->lcd_color.green = 255;
lcd->lcd_color.blue = 255;
}


static void gtk_lcd_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
GtkLcd *lcd;

lcd = GTK_LCD(object);
switch (prop_id)
  {
  case PROP_ORIENTATION :
            break;
  case PROP_NEGATIV :
            lcd->negativ = g_value_get_boolean(value);
            break;
  case PROP_SIZE :
            lcd->size = g_value_get_uint(value);
            break;
  case PROP_DIGITS :
            lcd->digits = g_value_get_uint(value);
            break;
  case PROP_VALUE :
            lcd->value = g_value_get_double(value);
            break;
  case PROP_LCD_COLOR :
            gtk_lcd_set_fg(lcd, g_value_get_boxed(value));
            break;
  default:  G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
  }
g_object_notify(object, "size-ratio");
gtk_widget_queue_resize(GTK_WIDGET(lcd));
}


static void gtk_lcd_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
GtkLcd *lcd ;
gdouble ratio;
GdkColor color;

lcd = GTK_LCD(object);
switch (prop_id)
  {
  case PROP_SIZE_RATIO :
            ratio = gtk_lcd_get_ratio(lcd);
            g_value_set_double(value, ratio);
            break;
  case PROP_NEGATIV :
            g_value_set_boolean(value, lcd->negativ);
            break;
  case PROP_SIZE :
            g_value_set_uint(value, lcd->size);
            break;
  case PROP_DIGITS :
            g_value_set_uint(value, lcd->digits);
            break;
  case PROP_VALUE :
            g_value_set_double(value, lcd->value);
            break;
  case PROP_LCD_COLOR :
            color.red = lcd->lcd_color.red;
            color.green = lcd->lcd_color.green;
            color.blue = lcd->lcd_color.blue;
            g_value_set_boxed(value, &color);
            break;
  default:  G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
  }
}


static void gtk_lcd_finalize(GObject *object)
{
(*G_OBJECT_CLASS(gtk_lcd_parent_class)->finalize)(object);
}


static gboolean gtk_lcd_expose_event(GtkWidget *widget, GdkEventExpose *event)
{
GtkLcd *lcd;
cairo_t *cr;
gdouble offset_x, offset_y, size, ratio;
gchar str[20];
gchar *s;
gchar c;

lcd = GTK_LCD(widget);
/* get the width:height ratio */
ratio = gtk_lcd_get_ratio(GTK_LCD(widget));

/* make sure we also fit on small vertical panels */
size = MIN ((gdouble) widget->allocation.width / ratio, widget->allocation.height);

// begin offsets
//offset_x = rint((widget->allocation.width - (size * ratio)) / 2.00);
offset_x = rint(widget->allocation.width - (size * ratio));
offset_y = rint((widget->allocation.height - size) / 2.00);

// only allow positive values from the base point
offset_x = widget->allocation.x + MAX (0.00, offset_x);
offset_y = widget->allocation.y + MAX (0.00, offset_y);

/* get the cairo context */
cr = gdk_cairo_create (widget->window);

if (G_LIKELY (cr != NULL))
  {
  //gdk_cairo_set_source_color(cr, &widget->style->fg[GTK_WIDGET_STATE (widget)]);
  gdk_cairo_set_source_color(cr, &lcd->lcd_color);
  gdk_cairo_rectangle(cr, &event->area);
  cairo_clip(cr);
  cairo_push_group(cr);
  /* width of the clear line */
  cairo_set_line_width(cr, MAX (size * 0.05, 1.5));

  //offset_x -= size * (RELATIVE_SPACE * 4);
  if (lcd->negativ)
    g_snprintf(str, 20, "% *.*f", lcd->size, lcd->digits, lcd->value);
  else
    g_snprintf(str, 20, "%*.*f", lcd->size, lcd->digits, lcd->value);
  for (s = str; (c = *s); s++)
    {
    if (c == ' ')
    offset_x += size * (RELATIVE_DIGIT + RELATIVE_SPACE);
    //offset_x = gtk_lcd_draw_digit(cr, 10, size, offset_x, offset_y); <*>
    else if (c == '-')
      offset_x = gtk_lcd_draw_digit(cr, 10, size, offset_x, offset_y);
    else if ((c == '.') || (c == ','))
      offset_x = gtk_lcd_draw_dot(cr, size, offset_x, offset_y);
    else if ((c >= '0') && (c <= '9'))
      offset_x = gtk_lcd_draw_digit(cr, (guint)(c - '0'), size, offset_x, offset_y);
    }
  cairo_pop_group_to_source (cr);
  cairo_paint (cr);
  cairo_destroy (cr);
  }

return(FALSE);
}


static gdouble gtk_lcd_get_ratio(GtkLcd *lcd)
{
guint size;
gdouble ratio;

size = lcd->size + 1;
if (lcd->digits)
  size += lcd->digits;
if (lcd->negativ)
  size++;
if (lcd->digits)
  ratio = (size * RELATIVE_DIGIT) + RELATIVE_DOTS + RELATIVE_SPACE;
else
  ratio = (size * RELATIVE_DIGIT) + RELATIVE_SPACE;
return(ratio);
}


/*static gdouble
gtk_lcd_draw_dots (cairo_t *cr,
                          gdouble  size,
                          gdouble  offset_x,
                          gdouble  offset_y)
{
  gint i;

  if (size >= 10)
    {
      // draw the dots (with rounding)
      for (i = 1; i < 3; i++)
        cairo_rectangle (cr, rint (offset_x), rint (offset_y + size * RELATIVE_DOTS * i),
                         rint (size * RELATIVE_SPACE), rint (size * RELATIVE_SPACE));
    }
  else
    {
      // draw the dots
      for (i = 1; i < 3; i++)
        cairo_rectangle (cr, offset_x, offset_y + size * RELATIVE_DOTS * i,
                         size * RELATIVE_SPACE, size * RELATIVE_SPACE);
    }

  // fill the dots
  cairo_fill (cr);

  return (offset_x + size * RELATIVE_SPACE * 2);
}  */

static gdouble gtk_lcd_draw_dot(cairo_t *cr, gdouble size, gdouble offset_x, gdouble offset_y)
{
if (size >= 10)
  // draw the dots (with rounding)
  cairo_rectangle (cr, rint (offset_x), rint(offset_y + (size * (1 - RELATIVE_SPACE))),
                         rint (size * RELATIVE_SPACE), rint (size * RELATIVE_SPACE));
else
  // draw the dots
  cairo_rectangle (cr, offset_x, offset_y + (size * (1 - RELATIVE_SPACE)),
                         size * RELATIVE_SPACE, size * RELATIVE_SPACE);
// fill the dots
cairo_fill(cr);

return(offset_x + size * RELATIVE_SPACE * 2);
}


static gdouble gtk_lcd_draw_digit(cairo_t *cr, guint number, gdouble size, gdouble offset_x, gdouble offset_y)
{
guint i, j;
gint segment;
gdouble x, y, rel_x, rel_y;

for (i = 0; i < 9; i++)
  {
  // get the segment we're going to draw
  segment = numbers[number][i];
  // leave when there are no more segments left
  if (segment == -1)
    break;
  // walk through the coordinate points
  for (j = 0; j < 6; j++)
    {
    // get the relative sizes
    rel_x = segment_points[segment][j].x;
    rel_y = segment_points[segment][j].y;
    // leave when there are no valid coordinates
    if (rel_x == -1.00 || rel_y == -1.00)
      break;
    // get x and y coordinates for this point
    x = rel_x * size + offset_x;
    y = rel_y * size + offset_y;

    if (G_UNLIKELY (j == 0))
      cairo_move_to(cr, x, y);
    else
      cairo_line_to(cr, x, y);
    }
  cairo_close_path (cr);
  }
cairo_fill(cr);

/* clear the space between the segments to get the lcd look */
cairo_set_operator (cr, CAIRO_OPERATOR_CLEAR);
for (j = 0; j < 2; j++)
  {
  for (i = 0; i < G_N_ELEMENTS (clear_points); i++)
    {
    x = (j == 0 ? clear_points[i].x : 0.5 - clear_points[i].x) * size + offset_x;
    y = clear_points[i].y * size + offset_y;

    if (G_UNLIKELY (i == 0))
      cairo_move_to(cr, x, y);
    else
      cairo_line_to(cr, x, y);
    }

  cairo_stroke(cr);
  }
cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);

return (offset_x + size * (RELATIVE_DIGIT + RELATIVE_SPACE));
}


void gtk_lcd_set_fg(GtkLcd *lcd, GdkColor *color)
{
lcd->lcd_color.red = color->red;
lcd->lcd_color.green = color->green;
lcd->lcd_color.blue = color->blue;
gtk_widget_queue_draw(GTK_WIDGET(lcd));
}


void gtk_lcd_set_value(GtkLcd *lcd, gdouble value)
{
if (lcd->value != value)
  {
  lcd->value = value;
  gtk_widget_queue_draw(GTK_WIDGET(lcd));
  }
}


void gtk_lcd_config(GtkLcd *lcd, gboolean negativ, guint size, guint digits)
{
lcd->negativ = negativ;
lcd->size = size;
lcd->digits = digits;
g_object_notify(G_OBJECT(lcd), "size-ratio");
gtk_widget_queue_resize(GTK_WIDGET(lcd));
}


GtkWidget *gtk_lcd_new(void)
{
GtkLcd *lcd;

lcd = g_object_new(GTK_TYPE_LCD, NULL);
return(GTK_WIDGET(lcd));
}


GtkWidget *gtk_lcd_new_with_config(gboolean negativ, guint size, guint digits)
{
GtkLcd *lcd;

lcd = g_object_new(GTK_TYPE_LCD, NULL);
lcd->negativ = negativ;
lcd->size = size;
lcd->digits = digits;
return(GTK_WIDGET(lcd));
}
