/*******************************************************************************
                            etable.c  -  description
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
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <pango/pango.h>
#include "etable.h"

#define DEFAULT_FONT "Courier New Normal 12"

#define ETABLE_GET_PRIVATE(obj) ((ETablePrivate *) ((obj)->priv))


struct _ETablePrivate
  {
  GtkWidget *main_box;
  GtkWidget *base_table;
  GtkWidget *cols_header;
  GdkGC *gc;
  GtkAdjustment *v_adj;
  GtkWidget *v_scrollbar;
  GtkWidget *draw_area;

  const struct TETableDesc *tab_desc;
  TGetLineCB get_line_cb;
  gpointer user_data;
  //gboolean auto_scroll;
  // Drawing Area
  PangoFontMetrics *font_metrics;
  PangoFontDescription *font_desc;
  PangoLayout *view_layout;

  gint cell_height;
  //gint row_size;
  //gint col_size;
  gchar line_buffer[1024];
  //struct TETableCol *col;
  };

/* Properties */
enum
  {
  PROP_0,
  PROP_GET_LINE_CB,
  PROP_USER_DATA,
  PROP_TAB_DESC,
  PROP_AUTO_SCROLL,
  PROP_RAW_SIZE
  };

G_DEFINE_TYPE(ETable, etable, GTK_TYPE_VBOX)


static void	etable_constructed (GObject *object);
static void etable_setup_font(ETable *et, const gchar *font_name);
static gint etable_get_text_width(ETable *et, const gchar *text);
static void etable_resize_columns(ETable *et);
static void etable_update_col_show(ETable *et);
static gint etable_configure_cb(GtkWidget *widget, GdkEventConfigure *configure, gpointer data);
static gint etable_expose_cb(GtkWidget *widget, GdkEventExpose *expose, gpointer data);
static void etable_value_changed_cb(GtkAdjustment *adj, gpointer data);


static void etable_finalize(GObject *object)
{
ETable *et;
ETablePrivate *priv;

et = ETABLE(object);
priv = ETABLE_GET_PRIVATE(et);
if (et->col)
  g_free(et->col);
g_free(priv);
et->priv = NULL;
G_OBJECT_CLASS(etable_parent_class)->finalize(object);
}


static void etable_get_property(GObject *object, guint prop_id,
              GValue *value, GParamSpec *pspec)
{
ETable *et = ETABLE(object);

switch (prop_id)
  {
  case PROP_GET_LINE_CB:
        {
        g_value_set_pointer(value, (gpointer)et->priv->get_line_cb);
        break;
        }
  case PROP_USER_DATA:
        {
        g_value_set_pointer(value, et->priv->user_data);
        break;
        }
  case PROP_TAB_DESC:
        {
        g_value_set_pointer(value, (gpointer)et->priv->tab_desc);
        break;
        }
  case PROP_AUTO_SCROLL:
        {
        g_value_set_boolean(value, et->auto_scroll);
        break;
        }
  case PROP_RAW_SIZE:
        {
        g_value_set_uint(value, et->row_size);
        break;
        }
  default:
        {
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
        }
  }
}


static void etable_set_property (GObject *object, guint prop_id,
              const GValue *value, GParamSpec *pspec)
{
ETable *et = ETABLE(object);

switch (prop_id)
  {
  case PROP_GET_LINE_CB:
        {
        et->priv->get_line_cb = g_value_get_pointer(value);
        break;
        }
  case PROP_USER_DATA:
        {
        et->priv->user_data = g_value_get_pointer(value);
        break;
        }
  case PROP_TAB_DESC:
        {
        et->priv->tab_desc = g_value_get_pointer(value);
        break;
        }
  case PROP_AUTO_SCROLL:
        {
        et->auto_scroll = g_value_get_boolean(value);
        break;
        }
  case PROP_RAW_SIZE:
        {
        et->row_size = g_value_get_uint(value);
        etable_set_row_size(et, et->row_size);
        break;
        }
  default:
        {
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
        }
  }
}


static void etable_class_init(ETableClass *klass)
{
GObjectClass *object_class = G_OBJECT_CLASS (klass);

g_type_class_add_private(klass, sizeof(ETablePrivate));

object_class->constructed = etable_constructed;
object_class->finalize = etable_finalize;
object_class->get_property = etable_get_property;
object_class->set_property = etable_set_property;

g_object_class_install_property(object_class, PROP_GET_LINE_CB,
                              g_param_spec_pointer("get_line_cb",
                                                   "Get Line Callback",
                                                   "Get Line Callback function",
                                                   G_PARAM_READWRITE |
                                                   G_PARAM_CONSTRUCT_ONLY));

g_object_class_install_property(object_class, PROP_USER_DATA,
                              g_param_spec_pointer("user_data",
                                                   "User Data",
                                                   "User Data for Callback function",
                                                   G_PARAM_READWRITE |
                                                   G_PARAM_CONSTRUCT_ONLY));

g_object_class_install_property(object_class, PROP_TAB_DESC,
                              g_param_spec_pointer("tab_desc",
	                                                 "Table Description",
	                                                 "Table Description",
	                                                 G_PARAM_READWRITE |
	                                                 G_PARAM_CONSTRUCT_ONLY));

g_object_class_install_property(object_class, PROP_AUTO_SCROLL,
                              g_param_spec_boolean("auto_scroll",
	                                                 "Auto Scroll",
	                                                 "Auto Scroll",
                                                   TRUE,
	                                                 G_PARAM_READWRITE));

g_object_class_install_property(object_class, PROP_RAW_SIZE,
		                          g_param_spec_uint("row_size",
		                                             "Row Size",
		                                             "Row Size",
		                                             0,
		                                             G_MAXUINT,
		                                             0,
		                                             G_PARAM_READWRITE));
}


static void etable_init(ETable *et)
{
ETablePrivate *priv;

priv = g_new0(ETablePrivate, 1);
et->priv = priv;
}


static void etable_constructed (GObject *object)
{
gint cols, i;
const gchar *row_name;
struct TETableDesc *col_desc;
GtkWidget *widget, *frame, *vbox;
GtkBox *main_box;
ETable *et;

et = ETABLE(object);
main_box = GTK_BOX(object);
cols = 0;
for (col_desc = (struct TETableDesc *)et->priv->tab_desc; col_desc->name; col_desc++)
  cols++;
/*if (!cols) <*> Fehler
  return(NULL); */
et->col = (struct TETableCol *)g_malloc0(sizeof(struct TETableCol) * cols);
/*if(!et->col)  <*> Fehler
  return(NULL); */
et->col_size = cols;
i = 0;
for (col_desc = (struct TETableDesc *)et->priv->tab_desc; (row_name = col_desc->name); col_desc++)
  {
  g_strlcpy(et->col[i].text, row_name, 80);
  g_strlcpy(et->col[i].templ, col_desc->templ, 80);
  et->col[i].show = col_desc->show;
  et->col[i].color.pixel = 0;
  et->col[i].color.red = col_desc->color.red;
  et->col[i].color.green = col_desc->color.green;
  et->col[i].color.blue = col_desc->color.blue;
  i++;
  }
// View GtkDrawingArea
et->priv->draw_area = gtk_drawing_area_new();
// Create the pango layout for the widget
et->priv->view_layout = gtk_widget_create_pango_layout(et->priv->draw_area, "");
// Create graphics context for view drawing area
et->priv->gc = gdk_gc_new((GdkWindow *)GDK_ROOT_PARENT());
// ****** Variablen Initialisieren
etable_setup_font(et, DEFAULT_FONT);
// **** View GtkTable
et->priv->base_table = gtk_table_new(1, 2, 0);
gtk_table_set_row_spacing(GTK_TABLE(et->priv->base_table), 0, 2);
gtk_table_set_col_spacing(GTK_TABLE(et->priv->base_table), 0, 2);
gtk_box_pack_start(main_box, et->priv->base_table, TRUE, TRUE, 0);
// **** View GtkFrame
frame = gtk_frame_new(NULL);
gtk_table_attach(GTK_TABLE(et->priv->base_table), frame, 0, 1, 0, 1, GTK_EXPAND | GTK_FILL | GTK_SHRINK,
       GTK_EXPAND | GTK_FILL | GTK_SHRINK, 0, 0);
gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
// Columns & View GtkVBox
vbox = gtk_vbox_new(FALSE, 0);
gtk_container_add(GTK_CONTAINER(frame), vbox);
// Column headings GtkHBox
et->priv->cols_header = gtk_hbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(vbox), et->priv->cols_header, FALSE, FALSE, 0);
// **** Cols
for (i = 0; i < et->col_size; i++)
  {
  // Frame
  frame = gtk_frame_new(NULL);
  gtk_widget_set_usize(frame, -1, 22);
  gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_OUT);
  gtk_box_pack_start(GTK_BOX(et->priv->cols_header), frame, FALSE, FALSE, 0);
  et->col[i].frame = frame;
  // Label
  widget = gtk_label_new(et->col[i].text);
  gtk_container_add(GTK_CONTAINER(frame), widget);
  et->col[i].label = GTK_LABEL(widget);
  }
// **** Ende (Open-End )
// Frame
frame = gtk_frame_new(NULL);
gtk_widget_set_usize(frame, -1, 22);
gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_OUT);
gtk_box_pack_start(GTK_BOX(et->priv->cols_header), frame, TRUE, TRUE, 0);
// View GtkDrawingArea
widget = et->priv->draw_area;
gtk_signal_connect_after(GTK_OBJECT(widget), "configure_event", GTK_SIGNAL_FUNC(etable_configure_cb), et);
gtk_signal_connect_after(GTK_OBJECT(widget), "expose_event", GTK_SIGNAL_FUNC(etable_expose_cb), et);
gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 0);
//gtk_widget_realize(widget);
// Create adjustment for vertical scrolling
et->priv->v_adj = (GtkAdjustment *)gtk_adjustment_new(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
gtk_signal_connect(GTK_OBJECT(et->priv->v_adj), "value_changed", GTK_SIGNAL_FUNC(etable_value_changed_cb), et);
// Vertical GtkScrollbar
widget = gtk_vscrollbar_new(et->priv->v_adj);
gtk_table_attach(GTK_TABLE(et->priv->base_table), widget, 1, 2, 0, 1, GTK_FILL, GTK_EXPAND | GTK_SHRINK | GTK_FILL, 0, 0);
et->priv->v_scrollbar = widget;
//ETableSetViewFont(et, DEFAULT_FONT);
gtk_widget_show_all(GTK_WIDGET(main_box));
etable_update_col_show(et);
/*g_signal_connect (panel, "show", G_CALLBACK(panel_show), NULL);*/
//G_OBJECT_CLASS(etable_parent_class)->constructed(object);
}


/*
******************** etable_new ********************
*/
GtkWidget *etable_new(TGetLineCB get_line_cb, gpointer user_data, const struct TETableDesc *tab_desc)
{
return GTK_WIDGET(g_object_new(ETABLE_TYPE,
    "get_line_cb", get_line_cb, "user_data", user_data, "tab_desc", tab_desc, NULL));
}


/*
******************** etable_set_row_size ********************
*/
void etable_set_row_size(ETable *et, gint row_size)
{
if (!GTK_IS_ETABLE(et))
  return;
et->row_size = row_size;
if (!row_size)
  etable_set_scroll_to(et, 1, 0);
else
  {
  if (et->auto_scroll)
    etable_set_scroll_to(et, 2, 0);
  else
    etable_set_scroll_to(et, 0, 0);
  }
}


/*
******************** etable_set_scroll_to ********************
*/
void etable_set_scroll_to(ETable *et, int mode, unsigned int pos)
{
gint row_size;
GtkAdjustment *adj;
GtkWidget *w;
gint height;

if (!GTK_IS_ETABLE(et))
  return;
if (!(adj = et->priv->v_adj))
  return;
// Update adjustment
w = et->priv->draw_area;
row_size = et->row_size;
if ((w != NULL) && (et->priv->cell_height > 0))
  {
  height = w->allocation.height;

  adj->lower = 0.0f;
  adj->upper = (gfloat)((row_size+1) * et->priv->cell_height);
  adj->page_size = (gfloat)height;
  adj->step_increment = (gfloat)et->priv->cell_height;
  adj->page_increment = adj->page_size / 2.0f;

  switch (mode)
    {
    case 0 : {  // Scroll Position nicht �ndern
             break;
             }
    case 1 : {  // Scroll Anfang
             adj->value = 0.0f;
             break;
             }
    case 2 : {  // Scroll Ende
             adj->value = adj->upper - adj->page_size;
             break;
             }
    case 3 : {
             if (!pos)
               break;
             if (pos >= (guint)row_size)
               pos = row_size;
             adj->value = (gfloat)((pos-1) * et->priv->cell_height);
             }
    }
  if (adj->value > (adj->upper - adj->page_size))
    adj->value = adj->upper - adj->page_size;
  if (adj->value < adj->lower)
    adj->value = adj->lower;
  gtk_signal_emit_by_name(GTK_OBJECT(adj), "value_changed");
  gtk_signal_emit_by_name(GTK_OBJECT(adj), "changed");
  }
}


static void etable_setup_font(ETable *et, const gchar *font_name)
{
PangoFontDescription *font_desc;
PangoContext *context;
PangoFont *new_font;

if(!et)
  return;
if (!font_name)
  return;
// **** Pango
if (et->priv->font_desc)
  pango_font_description_free(et->priv->font_desc);
et->priv->font_desc = NULL;
if (et->priv->font_metrics)
  pango_font_metrics_unref(et->priv->font_metrics);
et->priv->font_metrics = NULL;
font_desc = pango_font_description_from_string(font_name);
if (!font_desc)
  return;
et->priv->font_desc = font_desc;
// Modify the font for the widget
gtk_widget_modify_font(et->priv->draw_area, font_desc);
context = gdk_pango_context_get();
// FIXME - Should get the locale language here
pango_context_set_language(context, gtk_get_default_language());
new_font = pango_context_load_font(context, font_desc);
if (new_font)
  {
  et->priv->font_metrics = pango_font_get_metrics (new_font, pango_context_get_language (context));
  g_object_unref(G_OBJECT(new_font));
  }
g_object_unref(G_OBJECT(context));
if ((!font_desc) || (!et->priv->font_metrics))
  return;
et->priv->cell_height = PANGO_PIXELS(pango_font_metrics_get_ascent (et->priv->font_metrics)) +
                 PANGO_PIXELS(pango_font_metrics_get_descent (et->priv->font_metrics));
}

/*
******************** etable_set_view_font ********************
*/
gboolean etable_set_view_font(ETable *et, const gchar *font_name)
{
g_return_val_if_fail(GTK_IS_ETABLE(et), FALSE);
g_return_val_if_fail (et->col != NULL, FALSE);
etable_setup_font(et, font_name);
etable_resize_columns(et);
gtk_widget_queue_draw(et->priv->draw_area);
return(TRUE);
}


/*
******************** etable_set_col_color ********************
*/
gboolean etable_set_col_color(ETable *et, gint col_idx, GdkColor *color)
{
g_return_val_if_fail(GTK_IS_ETABLE(et), FALSE);
g_return_val_if_fail (et->col != NULL, FALSE);
if (col_idx >= et->col_size)
  return(FALSE);
memcpy(&et->col[col_idx].color, color, sizeof(GdkColor));
gtk_widget_queue_draw(et->priv->draw_area);
return(TRUE);
}


/*
******************** etable_set_col_view ********************
*/
gboolean etable_set_col_show(ETable *et, gint col_idx, gboolean show)
{
g_return_val_if_fail(GTK_IS_ETABLE(et), FALSE);
g_return_val_if_fail (et->col != NULL, FALSE);
if (col_idx >= et->col_size)
  return(FALSE);
et->col[col_idx].show = show;
etable_update_col_show(et);
return(TRUE);
}


/*
******************** etable_set_col_header_text ********************
*/
gboolean etable_set_col_header_text(ETable *et, gint col_idx, gchar *text)
{
g_return_val_if_fail(GTK_IS_ETABLE(et), FALSE);
g_return_val_if_fail (et->col != NULL, FALSE);
if (col_idx > et->col_size)
  return(FALSE);
g_strlcpy(et->col[col_idx].text, text, 80);
etable_update_col_show(et);
return(TRUE);
}


/*
******************** ETableDraw ********************
*/
/*void ETableDraw(struct TETable *et)
{
etable_expose_cb(NULL, NULL, et);
} */


/*
******************** etable_get_text_width ********************
*/
static gint etable_get_text_width(ETable *et, const gchar *text)
{
PangoLayout *layout;
PangoRectangle logical_rect;

if ((!et) || (!text))
  return(0);
layout = gtk_widget_create_pango_layout(et->priv->draw_area, text);
pango_layout_set_font_description(layout, et->priv->font_desc);
//logical_rect.width = 0;
pango_layout_get_pixel_extents(layout, NULL, &logical_rect);
g_object_unref (G_OBJECT (layout));
return(logical_rect.width);
}


/*
******************** etable_resize_columns ********************
*/
static void etable_resize_columns(ETable *et)
{
GtkRequisition size;
gint x_ofs, column_width, w;
gint i;

x_ofs = 0;
for (i = 0; i < et->col_size; i++)
  {
  if (et->col[i].show)
    {
    gtk_widget_size_request(GTK_WIDGET(et->col[i].label), &size);
    column_width = size.width;
    if ((w = etable_get_text_width(et, et->col[i].templ)) > column_width)
      column_width = w;
    gtk_widget_set_usize(et->col[i].frame, column_width, size.height);
    et->col[i].x_pos = x_ofs;
    x_ofs += column_width;
    }
  }
gtk_widget_queue_resize(et->priv->cols_header);
}


/*
******************** etable_update_col_show ********************
*/
static void etable_update_col_show(ETable *et)
{
gint i;

for (i = 0; i < et->col_size; i++)
  {
  if (et->col[i].show)
    {
    gtk_label_set_text(et->col[i].label, et->col[i].text);
    gtk_widget_show(et->col[i].frame);
    g_object_set(G_OBJECT(et->col[i].frame), "no-show-all", FALSE, NULL);
    }
  else
    {
    gtk_widget_hide_all(et->col[i].frame);
    g_object_set(G_OBJECT(et->col[i].frame), "no-show-all", TRUE, NULL);
    }
  }
etable_resize_columns(et);
etable_expose_cb(NULL, NULL, et);
}


/**************************************************************/
/* Callback Funktionen                                        */
/**************************************************************/

/*
 *	"configure_event" signal callback.
 */
static gint etable_configure_cb(GtkWidget *widget, GdkEventConfigure *configure, gpointer data)
{
gint height, row_size; // width <*>
GtkAdjustment *adj;
ETable *et;

et = ETABLE(data);
if((!widget) || (!configure) || (!et))
  return(FALSE);
if (!et->priv->draw_area)
  return(FALSE);
row_size = et->row_size;
//width = configure->width; <*> ?
height = configure->height;
// Update adjustments
adj = et->priv->v_adj;
if(adj)
  {
  adj->lower = 0.0f;
  adj->upper = (gfloat)((row_size+1) * et->priv->cell_height);
  adj->page_size = (gfloat)height;
  adj->step_increment = (gfloat)et->priv->cell_height;
  adj->page_increment = adj->page_size / 2.0f;
  gtk_signal_emit_by_name(GTK_OBJECT(adj), "changed");

  if(adj->value > (adj->upper - adj->page_size))
    {
    adj->value = adj->upper - adj->page_size;
    if (adj->value < adj->lower)
      adj->value = adj->lower;
    gtk_signal_emit_by_name(GTK_OBJECT(adj), "value_changed");
    }
  }
return(TRUE);
}


/*
 *	"expose_event" callback.
 */
static gint etable_expose_cb(GtkWidget *widget, GdkEventExpose *expose, gpointer data)
{
ETable *et;
gint cell_height, width, height, row_size, i, y;
GdkDrawable *window;
GdkGC *gc;
GtkAdjustment *adj;
GtkStateType state;
GtkStyle *style;
GtkWidget *w;
guint idx, flags;
GdkColor color;
gchar *line, *item, *next_item, *s;
gchar c;
gpointer user_data;
(void)widget;
(void)expose;

et = ETABLE(data);
cell_height = et->priv->cell_height;
row_size = et->row_size;
if(cell_height <= 0)
  return(FALSE);
gc = et->priv->gc;
adj = et->priv->v_adj;
w = et->priv->draw_area;
if((!gc) || (!adj) || (!w))
  return(FALSE);
if(!GTK_WIDGET_VISIBLE(w))
  return(FALSE);
gdk_gc_set_function(gc, GDK_COPY);
gdk_gc_set_fill(gc, GDK_SOLID);

state = GTK_WIDGET_STATE(w);
style = gtk_widget_get_style(w);
window = w->window;
if((style == NULL) || (window == NULL))
  return(FALSE);

gdk_window_get_size(window, &width, &height);
if((width <= 0) || (height <= 0))
  return(FALSE);
// Draw text
if (!et->priv->font_desc)
  return(FALSE);
idx = (gint)adj->value / cell_height;
y = 0;
  // Set the starting positions
if (idx >= (guint)row_size-1)
  idx = row_size-1;
line = et->priv->line_buffer;
user_data = et->priv->user_data;
// Draw each line
while((idx < (guint)row_size) && (y < height))
  {
  flags = 0;
  if ((et->priv->get_line_cb)(idx, user_data, line, &color, &flags) < 0)
    line[0] = '\0';
  if (flags & ET_COL_SINGLE_LINE)
    {
    if (state == GTK_STATE_INSENSITIVE)
      gdk_gc_set_foreground(gc, &style->text[GTK_STATE_INSENSITIVE]);
    else
      {
      if (flags & ET_COL_SET_COLOR)
        gdk_gc_set_rgb_fg_color(gc, &color);
      else
        gdk_gc_set_rgb_fg_color(gc, &et->col[0].color);
      }
    pango_layout_set_text(et->priv->view_layout, line, -1);
    gdk_draw_layout(window, gc, et->col[0].x_pos, y, et->priv->view_layout);
    }
  else
    {
    item = line;
    next_item = line;
    for (i = 0; i < et->col_size; i++)
      {
      for (s = item; (c = *s) != '\t'; s++)
        {
        if (!c)
          {
          next_item = s;
          break;
          }
        }
      if (c == '\t')
        {
        *s++ = '\0';
        next_item = s;
        }
      if (et->col[i].show)
        {
        if (state == GTK_STATE_INSENSITIVE)
          gdk_gc_set_foreground(gc, &style->text[GTK_STATE_INSENSITIVE]);
        else
          {
          if (flags & ET_COL_SET_COLOR)
            gdk_gc_set_rgb_fg_color(gc, &color);
          else
            gdk_gc_set_rgb_fg_color(gc, &et->col[i].color);
          }
        pango_layout_set_text(et->priv->view_layout, item, -1);
        gdk_draw_layout (window, gc, et->col[i].x_pos, y, et->priv->view_layout);
        }
      item = next_item;
      }
    }
  y += cell_height;  // nächste Zeile
  idx++;
  }
return(TRUE);
}


/*
 *	GtkAdjustment "value_changed" callback.
 */
static void etable_value_changed_cb(GtkAdjustment *adj, gpointer data)
{
ETable *et;
(void)adj;

et = ETABLE(data);
gtk_widget_queue_draw(et->priv->draw_area);
}
