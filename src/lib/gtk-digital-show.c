/*******************************************************************************
                          gtk_digital_show.c  -  description
                             -------------------
    begin             : 28.02.2017
    copyright         : (C) 2017 by MHS-Elektronik GmbH & Co. KG, Germany
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
#include <math.h>
#include <gtk/gtk.h>
#include "util.h"
#include "gtk-ex-frame.h"
#include "gtk-lcd.h"
#include "gtk-digital-show.h"


enum
  {
  PROP_0,
  PROP_MIN_LIMIT_FG_COLOR,
  PROP_MIN_LIMIT_BG_COLOR,
  PROP_MIN_WARN_FG_COLOR,
  PROP_MIN_WARN_BG_COLOR,
  PROP_FG_COLOR,
  PROP_BG_COLOR,
  PROP_MAX_WARN_FG_COLOR,
  PROP_MAX_WARN_BG_COLOR,
  PROP_MAX_LIMIT_FG_COLOR,
  PROP_MAX_LIMIT_BG_COLOR,
  PROP_SHOW_PBAR,
  PROP_DESCRIPTION,
  PROP_UNIT,
  PROP_MIN,
  PROP_MIN_LIMIT,
  PROP_MIN_WARN,
  PROP_MAX_WARN,
  PROP_MAX_LIMIT,
  PROP_MAX,
  PROP_DIGITS,
  PROP_VALUE
  };

static GdkColor DefBgColor = { 0, 0xFFFF, 0xFFFF, 0xFFFF };  // weiß
static GdkColor DefFgColor = { 0, 0,      0,      0xFFFF };  // blau


static void gtk_digital_show_init(GtkDigitalShow *digital);
static void gtk_digital_show_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void gtk_digital_show_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
static void gtk_digital_show_finalize(GObject *object);
static void gtk_digital_show_constructed(GObject *object);
static void gtk_digital_update_value(GtkDigitalShow *digital);
static void gtk_digital_size_setup(GtkDigitalShow *digital);


G_DEFINE_TYPE(GtkDigitalShow, gtk_digital_show, GTK_TYPE_EX_FRAME)


static void gtk_digital_show_class_init(GtkDigitalShowClass *klass)
{
GObjectClass *gobject_class;

gobject_class = G_OBJECT_CLASS (klass);
gobject_class->constructed = gtk_digital_show_constructed;
gobject_class->set_property = gtk_digital_show_set_property;
gobject_class->get_property = gtk_digital_show_get_property;
gobject_class->finalize = gtk_digital_show_finalize;

g_object_class_install_property(gobject_class, PROP_MIN_LIMIT_FG_COLOR,
                                g_param_spec_boxed("min_limit_fg_color", NULL, NULL,
                                GDK_TYPE_COLOR, G_PARAM_READWRITE));

g_object_class_install_property(gobject_class, PROP_MIN_LIMIT_BG_COLOR,
                                g_param_spec_boxed("min_limit_bg_color", NULL, NULL,
                                GDK_TYPE_COLOR, G_PARAM_READWRITE));

g_object_class_install_property(gobject_class, PROP_MIN_WARN_FG_COLOR,
                                g_param_spec_boxed("min_warn_fg_color", NULL, NULL,
                                GDK_TYPE_COLOR, G_PARAM_READWRITE));

g_object_class_install_property(gobject_class, PROP_MIN_WARN_BG_COLOR,
                                g_param_spec_boxed("min_warn_bg_color", NULL, NULL,
                                GDK_TYPE_COLOR, G_PARAM_READWRITE));

g_object_class_install_property(gobject_class, PROP_FG_COLOR,
                                g_param_spec_boxed("fg_color", NULL, NULL,
                                GDK_TYPE_COLOR, G_PARAM_READWRITE));

g_object_class_install_property(gobject_class, PROP_BG_COLOR,
                                g_param_spec_boxed("bg_color", NULL, NULL,
                                GDK_TYPE_COLOR, G_PARAM_READWRITE));

g_object_class_install_property(gobject_class, PROP_MAX_WARN_FG_COLOR,
                                g_param_spec_boxed("max_warn_fg_color", NULL, NULL,
                                GDK_TYPE_COLOR, G_PARAM_READWRITE));

g_object_class_install_property(gobject_class, PROP_MAX_WARN_BG_COLOR,
                                g_param_spec_boxed("max_warn_bg_color", NULL, NULL,
                                GDK_TYPE_COLOR, G_PARAM_READWRITE));

g_object_class_install_property(gobject_class, PROP_MAX_LIMIT_FG_COLOR,
                                g_param_spec_boxed("max_limit_fg_color", NULL, NULL,
                                GDK_TYPE_COLOR, G_PARAM_READWRITE));

g_object_class_install_property(gobject_class, PROP_MAX_LIMIT_BG_COLOR,
                                g_param_spec_boxed("max_limit_bg_color", NULL, NULL,
                                GDK_TYPE_COLOR, G_PARAM_READWRITE));

g_object_class_install_property(gobject_class, PROP_SHOW_PBAR,
                                g_param_spec_boolean("show_pbar", NULL, NULL,
                                FALSE, G_PARAM_READWRITE));

g_object_class_install_property(gobject_class, PROP_DESCRIPTION,
                                g_param_spec_string ("description", NULL, NULL,
                                NULL, G_PARAM_READWRITE));

g_object_class_install_property(gobject_class, PROP_UNIT,
                                g_param_spec_string ("unit", NULL, NULL,
                                NULL, G_PARAM_READWRITE));

g_object_class_install_property(gobject_class, PROP_MIN,
                                g_param_spec_double("min", NULL, NULL,
                                -G_MAXDOUBLE, G_MAXDOUBLE, -G_MAXDOUBLE,
                                G_PARAM_READWRITE)); // | G_PARAM_STATIC_STRINGS));

g_object_class_install_property(gobject_class, PROP_MIN_LIMIT,
                                g_param_spec_double("min_limit", NULL, NULL,
                                -G_MAXDOUBLE, G_MAXDOUBLE, -G_MAXDOUBLE,
                                G_PARAM_READWRITE)); // | G_PARAM_STATIC_STRINGS));

g_object_class_install_property(gobject_class, PROP_MIN_WARN,
                                g_param_spec_double("min_warn", NULL, NULL,
                                -G_MAXDOUBLE, G_MAXDOUBLE, -G_MAXDOUBLE,
                                G_PARAM_READWRITE)); // | G_PARAM_STATIC_STRINGS));

g_object_class_install_property(gobject_class, PROP_MAX_WARN,
                                g_param_spec_double("max_warn", NULL, NULL,
                                -G_MAXDOUBLE, G_MAXDOUBLE, G_MAXDOUBLE,
                                G_PARAM_READWRITE)); // | G_PARAM_STATIC_STRINGS));

g_object_class_install_property(gobject_class, PROP_MAX_LIMIT,
                                g_param_spec_double("max_limit", NULL, NULL,
                                -G_MAXDOUBLE, G_MAXDOUBLE, G_MAXDOUBLE,
                                G_PARAM_READWRITE)); // | G_PARAM_STATIC_STRINGS));

g_object_class_install_property(gobject_class, PROP_MAX,
                                g_param_spec_double("max", NULL, NULL,
                                -G_MAXDOUBLE, G_MAXDOUBLE, G_MAXDOUBLE,
                                G_PARAM_READWRITE)); // | G_PARAM_STATIC_STRINGS));

g_object_class_install_property(gobject_class, PROP_DIGITS,
                                g_param_spec_uint("digits", NULL, NULL,
                                1, 10, 1,
                                G_PARAM_READWRITE)); // | G_PARAM_STATIC_STRINGS));

g_object_class_install_property(gobject_class, PROP_VALUE,
                                g_param_spec_double("value", NULL, NULL,
                                -G_MAXDOUBLE, G_MAXDOUBLE, 0.0,
                                G_PARAM_READWRITE)); // | G_PARAM_STATIC_STRINGS));
}


static void gtk_digital_show_init(GtkDigitalShow *digital)
{
digital->fg_color.red = DefFgColor.red;
digital->fg_color.green = DefFgColor.green;
digital->fg_color.blue = DefFgColor.blue;
digital->bg_color.red = DefBgColor.red;
digital->bg_color.green = DefBgColor.green;
digital->bg_color.blue = DefBgColor.blue;
digital->min_limit_fg_color.red = DefFgColor.red;
digital->min_limit_fg_color.green = DefFgColor.green;
digital->min_limit_fg_color.blue = DefFgColor.blue;
digital->min_limit_bg_color.red = DefBgColor.red;
digital->min_limit_bg_color.green = DefBgColor.green;
digital->min_limit_bg_color.blue = DefBgColor.blue;
digital->min_warn_fg_color.red = DefFgColor.red;
digital->min_warn_fg_color.green = DefFgColor.green;
digital->min_warn_fg_color.blue = DefFgColor.blue;
digital->min_warn_bg_color.red = DefBgColor.red;
digital->min_warn_bg_color.green = DefBgColor.green;
digital->min_warn_bg_color.blue = DefBgColor.blue;
digital->max_warn_fg_color.red = DefFgColor.red;
digital->max_warn_fg_color.green = DefFgColor.green;
digital->max_warn_fg_color.blue = DefFgColor.blue;
digital->max_warn_bg_color.red = DefBgColor.red;
digital->max_warn_bg_color.green = DefBgColor.green;
digital->max_warn_bg_color.blue = DefBgColor.blue;
digital->max_limit_fg_color.red = DefFgColor.red;
digital->max_limit_fg_color.green = DefFgColor.green;
digital->max_limit_fg_color.blue = DefFgColor.blue;
digital->max_limit_bg_color.red = DefBgColor.red;
digital->max_limit_bg_color.green = DefBgColor.green;
digital->max_limit_bg_color.blue = DefBgColor.blue;

digital->show_pbar = TRUE;
digital->description = NULL;
digital->unit = NULL;
digital->digits = 0;
digital->min = -G_MAXDOUBLE;
digital->min_limit = -G_MAXDOUBLE;
digital->min_warn = -G_MAXDOUBLE;
digital->max_warn = G_MAXDOUBLE;
digital->max_limit = G_MAXDOUBLE;
digital->max = G_MAXDOUBLE;
digital->value = 0;
}


static void gtk_digital_show_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
GtkDigitalShow *digital;

digital = GTK_DIGITAL_SHOW(object);
switch (prop_id)
  {
  case PROP_MIN_LIMIT_FG_COLOR :
            gtk_digital_show_modify_fg(digital, GTK_DIGITAL_MIN_LIMIT_COLOR, g_value_get_boxed(value));
            break;
  case PROP_MIN_LIMIT_BG_COLOR :
            gtk_digital_show_modify_bg(digital, GTK_DIGITAL_MIN_LIMIT_COLOR, g_value_get_boxed(value));
            break;
  case PROP_MIN_WARN_FG_COLOR :
            gtk_digital_show_modify_fg(digital, GTK_DIGITAL_MIN_WARN_COLOR, g_value_get_boxed(value));
            break;
  case PROP_MIN_WARN_BG_COLOR :
            gtk_digital_show_modify_bg(digital, GTK_DIGITAL_MIN_WARN_COLOR, g_value_get_boxed(value));
            break;
  case PROP_FG_COLOR :
            gtk_digital_show_modify_fg(digital, 0, g_value_get_boxed(value));
            break;
  case PROP_BG_COLOR :
            gtk_digital_show_modify_bg(digital, 0, g_value_get_boxed(value));
            break;
  case PROP_MAX_WARN_FG_COLOR :
            gtk_digital_show_modify_fg(digital, GTK_DIGITAL_MAX_WARN_COLOR, g_value_get_boxed(value));
            break;
  case PROP_MAX_WARN_BG_COLOR :
            gtk_digital_show_modify_bg(digital, GTK_DIGITAL_MAX_WARN_COLOR, g_value_get_boxed(value));
            break;
  case PROP_MAX_LIMIT_FG_COLOR :
            gtk_digital_show_modify_fg(digital, GTK_DIGITAL_MAX_LIMIT_COLOR, g_value_get_boxed(value));
            break;
  case PROP_MAX_LIMIT_BG_COLOR :
            gtk_digital_show_modify_bg(digital, GTK_DIGITAL_MAX_LIMIT_COLOR, g_value_get_boxed(value));
            break;
  case PROP_SHOW_PBAR :
            gtk_digital_show_visible_pbar(digital, g_value_get_boolean(value));
            break;
  case PROP_DESCRIPTION :
            gtk_digital_show_set_description(digital, g_value_get_string(value));
            break;
  case PROP_UNIT :
            gtk_digital_show_set_unit(digital, g_value_get_string(value));
            break;
  case PROP_MIN :
            digital->min = g_value_get_double(value);
            gtk_digital_size_setup(digital);
            break;
  case PROP_MIN_LIMIT :
            digital->min_limit = g_value_get_double(value);
            break;
  case PROP_MIN_WARN :
            digital->min_warn = g_value_get_double(value);
            break;
  case PROP_MAX_WARN :
            digital->max_warn = g_value_get_double(value);
            break;
  case PROP_MAX_LIMIT :
            digital->max_limit = g_value_get_double(value);
            break;
  case PROP_MAX :
            digital->max = g_value_get_double(value);
            gtk_digital_size_setup(digital);
            break;
  case PROP_DIGITS :
            gtk_digital_show_set_digits(digital, g_value_get_int(value));
            break;
  case PROP_VALUE :
            gtk_digital_show_set_value(digital, g_value_get_double(value));
            break;
  default:  G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
  }
}


static void gtk_digital_show_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
GtkDigitalShow *digital;

digital = GTK_DIGITAL_SHOW(object);
switch (prop_id)
  {
  case PROP_MIN_LIMIT_FG_COLOR :
            g_value_set_boxed(value, &digital->min_limit_fg_color);
            break;
  case PROP_MIN_LIMIT_BG_COLOR :
            g_value_set_boxed(value, &digital->min_limit_bg_color);
            break;
  case PROP_MIN_WARN_FG_COLOR :
            g_value_set_boxed(value, &digital->min_warn_fg_color);
            break;
  case PROP_MIN_WARN_BG_COLOR :
            g_value_set_boxed(value, &digital->min_warn_bg_color);
            break;
  case PROP_FG_COLOR :
            g_value_set_boxed(value, &digital->fg_color);
            break;
  case PROP_BG_COLOR :
            g_value_set_boxed(value, &digital->bg_color);
            break;
  case PROP_MAX_WARN_FG_COLOR :
            g_value_set_boxed(value, &digital->max_warn_fg_color);
            break;
  case PROP_MAX_WARN_BG_COLOR :
            g_value_set_boxed(value, &digital->max_warn_bg_color);
            break;
  case PROP_MAX_LIMIT_FG_COLOR :
            g_value_set_boxed(value, &digital->max_limit_fg_color);
            break;
  case PROP_MAX_LIMIT_BG_COLOR :
            g_value_set_boxed(value, &digital->max_limit_bg_color);
            break;
  case PROP_SHOW_PBAR :
            g_value_set_boolean(value, digital->show_pbar);
            break;
  case PROP_DESCRIPTION :
            g_value_set_string(value, digital->description);
            break;
  case PROP_UNIT :
            g_value_set_string(value, digital->unit);
            break;
  case PROP_MIN :
            g_value_set_double(value, digital->min);
            break;
  case PROP_MIN_LIMIT :
            g_value_set_double(value, digital->min_limit);
            break;
  case PROP_MIN_WARN :
            g_value_set_double(value, digital->min_warn);
            break;
  case PROP_MAX_WARN :
            g_value_set_double(value, digital->max_warn);
            break;
  case PROP_MAX_LIMIT :
            g_value_set_double(value, digital->max_limit);
            break;
  case PROP_MAX :
            g_value_set_double(value, digital->max);
            break;
  case PROP_DIGITS :
            g_value_set_uint(value, digital->digits);
            break;
  case PROP_VALUE :
            g_value_set_double(value, digital->value);
            break;
  default:  G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
  }
}


static void gtk_digital_show_finalize(GObject *object)
{
(*G_OBJECT_CLASS(gtk_digital_show_parent_class)->finalize)(object);
}


static void gtk_digital_show_constructed(GObject *object)
{
GtkDigitalShow *digital;
GtkWidget *box, *widget;

digital = GTK_DIGITAL_SHOW(object);

widget = gtk_alignment_new (0, 0, 1, 1);
gtk_container_add(GTK_CONTAINER(digital), widget);
gtk_alignment_set_padding(GTK_ALIGNMENT(widget), 5, 5, 5, 5);
// VBox
digital->base_vbox = gtk_vbox_new(FALSE, 5);
gtk_container_add(GTK_CONTAINER(widget), digital->base_vbox);

digital->desc_label = gtk_label_new(NULL);
gtk_label_set_markup(GTK_LABEL(digital->desc_label), digital->description);
gtk_box_pack_start(GTK_BOX(digital->base_vbox), digital->desc_label, FALSE, FALSE, 0);

box = gtk_hbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(digital->base_vbox), box, TRUE, TRUE, 0);

// **** 7-Seg LCD
digital->value_lcd = gtk_lcd_new();
gtk_misc_set_alignment(GTK_MISC(digital->value_lcd), 1, 0);
g_object_set(G_OBJECT(digital->value_lcd), "fg_color", &digital->fg_color, NULL);
//gtk_widget_modify_bg(digital->value_lcd, GTK_STATE_NORMAL, &digital->bg_color);
gtk_box_pack_start(GTK_BOX(box), digital->value_lcd, TRUE, TRUE, 0);
// **** Einheit Label
digital->unit_label = gtk_label_new(digital->unit);
gtk_widget_modify_fg(digital->unit_label, GTK_STATE_NORMAL, &digital->fg_color);
gtk_misc_set_alignment(GTK_MISC(digital->unit_label), 0, 0);
gtk_box_pack_start(GTK_BOX(box), digital->unit_label, FALSE, FALSE, 0);

digital->value_pbar = gtk_progress_bar_new();
gtk_widget_modify_fg(digital->value_pbar, GTK_STATE_NORMAL, &digital->fg_color);
gtk_box_pack_start(GTK_BOX(digital->base_vbox), digital->value_pbar, FALSE, FALSE, 0);

gtk_widget_show_all(GTK_WIDGET(digital));
// gtk_digital_show_modify_fg(digital, &digital->fg_color); <*>
// gtk_digital_show_modify_bg(digital, &digital->bg_color); <*>
gtk_digital_size_setup(digital);
gtk_digital_update_value(digital);
/*if (!digital->show_pbar)
  gtk_widget_hide(digital->value_pbar);*/
}


static void gtk_digital_update_value(GtkDigitalShow *digital)
{
gdouble percent;
GdkColor *fg_color, *bg_color;

if (digital->value <= digital->min_limit)
  {
  fg_color = &digital->min_limit_fg_color;
  bg_color = &digital->min_limit_bg_color;
  }
else if (digital->value <= digital->min_warn)
  {
  fg_color = &digital->min_warn_fg_color;
  bg_color = &digital->min_warn_bg_color;
  }
else if (digital->value >= digital->max_limit)
  {
  fg_color = &digital->max_limit_fg_color;
  bg_color = &digital->max_limit_bg_color;
  }
else if (digital->value >= digital->max_warn)
  {
  fg_color = &digital->max_warn_fg_color;
  bg_color = &digital->max_warn_bg_color;
  }
else
  {
  fg_color = &digital->fg_color;
  bg_color = &digital->bg_color;
  }

if (digital->show_pbar)
  {
  if (digital->max != digital->min)
    percent = ((digital->value - digital->min) / (digital->max - digital->min));
  else
    percent = 0.0;

  if (percent < 0)
    percent = 0;
  else if (percent > 1)
    percent = 1;
  gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(digital->value_pbar), percent);
  }

g_object_set(G_OBJECT(digital->value_lcd), "fg_color", fg_color, NULL);
//gtk_widget_modify_fg(GTK_WIDGET(digital), GTK_STATE_NORMAL, fg_color);
gtk_widget_modify_bg(GTK_WIDGET(digital), GTK_STATE_NORMAL, bg_color);

gtk_lcd_set_value(GTK_LCD(digital->value_lcd), digital->value);
}


static void gtk_digital_size_setup(GtkDigitalShow *digital)
{
guint size;
gdouble max;

if (digital->min < 0)
  {
  digital->negative = TRUE;
  size = 1;
  }
else
  {
  digital->negative = FALSE;
  size = 0;
  }
for (max = digital->max; max >= 1; max /= 10)
  size++;
digital->char_size = digital->digits + size;
gtk_lcd_config(GTK_LCD(digital->value_lcd), digital->negative, digital->char_size, digital->digits);
}


void gtk_digital_show_modify_fg(GtkDigitalShow *digital, gint color_index, GdkColor *fg_color)
{
if (color_index == GTK_DIGITAL_MIN_LIMIT_COLOR)
  {
  digital->min_limit_fg_color.red = fg_color->red;
  digital->min_limit_fg_color.green = fg_color->green;
  digital->min_limit_fg_color.blue = fg_color->blue;
  }
else if (color_index == GTK_DIGITAL_MIN_WARN_COLOR)
  {
  digital->min_warn_fg_color.red = fg_color->red;
  digital->min_warn_fg_color.green = fg_color->green;
  digital->min_warn_fg_color.blue = fg_color->blue;
  }
else if (color_index == GTK_DIGITAL_MAX_LIMIT_COLOR)
  {
  digital->max_limit_fg_color.red = fg_color->red;
  digital->max_limit_fg_color.green = fg_color->green;
  digital->max_limit_fg_color.blue = fg_color->blue;
  }
else if (color_index == GTK_DIGITAL_MAX_WARN_COLOR)
  {
  digital->max_warn_fg_color.red = fg_color->red;
  digital->max_warn_fg_color.green = fg_color->green;
  digital->max_warn_fg_color.blue = fg_color->blue;
  }
else
  {
  digital->fg_color.red = fg_color->red;
  digital->fg_color.green = fg_color->green;
  digital->fg_color.blue = fg_color->blue;
  }

gtk_digital_update_value(digital);
// g_object_set(G_OBJECT(digital->value_lcd), "fg_color", &digital->fg_color, NULL); <*>
gtk_widget_modify_fg(digital->unit_label, GTK_STATE_NORMAL, &digital->fg_color);
gtk_widget_modify_fg(digital->value_pbar, GTK_STATE_NORMAL, &digital->fg_color);
}


void gtk_digital_show_modify_bg(GtkDigitalShow *digital, gint color_index, GdkColor *bg_color)
{
if (color_index == GTK_DIGITAL_MIN_LIMIT_COLOR)
  {
  digital->min_limit_bg_color.red = bg_color->red;
  digital->min_limit_bg_color.green = bg_color->green;
  digital->min_limit_bg_color.blue = bg_color->blue;
  }
else if (color_index == GTK_DIGITAL_MIN_WARN_COLOR)
  {
  digital->min_warn_bg_color.red = bg_color->red;
  digital->min_warn_bg_color.green = bg_color->green;
  digital->min_warn_bg_color.blue = bg_color->blue;
  }
else if (color_index == GTK_DIGITAL_MAX_LIMIT_COLOR)
  {
  digital->max_limit_bg_color.red = bg_color->red;
  digital->max_limit_bg_color.green = bg_color->green;
  digital->max_limit_bg_color.blue = bg_color->blue;
  }
else if (color_index == GTK_DIGITAL_MAX_WARN_COLOR)
  {
  digital->max_warn_bg_color.red = bg_color->red;
  digital->max_warn_bg_color.green = bg_color->green;
  digital->max_warn_bg_color.blue = bg_color->blue;
  }
else
  {
  digital->bg_color.red = bg_color->red;
  digital->bg_color.green = bg_color->green;
  digital->bg_color.blue = bg_color->blue;
  }
gtk_digital_update_value(digital);
}


void gtk_digital_show_visible_pbar(GtkDigitalShow *digital, gboolean show_pbar)
{
digital->show_pbar = show_pbar;
if (show_pbar)
  gtk_widget_show(digital->value_pbar);
else
  gtk_widget_hide(digital->value_pbar);
}


void gtk_digital_show_set_description(GtkDigitalShow *digital, const gchar *description)
{
safe_free(digital->description);
digital->description = g_strdup(description);
gtk_label_set_markup(GTK_LABEL(digital->desc_label), description);
}


void gtk_digital_show_set_unit(GtkDigitalShow *digital, const gchar *unit)
{
safe_free(digital->unit);
digital->unit = g_strdup(unit);
gtk_label_set_markup(GTK_LABEL(digital->unit_label), unit);
}


void gtk_digital_show_set_digits(GtkDigitalShow *digital, guint digits)
{
digital->digits = digits;
gtk_digital_size_setup(digital);
}


void gtk_digital_show_set_value_range(GtkDigitalShow *digital, gdouble min, gdouble max)
{
digital->min = min;
digital->max = max;
gtk_digital_size_setup(digital);
}


void gtk_digital_show_set_value(GtkDigitalShow *digital, gdouble value)
{
digital->value = value;
gtk_digital_update_value(digital);
}


GtkWidget *gtk_digital_show_new(void)
{
GtkDigitalShow *digital;

digital = g_object_new(GTK_DIGITAL_SHOW_TYPE, NULL);
return(GTK_WIDGET(digital));
}


GtkWidget *gtk_digital_show_new_with_config(const gchar *desc, const gchar *unit, guint digits, gboolean show_pbar,
             gdouble min, gdouble max, gdouble value, GdkColor *fg_color, GdkColor *bg_color)
{
GtkDigitalShow *digital;

digital = g_object_new(GTK_DIGITAL_SHOW_TYPE, NULL);
if (fg_color)
  gtk_digital_show_modify_fg(digital, 0, fg_color);
if (bg_color)
  gtk_digital_show_modify_bg(digital, 0, bg_color);
gtk_digital_show_visible_pbar(digital, show_pbar);
gtk_digital_show_set_description(digital, desc);
gtk_digital_show_set_unit(digital, unit);
gtk_digital_show_set_digits(digital, digits);
gtk_digital_show_set_value_range(digital, min, max);
gtk_digital_show_set_value(digital, value);
return(GTK_WIDGET(digital));
}
