/*
 * Copyright (C) 2006 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 * and Christopher Mire, 2006
 *
 * MegaTunix gauge widget
 *
 *
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 *
 * No warranty is made or implied. You use this program at your own risk.
 *
 *
 * -------------------------------------------------------------------------
 *  Hacked and slashed to hell by David J. Andruczyk in order to bend and
 *  tweak it to my needs for MegaTunix.  Added in rendering ability using
 *  cairo and raw GDK callls for those less fortunate (OS-X)
 *  Added a HUGE number of functions to get/set every gauge attribute
 *
 *
 *  Was offered a fine contribution by Ari Karhu
 *  "ari <at> ultimatevw <dot> com" from the msefi.com forums.
 *  His contribution made the gauges look, ohh so much nicer than I could
 *  have come up with!
 */

/*!
  \file widgets/gauge-private.c
  \brief MtxGaugeFace private functions
  \author David Andruczyk
 */

//#include <defines.h>
#include "gauge-private.h"
#include "gauge-xml.h"
#include <gdk/gdkkeysyms.h>
#include <math.h>
#include <stdio.h>

/*!
  \brief if a MtxGaugeFace isn't registered with GTK+ register the Type and
  return a Gtype object for it
  */
GType mtx_gauge_face_get_type(void)
{
	static GType mtx_gauge_face_type = 0;

	if (!mtx_gauge_face_type)
	{
		static const GTypeInfo mtx_gauge_face_info =
		{
			sizeof(MtxGaugeFaceClass),
			NULL,
			NULL,
			(GClassInitFunc) mtx_gauge_face_class_init,
			NULL,
			NULL,
			sizeof(MtxGaugeFace),
			0,
			(GInstanceInitFunc) mtx_gauge_face_init,
		};
		mtx_gauge_face_type = g_type_register_static(GTK_TYPE_DRAWING_AREA, "MtxGaugeFace", &mtx_gauge_face_info, (GTypeFlags)0);
	}
	return mtx_gauge_face_type;
}


/*!
 \brief Initializes the mtx gauge face class and links in the primary
 signal handlers for config event, expose event, and button press/release
 \param class_name is the pointer to the class
 */
void mtx_gauge_face_class_init (MtxGaugeFaceClass *class_name)
{
	GObjectClass *obj_class;
	GtkWidgetClass *widget_class;

	obj_class = G_OBJECT_CLASS (class_name);
	widget_class = GTK_WIDGET_CLASS (class_name);

	/* GtkWidget signals */

	widget_class->configure_event = mtx_gauge_face_configure;
	widget_class->expose_event = mtx_gauge_face_expose;
	widget_class->button_press_event = mtx_gauge_face_button_press;
	widget_class->button_release_event = mtx_gauge_face_button_release;
	widget_class->key_press_event = mtx_gauge_face_key_event;
	/* Motion event not needed, as unused currently */

	/*widget_class->motion_notify_event = mtx_gauge_face_motion_event;*/

	/* Realize and size_allocate are NOT NEEDED casue GtkDrawingArea
	   does it for us */

	widget_class->size_request = mtx_gauge_face_size_request;
	obj_class->finalize = mtx_gauge_face_finalize;

	g_type_class_add_private (class_name, sizeof (MtxGaugeFacePrivate));
}



/*!
 \brief Free's data
 \param gauge (MtxGaugeFace *) pointer to the gauge object
 */
void mtx_gauge_face_finalize (GObject *gauge)
{
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	if (priv->bitmap)
		g_object_unref(priv->bitmap);
	if (priv->pixmap)
		g_object_unref(priv->pixmap);
	if (priv->bg_pixmap)
		g_object_unref(priv->bg_pixmap);
	if (priv->tmp_pixmap)
		g_object_unref(priv->tmp_pixmap);
	if (priv->font_options)
		cairo_font_options_destroy(priv->font_options);
	if (priv->layout)
		g_object_unref(priv->layout);
	if (priv->font_desc)
		g_object_unref(priv->font_desc);
	if (priv->value_font)
		g_free(priv->value_font);
	if (priv->xmlfunc_array)
		g_array_free(priv->xmlfunc_array,TRUE);
	if (priv->xmlfunc_hash)
		g_hash_table_destroy(priv->xmlfunc_hash);
	if (priv->xml_filename)
		g_free(priv->xml_filename);
	if (priv->t_blocks)
		mtx_gauge_face_cleanup_text_blocks(priv->t_blocks);
	if (priv->w_ranges)
		mtx_gauge_face_cleanup_warning_ranges(priv->w_ranges);
	if (priv->a_ranges)
		mtx_gauge_face_cleanup_alert_ranges(priv->a_ranges);
	if (priv->tick_groups)
		mtx_gauge_face_cleanup_tick_groups(priv->tick_groups);
	if (priv->polygons)
		mtx_gauge_face_cleanup_polygons(priv->polygons);
	g_object_set_data(G_OBJECT(gauge),"bg_color", NULL);
	g_object_set_data(G_OBJECT(gauge),"bg_color_alt", NULL);
	g_object_set_data(G_OBJECT(gauge),"needle_color", NULL);
	g_object_set_data(G_OBJECT(gauge),"needle_color_alt", NULL);
	g_object_set_data(G_OBJECT(gauge),"value_font_color", NULL);
	g_object_set_data(G_OBJECT(gauge),"value_font_color_alt", NULL);
	g_object_set_data(G_OBJECT(gauge),"gradient_begin_color", NULL);
	g_object_set_data(G_OBJECT(gauge),"gradient_begin_color_alt", NULL);
	g_object_set_data(G_OBJECT(gauge),"gradient_end_color", NULL);
	g_object_set_data(G_OBJECT(gauge),"gradient_end_color_alt", NULL);
	g_object_set_data(G_OBJECT(gauge),"needle_length", NULL);
	g_object_set_data(G_OBJECT(gauge),"bg_color_day", NULL);
	g_object_set_data(G_OBJECT(gauge),"bg_color_nite", NULL);
	g_object_set_data(G_OBJECT(gauge),"needle_color_day", NULL);
	g_object_set_data(G_OBJECT(gauge),"needle_color_nite", NULL);
	g_object_set_data(G_OBJECT(gauge),"value_font_color_day", NULL);
	g_object_set_data(G_OBJECT(gauge),"value_font_color_nite", NULL);
	g_object_set_data(G_OBJECT(gauge),"gradient_begin_color_day", NULL);
	g_object_set_data(G_OBJECT(gauge),"gradient_begin_color_nite", NULL);
	g_object_set_data(G_OBJECT(gauge),"gradient_end_color_day", NULL);
	g_object_set_data(G_OBJECT(gauge),"gradient_end_color_nite", NULL);
	g_object_set_data(G_OBJECT(gauge),"needle_length", NULL);
	g_object_set_data(G_OBJECT(gauge),"needle_tip_width", NULL);
	g_object_set_data(G_OBJECT(gauge),"needle_tail_width", NULL);
	g_object_set_data(G_OBJECT(gauge),"needle_width", NULL);
	g_object_set_data(G_OBJECT(gauge),"needle_tail", NULL);
	g_object_set_data(G_OBJECT(gauge),"show_tattletale", NULL);
	g_object_set_data(G_OBJECT(gauge),"tattletale_alpha", NULL);
	g_object_set_data(G_OBJECT(gauge),"precision", NULL);
	g_object_set_data(G_OBJECT(gauge),"width", NULL);
	g_object_set_data(G_OBJECT(gauge),"height", NULL);
	g_object_set_data(G_OBJECT(gauge),"main_start_angle", NULL);
	g_object_set_data(G_OBJECT(gauge),"main_sweep_angle", NULL);
	g_object_set_data(G_OBJECT(gauge),"rotation", NULL);
	g_object_set_data(G_OBJECT(gauge),"lbound", NULL);
	g_object_set_data(G_OBJECT(gauge),"ubound", NULL);
	g_object_set_data(G_OBJECT(gauge),"value_font", NULL);
	g_object_set_data(G_OBJECT(gauge),"value_font_scale", NULL);
	g_object_set_data(G_OBJECT(gauge),"value_justification", NULL);
	g_object_set_data(G_OBJECT(gauge),"value_str_xpos", NULL);
	g_object_set_data(G_OBJECT(gauge),"value_str_ypos", NULL);
	g_object_set_data(G_OBJECT(gauge),"antialias", NULL);
	g_object_set_data(G_OBJECT(gauge),"show_value", NULL);
	g_object_set_data(G_OBJECT(gauge),"datasource", NULL);
	g_object_set_data(G_OBJECT(gauge),"gtk-event-mask",NULL);
}


/*!
 \brief Frees up memory for tblocks array of structures
 \param array is the pointer to the gauge text blocks array
 */
void mtx_gauge_face_cleanup_text_blocks(GArray *array)
{
	guint i = 0;
	MtxTextBlock *tblock = NULL;
	for (i=0;i<array->len;i++)
	{
		tblock = g_array_index(array,MtxTextBlock *, i);
		if (tblock->font)
			g_free(tblock->font);
		if (tblock->text)
			g_free(tblock->text);
		if (tblock)
			g_free(tblock);
	}
	g_array_free(array,TRUE);
}


/*!
 \brief Frees up memory for tgroups array of structures
 \param array is the pointer to the gauge tick groups array
 */
void mtx_gauge_face_cleanup_tick_groups(GArray *array)
{
	guint i = 0;
	MtxTickGroup *tgroup = NULL;
	for (i=0;i<array->len;i++)
	{
		tgroup = g_array_index(array,MtxTickGroup *, i);
		if (tgroup->font)
			g_free(tgroup->font);
		if (tgroup->text)
			g_free(tgroup->text);
		if (tgroup)
			g_free(tgroup);
	}
	g_array_free(array,TRUE);
}


/*!
 \brief Frees up memory for w_ranges array of structures
 \param array is the pointer to the gauge warning ranges array
 */
void mtx_gauge_face_cleanup_warning_ranges(GArray *array)
{
	guint i = 0;
	MtxWarningRange *range = NULL;
	for (i=0;i<array->len;i++)
	{
		range = g_array_index(array,MtxWarningRange *, i);
		if (range)
			g_free(range);
	}
	g_array_free(array,TRUE);
}


/*!
 \brief Frees up memory for a_ranges array of structures
 \param array is the pointer to the gauge alert ranges array
 */
void mtx_gauge_face_cleanup_alert_ranges(GArray *array)
{
	guint i = 0;
	MtxAlertRange *range = NULL;
	for (i=0;i<array->len;i++)
	{
		range = g_array_index(array,MtxAlertRange *, i);
		if (range)
			g_free(range);
	}
	g_array_free(array,TRUE);
}


/*!
 \brief Frees up memory for polygons array of structures
 \param array is the pointer to the gauge polygons array
 */
void mtx_gauge_face_cleanup_polygons(GArray *array)
{
	guint i = 0;
	MtxPolygon *poly = NULL;
	//MtxPolyType type; <*>
	MtxCircle *circle = NULL;
	MtxArc *arc = NULL;
	MtxRectangle *rect = NULL;
	MtxGenPoly *genpoly = NULL;
	for (i=0;i<array->len;i++)
	{
		poly = g_array_index(array,MtxPolygon *, i);
		if (poly)
		{
			switch (poly->type)
			{
				case MTX_CIRCLE:
					circle = (MtxCircle *)poly->data;
					if (circle)
						g_free(circle);
					break;
				case MTX_ARC:
					arc = (MtxArc *)poly->data;
					if (arc)
						g_free(arc);
					break;
				case MTX_RECTANGLE:
					rect = (MtxRectangle *)poly->data;
					if (rect)
						g_free(rect);
					break;
				case MTX_GENPOLY:
					genpoly = (MtxGenPoly *)poly->data;
					if (genpoly)
					{
						if (genpoly->points)
							g_free(genpoly->points);
						g_free(genpoly);
					}
					break;
				default:
					break;
			}

			g_free(poly);
		}
	}
	g_array_free(array,TRUE);
}


/*!
 \brief Initializes the gauge attributes to sane defaults
 \param gauge is the pointer to the gauge object
 */
void mtx_gauge_face_init (MtxGaugeFace *gauge)
{
	/* The events the gauge receives
	 * Need events for button press/release AND motion EVEN THOUGH
	 * we don't have a motion handler defined.  It's required for the
	 * dash designer to do drag and move placement
	 */

	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	gtk_widget_set_events (GTK_WIDGET (gauge),
			GDK_KEY_PRESS_MASK |
			GDK_BUTTON_PRESS_MASK |
			GDK_BUTTON_RELEASE_MASK |
			GDK_POINTER_MOTION_MASK);

	g_object_set(G_OBJECT(gauge),"can_focus",GINT_TO_POINTER(TRUE),NULL);
	gtk_widget_set_double_buffered (GTK_WIDGET(gauge), FALSE);

	priv->max_layers = 10;
	priv->w = 0;
	priv->h = 0;
	priv->xc = 0.0;
	priv->yc = 0.0;
	priv->radius = 0.0;
	priv->value = 0.0;		/* default values */

	priv->lbound = 0.0;
	priv->ubound = 100.0;
	priv->rotation = MTX_ROT_CW;
	priv->precision = 2;
	priv->clamped = CLAMP_NONE;
	priv->start_angle = 135; 	/* lower left quadrant */

	priv->sweep_angle = 270; 	/* CW sweep */

	priv->needle_width = 0.05;  	/* % of radius */

	priv->needle_tip_width = 0.0;
	priv->needle_tail_width = 0.0;
	priv->needle_tail = 0.083;  	/* % of radius */

	priv->needle_length = 0.850; 	/* % of radius */

	priv->value_font = g_strdup("Bitstream Vera Sans");
	priv->value_xpos = 0.0;
	priv->value_ypos = 0.40;
	priv->value_font_scale = 0.2;
	priv->value_justification = MTX_JUSTIFY_CENTER;
	priv->antialias = TRUE;
	priv->show_value = TRUE;
	priv->show_tattletale = FALSE;
	priv->tattletale_alpha = 0.5;
	priv->peak = priv->lbound;
	priv->a_ranges = g_array_new(FALSE,TRUE,sizeof(MtxAlertRange *));
	priv->w_ranges = g_array_new(FALSE,TRUE,sizeof(MtxWarningRange *));
	priv->t_blocks = g_array_new(FALSE,TRUE,sizeof(MtxTextBlock *));
	priv->tick_groups = g_array_new(FALSE,TRUE,sizeof(MtxTickGroup *));
	priv->polygons = g_array_new(FALSE,TRUE,sizeof(MtxPolygon *));
	priv->daytime_mode = MTX_DAY;
	mtx_gauge_face_init_default_tick_group(gauge);
	mtx_gauge_face_init_colors(gauge);
	mtx_gauge_face_init_name_bindings(gauge);
	mtx_gauge_face_init_xml_hash(gauge);
	mtx_gauge_face_redraw_canvas (gauge);
}


/*!
 \brief Initializes the gauge XML name bindings which basically sets the
 memory location of things by name which is super convenient
 \param gauge is the pointer to the gauge object
 */
void mtx_gauge_face_init_name_bindings(MtxGaugeFace *gauge)
{
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);

	/* Compat with older gauges */

	g_object_set_data(G_OBJECT(gauge),"bg_color", &priv->colors[GAUGE_COL_BG_DAY]);
	g_object_set_data(G_OBJECT(gauge),"bg_color_alt", &priv->colors[GAUGE_COL_BG_NITE]);
	g_object_set_data(G_OBJECT(gauge),"needle_color", &priv->colors[GAUGE_COL_NEEDLE_DAY]);
	g_object_set_data(G_OBJECT(gauge),"needle_color_alt", &priv->colors[GAUGE_COL_NEEDLE_NITE]);
	g_object_set_data(G_OBJECT(gauge),"value_font_color", &priv->colors[GAUGE_COL_VALUE_FONT_DAY]);
	g_object_set_data(G_OBJECT(gauge),"value_font_color_alt", &priv->colors[GAUGE_COL_VALUE_FONT_NITE]);
	g_object_set_data(G_OBJECT(gauge),"gradient_begin_color", &priv->colors[GAUGE_COL_GRADIENT_BEGIN_DAY]);
	g_object_set_data(G_OBJECT(gauge),"gradient_begin_color_alt", &priv->colors[GAUGE_COL_GRADIENT_BEGIN_NITE]);
	g_object_set_data(G_OBJECT(gauge),"gradient_end_color", &priv->colors[GAUGE_COL_GRADIENT_END_DAY]);
	g_object_set_data(G_OBJECT(gauge),"gradient_end_color_alt", &priv->colors[GAUGE_COL_GRADIENT_END_NITE]);
	g_object_set_data(G_OBJECT(gauge),"needle_length", &priv->needle_length);
	g_object_set_data(G_OBJECT(gauge),"bg_color_day", &priv->colors[GAUGE_COL_BG_DAY]);
	g_object_set_data(G_OBJECT(gauge),"bg_color_nite", &priv->colors[GAUGE_COL_BG_NITE]);
	g_object_set_data(G_OBJECT(gauge),"needle_color_day", &priv->colors[GAUGE_COL_NEEDLE_DAY]);
	g_object_set_data(G_OBJECT(gauge),"needle_color_nite", &priv->colors[GAUGE_COL_NEEDLE_NITE]);
	g_object_set_data(G_OBJECT(gauge),"value_font_color_day", &priv->colors[GAUGE_COL_VALUE_FONT_DAY]);
	g_object_set_data(G_OBJECT(gauge),"value_font_color_nite", &priv->colors[GAUGE_COL_VALUE_FONT_NITE]);
	g_object_set_data(G_OBJECT(gauge),"gradient_begin_color_day", &priv->colors[GAUGE_COL_GRADIENT_BEGIN_DAY]);
	g_object_set_data(G_OBJECT(gauge),"gradient_begin_color_nite", &priv->colors[GAUGE_COL_GRADIENT_BEGIN_NITE]);
	g_object_set_data(G_OBJECT(gauge),"gradient_end_color_day", &priv->colors[GAUGE_COL_GRADIENT_END_DAY]);
	g_object_set_data(G_OBJECT(gauge),"gradient_end_color_nite", &priv->colors[GAUGE_COL_GRADIENT_END_NITE]);
	g_object_set_data(G_OBJECT(gauge),"needle_length", &priv->needle_length);
	g_object_set_data(G_OBJECT(gauge),"needle_tip_width", &priv->needle_tip_width);
	g_object_set_data(G_OBJECT(gauge),"needle_tail_width", &priv->needle_tail_width);
	g_object_set_data(G_OBJECT(gauge),"needle_width", &priv->needle_width);
	g_object_set_data(G_OBJECT(gauge),"needle_tail", &priv->needle_tail);
	g_object_set_data(G_OBJECT(gauge),"show_tattletale", &priv->show_tattletale);
	g_object_set_data(G_OBJECT(gauge),"tattletale_alpha", &priv->tattletale_alpha);
	g_object_set_data(G_OBJECT(gauge),"precision", &priv->precision);
	g_object_set_data(G_OBJECT(gauge),"width", &priv->w);
	g_object_set_data(G_OBJECT(gauge),"height", &priv->h);
	g_object_set_data(G_OBJECT(gauge),"main_start_angle", &priv->start_angle);
	g_object_set_data(G_OBJECT(gauge),"main_sweep_angle", &priv->sweep_angle);
	g_object_set_data(G_OBJECT(gauge),"rotation", &priv->rotation);
	g_object_set_data(G_OBJECT(gauge),"lbound", &priv->lbound);
	g_object_set_data(G_OBJECT(gauge),"ubound", &priv->ubound);
	g_object_set_data(G_OBJECT(gauge),"value_font", &priv->value_font);
	g_object_set_data(G_OBJECT(gauge),"value_font_scale", &priv->value_font_scale);
	g_object_set_data(G_OBJECT(gauge),"value_justification", &priv->value_justification);
	g_object_set_data(G_OBJECT(gauge),"value_str_xpos", &priv->value_xpos);
	g_object_set_data(G_OBJECT(gauge),"value_str_ypos", &priv->value_ypos);
	g_object_set_data(G_OBJECT(gauge),"antialias", &priv->antialias);
	g_object_set_data(G_OBJECT(gauge),"show_value", &priv->show_value);
}

/*!
  \brief initializes and populates the xml_functions hashtable
  \param gauge is a pointer to the gauge object
 */
void mtx_gauge_face_init_xml_hash(MtxGaugeFace *gauge)
{
	gint i = 0;
	MtxXMLFuncs * funcs = NULL;
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	gint num_xml_funcs = sizeof(xml_functions) / sizeof(xml_functions[0]);
	priv->xmlfunc_hash = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,g_free);

	priv->xmlfunc_array = g_array_sized_new(FALSE,TRUE,sizeof (MtxXMLFuncs *),num_xml_funcs);

	for (i=0;i<num_xml_funcs;i++)
	{
		funcs = g_new0(MtxXMLFuncs, 1);
		funcs->import_func = xml_functions[i].import_func;
		funcs->export_func = xml_functions[i].export_func;
		funcs->varname = xml_functions[i].varname;
		funcs->dest_var = (gpointer)g_object_get_data(G_OBJECT(gauge),xml_functions[i].varname);
		funcs->api_compat = xml_functions[i].api_compat;
		g_hash_table_insert (priv->xmlfunc_hash,g_strdup(xml_functions[i].varname),funcs);
		g_array_append_val(priv->xmlfunc_array,funcs);
	}

}

/*!
 \brief Allocates the default colors for a gauge with no options
 \param gauge is the pointer to the gauge object
 */
void mtx_gauge_face_init_colors(MtxGaugeFace *gauge)
{
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	/* Defaults for the gauges,  user over-ridable */


	/*! Background */

	priv->colors[GAUGE_COL_BG_DAY].red=0*65535;
	priv->colors[GAUGE_COL_BG_DAY].green=0*65535;
	priv->colors[GAUGE_COL_BG_DAY].blue=0*65535;
	priv->colors[GAUGE_COL_BG_NITE].red=1*65535;
	priv->colors[GAUGE_COL_BG_NITE].green=1*65535;
	priv->colors[GAUGE_COL_BG_NITE].blue=1*65535;
	/*! Needle */

	priv->colors[GAUGE_COL_NEEDLE_DAY].red=1.0*65535;
	priv->colors[GAUGE_COL_NEEDLE_DAY].green=1.0*65535;
	priv->colors[GAUGE_COL_NEEDLE_DAY].blue=1.0*65535;
	priv->colors[GAUGE_COL_NEEDLE_NITE].red=0.0*65535;
	priv->colors[GAUGE_COL_NEEDLE_NITE].green=0.0*65535;
	priv->colors[GAUGE_COL_NEEDLE_NITE].blue=0.0*65535;
	/*! Units Font*/

	priv->colors[GAUGE_COL_VALUE_FONT_DAY].red=0.8*65535;
	priv->colors[GAUGE_COL_VALUE_FONT_DAY].green=0.8*65535;
	priv->colors[GAUGE_COL_VALUE_FONT_DAY].blue=0.8*65535;
	priv->colors[GAUGE_COL_VALUE_FONT_NITE].red=0.2*65535;
	priv->colors[GAUGE_COL_VALUE_FONT_NITE].green=0.2*65535;
	priv->colors[GAUGE_COL_VALUE_FONT_NITE].blue=0.2*65535;
	/*! Gradient Color Begin */

	priv->colors[GAUGE_COL_GRADIENT_BEGIN_DAY].red=0.85*65535;
	priv->colors[GAUGE_COL_GRADIENT_BEGIN_DAY].green=0.85*65535;
	priv->colors[GAUGE_COL_GRADIENT_BEGIN_DAY].blue=0.85*65535;
	priv->colors[GAUGE_COL_GRADIENT_BEGIN_NITE].red=0.15*65535;
	priv->colors[GAUGE_COL_GRADIENT_BEGIN_NITE].green=0.15*65535;
	priv->colors[GAUGE_COL_GRADIENT_BEGIN_NITE].blue=0.15*65535;
	/*! Gradient Color End */

	priv->colors[GAUGE_COL_GRADIENT_END_DAY].red=0.15*65535;
	priv->colors[GAUGE_COL_GRADIENT_END_DAY].green=0.15*65535;
	priv->colors[GAUGE_COL_GRADIENT_END_DAY].blue=0.15*65535;
	priv->colors[GAUGE_COL_GRADIENT_END_NITE].red=0.85*65535;
	priv->colors[GAUGE_COL_GRADIENT_END_NITE].green=0.85*65535;
	priv->colors[GAUGE_COL_GRADIENT_END_NITE].blue=0.85*65535;
}


/*!
 \brief creates a default tick group (replacing old tick system)
 \param gauge is the pointer to the gauge object
 */
void mtx_gauge_face_init_default_tick_group(MtxGaugeFace *gauge)
{
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	MtxTickGroup *tgroup = NULL;
	GdkColor white = { 0, 65535, 65535, 65535};
	GdkColor black = { 0, 0, 0, 0};

	tgroup = g_new0(MtxTickGroup, 1);
	tgroup->num_maj_ticks = 9;
	tgroup->num_min_ticks = 4;
	tgroup->start_angle = priv->start_angle;
	tgroup->sweep_angle = priv->sweep_angle;
	tgroup->maj_tick_inset = 0.15;
	tgroup->maj_tick_width = 0.175;
	tgroup->maj_tick_length = 0.110;
	tgroup->min_tick_inset = 0.175;
	tgroup->min_tick_length = 0.05;
	tgroup->min_tick_width = 0.10;
	tgroup->maj_tick_color[MTX_DAY]	= white;
	tgroup->maj_tick_color[MTX_NITE] = black;
	tgroup->min_tick_color[MTX_DAY]	= white;
	tgroup->min_tick_color[MTX_NITE] = black;
	tgroup->font = g_strdup("Arial");
	tgroup->font_scale = 0.135;
	tgroup->text_inset = 0.255;
	tgroup->text = g_strdup("");
	tgroup->text_color[MTX_DAY] = white;
	tgroup->text_color[MTX_NITE] = black;
	g_array_append_val(priv->tick_groups,tgroup);

}
/*!
 \brief updates the gauge position,  This is the CAIRO implementation that
 looks a bit nicer, though is a little bit slower
 \param gauge is the pointer to the gauge object
 */
void update_gauge_position (MtxGaugeFace *gauge)
{
	GtkWidget *widget = NULL;
	gfloat tmpf = 0.0;
	gfloat needle_pos = 0.0;
	gchar * message = NULL;
	gchar * tmpbuf = NULL;
	cairo_font_weight_t weight;
	cairo_font_slant_t slant;
	guint i = 0;
	gfloat n_width = 0.0;
	gfloat n_tail = 0.0;
	gfloat n_tip = 0.0;
	gfloat tip_width = 0.0;
	gfloat tail_width = 0.0;
	gfloat xc = 0.0;
	gfloat yc = 0.0;
	gfloat lwidth = 0.0;
	gfloat val = 0.0;
	gboolean alert = FALSE;
	MtxAlertRange *range = NULL;
	GtkAllocation allocation;
	cairo_t *cr = NULL;
	cairo_text_extents_t extents;
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);

	widget = GTK_WIDGET(gauge);
	gtk_widget_get_allocation(widget,&allocation);

	/* Check if in alert bounds and alert as necessary */

	alert = FALSE;
	for (i=0;i<priv->a_ranges->len;i++)
	{
		range = g_array_index(priv->a_ranges,MtxAlertRange *, i);
		if ((priv->value >= range->lowpoint)  &&
				(priv->value <= range->highpoint))
		{
			alert = TRUE;
			if (priv->last_alert_index == i)
				goto cairo_jump_out_of_alerts;

			/* If we alert, in order to save CPU, we copy the
			 * background pixmap to a temp pixmap and render on
			 * that and STORE the index of this alert.  Next time
			 * around we'll detect we ALREADY drew the alert and
			 * just copy the pixmap (saving all the render time)
			 * as pixmap copies are reasonably fast.
			 */

			priv->last_alert_index = i;
			cr = gdk_cairo_create(priv->tmp_pixmap);
			gdk_cairo_set_source_pixmap(cr,priv->bg_pixmap,0,0);
			cairo_rectangle(cr,0,0,allocation.width,allocation.height);
			cairo_fill(cr);
			cairo_destroy(cr);
			cr = gdk_cairo_create (priv->tmp_pixmap);

			cairo_set_source_rgb(cr,range->color[priv->daytime_mode].red/65535.0,
					range->color[priv->daytime_mode].green/65535.0,
					range->color[priv->daytime_mode].blue/65535.0);
			lwidth = priv->radius*range->lwidth < 1 ? 1: priv->radius*range->lwidth;
			cairo_set_line_width (cr, lwidth);
			cairo_arc(cr, priv->xc + (range->x_offset*priv->radius), priv->yc + (range->y_offset*priv->radius), (range->inset * priv->radius),0, 2*M_PI);
			cairo_stroke(cr);
			cairo_destroy(cr);
			break;
		}
	}
cairo_jump_out_of_alerts:
	/* Copy background pixmap to intermediary for final rendering */

	if (!alert)
	{
		/* Not in alert status,  copy from bg_pixmap to current pixmap */

		cr = gdk_cairo_create(priv->pixmap);
		gdk_cairo_set_source_pixmap(cr,priv->bg_pixmap,0,0);
		cairo_rectangle(cr,0,0,allocation.width,allocation.height);
		cairo_fill(cr);
		cairo_destroy(cr);
	}
	else
	{
		/* In ALERT status, copy from tmp_pixmap to current pixmap instead */

		cr = gdk_cairo_create(priv->pixmap);
		gdk_cairo_set_source_pixmap(cr,priv->tmp_pixmap,0,0);
		cairo_rectangle(cr,0,0,allocation.width,allocation.height);
		cairo_fill(cr);
		cairo_destroy(cr);
	}

	cr = gdk_cairo_create (priv->pixmap);
	cairo_set_font_options(cr,priv->font_options);

	if (priv->antialias)
		cairo_set_antialias(cr,CAIRO_ANTIALIAS_DEFAULT);
	else
		cairo_set_antialias(cr,CAIRO_ANTIALIAS_NONE);
	/* Update the VALUE text */

	if (priv->show_value)
	{
		if (priv->daytime_mode == MTX_DAY)
			cairo_set_source_rgb (cr, priv->colors[GAUGE_COL_VALUE_FONT_DAY].red/65535.0,
					priv->colors[GAUGE_COL_VALUE_FONT_DAY].green/65535.0,
					priv->colors[GAUGE_COL_VALUE_FONT_DAY].blue/65535.0);
		else
			cairo_set_source_rgb (cr, priv->colors[GAUGE_COL_VALUE_FONT_NITE].red/65535.0,
					priv->colors[GAUGE_COL_VALUE_FONT_NITE].green/65535.0,
					priv->colors[GAUGE_COL_VALUE_FONT_NITE].blue/65535.0);
		tmpbuf = g_utf8_strup(priv->value_font,-1);
		if (g_strrstr(tmpbuf,"BOLD"))
			weight = CAIRO_FONT_WEIGHT_BOLD;
		else
			weight = CAIRO_FONT_WEIGHT_NORMAL;
		if (g_strrstr(tmpbuf,"OBLIQUE"))
			slant = CAIRO_FONT_SLANT_OBLIQUE;
		else if (g_strrstr(tmpbuf,"ITALIC"))
			slant = CAIRO_FONT_SLANT_ITALIC;
		else
			slant = CAIRO_FONT_SLANT_NORMAL;
		g_free(tmpbuf);
		cairo_select_font_face (cr, priv->value_font,  slant, weight);

		cairo_set_font_size (cr, (priv->radius * priv->value_font_scale));

		message = g_strdup_printf("%.*f", priv->precision,priv->value);

		cairo_text_extents (cr, message, &extents);

		switch (priv->value_justification) {
			case MTX_JUSTIFY_LEFT:
				/* Left justified */
				cairo_move_to (cr,
						priv->xc+(priv->value_xpos*priv->radius),
						priv->yc+(priv->value_ypos*priv->radius));
				break;
			case MTX_JUSTIFY_RIGHT:
				cairo_move_to (cr,
						priv->xc-(extents.x_advance)+(priv->value_xpos*priv->radius),
						priv->yc-(extents.y_advance)+(priv->value_ypos*priv->radius));
				break;
			case MTX_JUSTIFY_CENTER:
			default:
				/* Centered positioning */
				cairo_move_to (cr,
						priv->xc-(extents.width/2 + extents.x_bearing)+(priv->value_xpos*priv->radius),
						priv->yc-(extents.height/2 + extents.y_bearing)+(priv->value_ypos*priv->radius));
				break;
		}
		cairo_show_text (cr, message);

		g_free(message);
		cairo_stroke (cr);
	}

	/* gauge needle */

	if (priv->clamped == CLAMP_UPPER)
		val = priv->ubound;
	else if (priv->clamped == CLAMP_LOWER)
		val = priv->lbound;
	else
		val = priv->value;
	tmpf = (val-priv->lbound)/(priv->ubound-priv->lbound);

	if (priv->rotation == MTX_ROT_CW)
		needle_pos = (priv->start_angle+(tmpf*priv->sweep_angle))*(M_PI/180);
	else
		needle_pos = ((priv->start_angle+priv->sweep_angle)-(tmpf*priv->sweep_angle))*(M_PI/180);
	if (priv->daytime_mode == MTX_DAY)
	{
		cairo_set_source_rgb (cr, priv->colors[GAUGE_COL_NEEDLE_DAY].red/65535.0,
				priv->colors[GAUGE_COL_NEEDLE_DAY].green/65535.0,
				priv->colors[GAUGE_COL_NEEDLE_DAY].blue/65535.0);
	}
	else
	{
		cairo_set_source_rgb (cr, priv->colors[GAUGE_COL_NEEDLE_NITE].red/65535.0,
				priv->colors[GAUGE_COL_NEEDLE_NITE].green/65535.0,
				priv->colors[GAUGE_COL_NEEDLE_NITE].blue/65535.0);
	}
	cairo_set_line_width (cr, 0);

	n_width = priv->needle_width * priv->radius;
	n_tail = priv->needle_tail * priv->radius;
	n_tip = priv->needle_length * priv->radius;
	tip_width = priv->needle_tip_width * priv->radius;
	tail_width = priv->needle_tail_width * priv->radius;
	xc = priv->xc;
	yc = priv->yc;

	/* Needle background,  points 0 and 1 are each "side" of the needle tip
	 * Points 2 and 5 are the points on either side of the pivot and
	 * points 3 and 4 are the points on either side of the needle's tail
	 */

	priv->needle_coords[0].x = xc + (((n_tip) * cos (needle_pos)) + ((tip_width) * -sin(needle_pos)));
	priv->needle_coords[0].y = yc + (((n_tip) * sin (needle_pos)) + ((tip_width) * cos(needle_pos)));
	priv->needle_coords[1].x = xc + (((n_tip) * cos (needle_pos)) + ((tip_width) * sin(needle_pos)));
	priv->needle_coords[1].y = yc + (((n_tip) * sin (needle_pos)) + ((tip_width) * -cos(needle_pos)));

	if (n_tail < 0)
	{
		/* "Hidden Pivot" needle, where pivot point is invisible */

		priv->needle_coords[2].x = xc + (((n_tail) * -cos (needle_pos)) + ((n_width) * sin(needle_pos)));
		priv->needle_coords[2].y = yc + (((n_tail) * -sin (needle_pos)) + ((n_width) * -cos(needle_pos)));
		priv->needle_coords[3].x = priv->needle_coords[2].x;
		priv->needle_coords[3].y = priv->needle_coords[2].y;
		priv->needle_coords[4].x = priv->needle_coords[2].x;
		priv->needle_coords[4].y = priv->needle_coords[2].y;
		priv->needle_coords[5].x = xc + (((n_tail) * -cos (needle_pos)) + ((n_width) * -sin(needle_pos)));
		priv->needle_coords[5].y = yc + (((n_tail) * -sin (needle_pos)) + ((n_width) * cos(needle_pos)));
	}
	else
	{
		/* Normal Needle */

		priv->needle_coords[2].x = xc + ((n_width) * sin(needle_pos));
		priv->needle_coords[2].y = yc + ((n_width) * -cos(needle_pos));

		priv->needle_coords[3].x = xc + (((n_tail) * -cos (needle_pos)) + ((tail_width) * sin (needle_pos)));
		priv->needle_coords[3].y = yc + (((n_tail) * -sin (needle_pos)) + ((tail_width) * -cos (needle_pos)));
		priv->needle_coords[4].x = xc + (((n_tail) * -cos (needle_pos)) + ((tail_width) * -sin (needle_pos)));
		priv->needle_coords[4].y = yc + (((n_tail) * -sin (needle_pos)) + ((tail_width) * cos (needle_pos)));
		priv->needle_coords[5].x = xc + ((n_width) * -sin (needle_pos));
		priv->needle_coords[5].y = yc + ((n_width) * cos (needle_pos));
	}
	priv->needle_polygon_points = 6;
	calc_bounding_box(priv->needle_coords,priv->needle_polygon_points, &priv->needle_bounding_box);

	cairo_move_to (cr, priv->needle_coords[0].x,priv->needle_coords[0].y);
	cairo_line_to (cr, priv->needle_coords[1].x,priv->needle_coords[1].y);
	cairo_line_to (cr, priv->needle_coords[2].x,priv->needle_coords[2].y);
	cairo_line_to (cr, priv->needle_coords[3].x,priv->needle_coords[3].y);
	cairo_line_to (cr, priv->needle_coords[4].x,priv->needle_coords[4].y);
	cairo_line_to (cr, priv->needle_coords[5].x,priv->needle_coords[5].y);
	cairo_fill_preserve (cr);
	cairo_stroke(cr);

	cairo_destroy(cr);
	return;
}


/*!
 \brief handles configure events when the gauge gets created or resized.
 Takes care of creating/destroying graphics contexts, backing pixmaps (two
 levels are used to split the rendering for speed reasons) colormaps are
 also created here as well
 \param widget is the pointer to the gauge object
 \param event is the pointer to a GdkEventConfigure structure that
 encodes important info like window dimensions and depth.
 \returns TRUE
 */
gboolean mtx_gauge_face_configure (GtkWidget *widget, GdkEventConfigure *event)
{
	cairo_t *cr = NULL;
	GtkAllocation allocation;
	GdkWindow *window = NULL;

	MtxGaugeFace * gauge = MTX_GAUGE_FACE(widget);
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(widget);
	gtk_widget_get_allocation(widget,&allocation);
	window = (GdkWindow *)gtk_widget_get_window(widget);

	if(window)
	{
		priv->w = allocation.width;
		priv->h = allocation.height;

		if (priv->layout)
			g_object_unref(priv->layout);
		/* Backing pixmap (copy of window) */

		if (priv->pixmap)
			g_object_unref(priv->pixmap);
		priv->pixmap=gdk_pixmap_new(window,
				priv->w,priv->h,
				-1);
		cr = gdk_cairo_create(priv->pixmap);
		cairo_set_operator(cr,CAIRO_OPERATOR_DEST_OUT);
		cairo_paint(cr);
		cairo_destroy(cr);
		/* Static Background pixmap */

		if (priv->bg_pixmap)
			g_object_unref(priv->bg_pixmap);
		priv->bg_pixmap=gdk_pixmap_new(window,
				priv->w,priv->h,
				-1);
		cr = gdk_cairo_create(priv->pixmap);
		cairo_set_operator(cr,CAIRO_OPERATOR_DEST_OUT);
		cairo_paint(cr);
		cairo_destroy(cr);
		/* Tmp Background pixmap */

		if (priv->tmp_pixmap)
			g_object_unref(priv->tmp_pixmap);
		priv->tmp_pixmap=gdk_pixmap_new(window,
				priv->w,priv->h,
				-1);
		cr = gdk_cairo_create(priv->pixmap);
		cairo_set_operator(cr,CAIRO_OPERATOR_DEST_OUT);
		cairo_paint(cr);
		cairo_destroy(cr);
		priv->last_alert_index = -1;

		priv->needle_bounding_box.x = 0;
		priv->needle_bounding_box.y = 0;
		priv->needle_bounding_box.width = priv->w;
		priv->needle_bounding_box.height = priv->h;
		gdk_window_set_back_pixmap(window,priv->pixmap,0);
		priv->layout = gtk_widget_create_pango_layout(GTK_WIDGET(gtk_widget_get_parent(widget)),NULL);
		priv->xc = priv->w / 2;
		priv->yc = priv->h / 2;
		priv->radius = MIN (priv->w/2, priv->h/2);

		if (priv->font_options)
			cairo_font_options_destroy(priv->font_options);
		priv->font_options = cairo_font_options_create();
		cairo_font_options_set_antialias(priv->font_options,
				CAIRO_ANTIALIAS_GRAY);

		/* Shape combine bitmap */

		if (priv->bitmap)
			g_object_unref(priv->bitmap);
		priv->bitmap = gdk_pixmap_new(NULL,priv->w,priv->h,1);
		/* Shape combine mask bitmap */

		cr = gdk_cairo_create(priv->bitmap);
		cairo_set_operator(cr,CAIRO_OPERATOR_DEST_OUT);
		cairo_paint(cr);
		cairo_set_operator(cr,CAIRO_OPERATOR_SOURCE);
		cairo_set_source_rgb (cr, 1.0,1.0,1.0);

		/* Drag border boxes... */
		if (priv->show_drag_border)
		{
			cairo_rectangle(cr,0,0,DRAG_BORDER,DRAG_BORDER);
			cairo_rectangle(cr,priv->w-DRAG_BORDER,0,DRAG_BORDER,DRAG_BORDER);
			cairo_rectangle(cr,priv->w-DRAG_BORDER,priv->h-DRAG_BORDER,DRAG_BORDER,DRAG_BORDER);
			cairo_rectangle(cr,0,priv->h-DRAG_BORDER,DRAG_BORDER,DRAG_BORDER);
			cairo_fill(cr);
		}
		/* Mask for gauge itself */
		cairo_arc(cr, priv->xc, priv->yc, priv->radius, 0, 2 * M_PI);
		cairo_fill(cr);
		cairo_stroke(cr);
		cairo_destroy(cr);
	}
	if (priv->radius > 0)
	{
		generate_gauge_background(gauge);
		update_gauge_position(gauge);
	}
	return TRUE;
}


/*!
 \brief handles exposure events when the screen is covered and then
 exposed. Works by copying from a backing pixmap to screen,
 \param widget is the pointer to the gauge object
 \param event is the pointer to a GdkEventExpose structure that
 encodes important info like window dimensions and depth.
 \returns FALSE
 */
gboolean mtx_gauge_face_expose (GtkWidget *widget, GdkEventExpose *event)
{
	MtxGaugeFacePrivate * priv = MTX_GAUGE_FACE_GET_PRIVATE(widget);
	cairo_t *cr = NULL;
	GdkWindow *window = NULL;
	GtkWidget *parent = NULL;

	g_return_val_if_fail(widget != NULL, FALSE);
	g_return_val_if_fail(MTX_IS_GAUGE_FACE(widget), FALSE);
	g_return_val_if_fail(event != NULL, FALSE);
	window = (GdkWindow *)gtk_widget_get_window(widget);
	parent = gtk_widget_get_parent(widget);

#if GTK_MINOR_VERSION >= 18
	if (gtk_widget_is_sensitive(GTK_WIDGET(widget)))
#else
	if (GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(widget)))
#endif
		{
			cr = gdk_cairo_create(window);
			gdk_cairo_set_source_pixmap(cr,priv->pixmap,0,0);
			cairo_rectangle(cr,event->area.x,event->area.y,event->area.width, event->area.height);
			cairo_fill(cr);
			cairo_destroy(cr);
		}
		else
		{
			cr = gdk_cairo_create(window);
			gdk_cairo_set_source_pixmap(cr,priv->pixmap,0,0);
			cairo_rectangle(cr,event->area.x,event->area.y,event->area.width, event->area.height);
			cairo_fill(cr);
			cairo_set_source_rgba (cr, 0.3,0.3,0.3,0.5);
			cairo_rectangle (cr,
					0,0,priv->w,priv->h);
			cairo_fill(cr);
			cairo_destroy(cr);
		}
	if (GTK_IS_WINDOW(parent))
	{
#if GTK_MINOR_VERSION >= 10
		if (gtk_minor_version >= 10)
			gtk_widget_input_shape_combine_mask(parent,priv->bitmap,0,0);
#endif
		gtk_widget_shape_combine_mask(parent,priv->bitmap,0,0);
	}
	else
	{
#if GTK_MINOR_VERSION >= 10
		if (gtk_minor_version >= 10)
			gdk_window_input_shape_combine_mask(window,priv->bitmap,0,0);
#endif
		gdk_window_shape_combine_mask(window,priv->bitmap,0,0);
	}
	return FALSE;
}


/*!
 \brief draws the static elements of the gauge (only on resize), This includes
 the border, units and name strings,  tick marks and warning regions
 This is the cairo version.
 \param gauge is the pointer to the gauge object
 */
void generate_gauge_background(MtxGaugeFace *gauge)
{
	//GtkWidget * widget = NULL; <*>
	cairo_t *cr = NULL;
	double dashes[2] = {4.0,4.0};
	gfloat deg_per_major_tick = 0.0;
	gfloat deg_per_minor_tick = 0.0;
	cairo_font_weight_t weight;
	cairo_font_slant_t slant;
	gchar * tmpbuf = NULL;
	//gint w = 0; <*>
	//gint h = 0; <*>
	guint i = 0;
	gint j = 0;
	gint k = 0;
	gint num_points = 0;
	gint count = 0;
	gfloat counter = 0;
	gfloat rad = 0.0;
	gfloat subcounter = 0;
	gchar ** vector = NULL;
	gfloat inset = 0.0;
	gfloat insetfrom = 0.0;
	gfloat mintick_inset = 0.0;
	gfloat lwidth = 0.0;
	gfloat angle1, angle2;
	gfloat val = 0.0;
	gfloat tmpf = 0.0;
	gfloat tattle_pos = 0.0;
	gfloat t_width = 0.0;
	gfloat t_tail = 0.0;
	gfloat t_tip = 0.0;
	gfloat tip_width = 0.0;
	gfloat tail_width = 0.0;
	gfloat xc = 0.0;
	gfloat yc = 0.0;
	gint layer = 0;
	cairo_pattern_t *gradient = NULL;
	cairo_text_extents_t extents;
	MtxPolygon *poly = NULL;
	MtxWarningRange *range = NULL;
	MtxTextBlock *tblock = NULL;
	MtxTickGroup *tgroup = NULL;
	GtkAllocation allocation;
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);

	gtk_widget_get_allocation(GTK_WIDGET(gauge),&allocation);

	//w = allocation.width; <*>
	//h = allocation.height; <*>

	if (!priv->bg_pixmap)
		return;
	/* get a cairo_t */

	cr = gdk_cairo_create (priv->bg_pixmap);
	cairo_set_font_options(cr,priv->font_options);
	/* Background set to black */

	cairo_set_source_rgb (cr, 0,0,0);
	/* The little corner resizer boxes... */

	if (priv->show_drag_border)
	{
		/* Fill 4 rectangles with black */

		cairo_rectangle (cr,
				0,0,
				DRAG_BORDER, DRAG_BORDER);
		cairo_rectangle (cr,
				priv->w-DRAG_BORDER,0,
				DRAG_BORDER, DRAG_BORDER);
		cairo_rectangle (cr,
				0,priv->h-DRAG_BORDER,
				DRAG_BORDER, DRAG_BORDER);
		cairo_rectangle (cr,
				priv->w-DRAG_BORDER,priv->h-DRAG_BORDER,
				DRAG_BORDER, DRAG_BORDER);
		cairo_fill(cr);
		cairo_set_source_rgb (cr, 0.9,0.9,0.9);
		/* draw light grey border around 4 rectangles */

		cairo_rectangle (cr,
				0,0,
				DRAG_BORDER, DRAG_BORDER);
		cairo_rectangle (cr,
				priv->w-DRAG_BORDER,0,
				DRAG_BORDER, DRAG_BORDER);
		cairo_rectangle (cr,
				0,priv->h-DRAG_BORDER,
				DRAG_BORDER, DRAG_BORDER);
		cairo_rectangle (cr,
				priv->w-DRAG_BORDER,priv->h-DRAG_BORDER,
				DRAG_BORDER, DRAG_BORDER);
		cairo_stroke(cr);
	}
	/* Black out the rest of the gauge */

	cairo_set_source_rgb (cr, 0,0,0);
	cairo_arc(cr, priv->xc, priv->yc, priv->radius, 0, 2 * M_PI);

	cairo_fill(cr);
	if (priv->antialias)
		cairo_set_antialias(cr,CAIRO_ANTIALIAS_DEFAULT);
	else
		cairo_set_antialias(cr,CAIRO_ANTIALIAS_NONE);


	/* Filled Arcs */

	/* Outside gradient ring */

	gradient = cairo_pattern_create_linear(priv->xc+(0.707*priv->xc),
			priv->yc-(0.707*priv->yc),
			priv->xc-(0.707*priv->xc),
			priv->yc+(0.707*priv->yc));
	if (priv->daytime_mode == MTX_DAY)
	{
		cairo_pattern_add_color_stop_rgb(gradient, 0,
				priv->colors[GAUGE_COL_GRADIENT_BEGIN_DAY].red/65535.0,
				priv->colors[GAUGE_COL_GRADIENT_BEGIN_DAY].green/65535.0,
				priv->colors[GAUGE_COL_GRADIENT_BEGIN_DAY].blue/65535.0);
		cairo_pattern_add_color_stop_rgb(gradient, 2*priv->radius,
				priv->colors[GAUGE_COL_GRADIENT_END_DAY].red/65535.0,
				priv->colors[GAUGE_COL_GRADIENT_END_DAY].green/65535.0,
				priv->colors[GAUGE_COL_GRADIENT_END_DAY].blue/65535.0);

	}
	else
	{
		cairo_pattern_add_color_stop_rgb(gradient, 0,
				priv->colors[GAUGE_COL_GRADIENT_BEGIN_NITE].red/65535.0,
				priv->colors[GAUGE_COL_GRADIENT_BEGIN_NITE].green/65535.0,
				priv->colors[GAUGE_COL_GRADIENT_BEGIN_NITE].blue/65535.0);
		cairo_pattern_add_color_stop_rgb(gradient, 2*priv->radius,
				priv->colors[GAUGE_COL_GRADIENT_END_NITE].red/65535.0,
				priv->colors[GAUGE_COL_GRADIENT_END_NITE].green/65535.0,
				priv->colors[GAUGE_COL_GRADIENT_END_NITE].blue/65535.0);

	}
	cairo_set_source(cr, gradient);
	cairo_arc(cr, priv->xc, priv->yc, priv->radius, 0, 2 * M_PI);
	cairo_fill(cr);
	cairo_pattern_destroy(gradient);

	/* Inside gradient ring */

	gradient = cairo_pattern_create_linear(priv->xc-(0.707*priv->xc),
			priv->yc+(0.707*priv->yc),
			priv->xc+(0.707*priv->xc),
			priv->yc-(0.707*priv->yc));
	if (priv->daytime_mode == MTX_DAY)
	{
		cairo_pattern_add_color_stop_rgb(gradient, 0,
				priv->colors[GAUGE_COL_GRADIENT_BEGIN_DAY].red/65535.0,
				priv->colors[GAUGE_COL_GRADIENT_BEGIN_DAY].green/65535.0,
				priv->colors[GAUGE_COL_GRADIENT_BEGIN_DAY].blue/65535.0);
		cairo_pattern_add_color_stop_rgb(gradient, 2*priv->radius,
				priv->colors[GAUGE_COL_GRADIENT_END_DAY].red/65535.0,
				priv->colors[GAUGE_COL_GRADIENT_END_DAY].green/65535.0,
				priv->colors[GAUGE_COL_GRADIENT_END_DAY].blue/65535.0);

	}
	else
	{
		cairo_pattern_add_color_stop_rgb(gradient, 0,
				priv->colors[GAUGE_COL_GRADIENT_BEGIN_NITE].red/65535.0,
				priv->colors[GAUGE_COL_GRADIENT_BEGIN_NITE].green/65535.0,
				priv->colors[GAUGE_COL_GRADIENT_BEGIN_NITE].blue/65535.0);
		cairo_pattern_add_color_stop_rgb(gradient, 2*priv->radius,
				priv->colors[GAUGE_COL_GRADIENT_END_NITE].red/65535.0,
				priv->colors[GAUGE_COL_GRADIENT_END_NITE].green/65535.0,
				priv->colors[GAUGE_COL_GRADIENT_END_NITE].blue/65535.0);
	}
	cairo_set_source(cr, gradient);
	cairo_arc(cr, priv->xc, priv->yc, (0.950 * priv->radius), 0, 2 * M_PI);
	cairo_fill(cr);
	cairo_pattern_destroy(gradient);

	/* Gauge background inside the bezel */

	if (priv->daytime_mode == MTX_DAY)
	{
		cairo_set_source_rgb (cr, priv->colors[GAUGE_COL_BG_DAY].red/65535.0,
				priv->colors[GAUGE_COL_BG_DAY].green/65535.0,
				priv->colors[GAUGE_COL_BG_DAY].blue/65535.0);
	}
	else
	{
		cairo_set_source_rgb (cr, priv->colors[GAUGE_COL_BG_NITE].red/65535.0,
				priv->colors[GAUGE_COL_BG_NITE].green/65535.0,
				priv->colors[GAUGE_COL_BG_NITE].blue/65535.0);
	}
	cairo_arc(cr, priv->xc, priv->yc, (0.900 * priv->radius), 0, 2 * M_PI);
	cairo_fill(cr);

	for (layer=0;layer<priv->max_layers;layer++)
	{
		/* The warning color ranges */

		for (i=0;i<priv->w_ranges->len;i++)
		{
			range = g_array_index(priv->w_ranges,MtxWarningRange *, i);
			if (range->layer != layer) /* Draw layers in order, 0 being "lowest" */

				continue;
			cairo_set_source_rgb(cr,range->color[priv->daytime_mode].red/65535.0,
					range->color[priv->daytime_mode].green/65535.0,
					range->color[priv->daytime_mode].blue/65535.0);
			/* percent of full scale is (lbound-range_lbound)/(fullspan)*/

			angle1 = (range->lowpoint-priv->lbound)/(priv->ubound-priv->lbound);
			angle2 = (range->highpoint-priv->lbound)/(priv->ubound-priv->lbound);
			/*printf("gauge warning range should be from %f, to %f of full scale\n",angle1, angle2);*/

			lwidth = priv->radius*range->lwidth < 1 ? 1: priv->radius*range->lwidth;
			cairo_set_line_width (cr, lwidth);
			if (priv->rotation == MTX_ROT_CW)
				cairo_arc(cr, priv->xc, priv->yc, (range->inset * priv->radius),(priv->start_angle+(angle1*(priv->sweep_angle)))*(M_PI/180.0), (priv->start_angle+(angle2*(priv->sweep_angle)))*(M_PI/180.0));
			else
				cairo_arc(cr, priv->xc, priv->yc, (range->inset * priv->radius),(priv->start_angle+priv->sweep_angle-(angle2*(priv->sweep_angle)))*(M_PI/180.0), (priv->start_angle+priv->sweep_angle-(angle1*(priv->sweep_angle)))*(M_PI/180.0));
			cairo_stroke(cr);
		}

		/* NEW STYLE Gauge tick groups */

		for (i=0;i<priv->tick_groups->len;i++)
		{
			tgroup = g_array_index(priv->tick_groups,MtxTickGroup *, i);
			if (tgroup->layer != layer) /* Draw layers in order, 0 being "lowest" */

				continue;
			cairo_set_source_rgb (cr,
					tgroup->maj_tick_color[priv->daytime_mode].red/65535.0,
					tgroup->maj_tick_color[priv->daytime_mode].green/65535.0,
					tgroup->maj_tick_color[priv->daytime_mode].blue/65535.0);

			deg_per_major_tick = (tgroup->sweep_angle)/(float)(tgroup->num_maj_ticks-1);
			deg_per_minor_tick = deg_per_major_tick/(float)(1+tgroup->num_min_ticks);

			insetfrom = priv->radius * tgroup->maj_tick_inset;
			if (priv->rotation == MTX_ROT_CW)
				counter = tgroup->start_angle *(M_PI/180.0);
			else
				counter = (tgroup->start_angle+tgroup->sweep_angle) *(M_PI/180.0);
			if (tgroup->text)
			{
				vector = g_strsplit(tgroup->text,",",-1);
				count = g_strv_length(vector);
				tmpbuf = g_utf8_strup(tgroup->font,-1);
				if (g_strrstr(tmpbuf,"BOLD"))
					weight = CAIRO_FONT_WEIGHT_BOLD;
				else
					weight = CAIRO_FONT_WEIGHT_NORMAL;
				if (g_strrstr(tmpbuf,"OBLIQUE"))
					slant = CAIRO_FONT_SLANT_OBLIQUE;
				else if (g_strrstr(tmpbuf,"ITALIC"))
					slant = CAIRO_FONT_SLANT_ITALIC;
				else
					slant = CAIRO_FONT_SLANT_NORMAL;
				g_free(tmpbuf);
				cairo_select_font_face (cr, tgroup->font, slant, weight);
				cairo_set_font_size (cr, (priv->radius * tgroup->font_scale));
			}
			for (j=0;j<tgroup->num_maj_ticks;j++)
			{
				inset = tgroup->maj_tick_length * priv->radius;

				lwidth = (priv->radius/10)*tgroup->maj_tick_width < 1 ? 1: (priv->radius/10)*tgroup->maj_tick_width;
				cairo_set_line_width (cr, lwidth);
				cairo_move_to (cr,
						priv->xc + (priv->radius - insetfrom) * cos (counter),
						priv->yc + (priv->radius - insetfrom) * sin (counter));
				cairo_line_to (cr,
						priv->xc + (priv->radius - insetfrom - inset) * cos (counter),
						priv->yc + (priv->radius - insetfrom - inset) * sin (counter));
				cairo_stroke (cr);
				if ((vector) && (j < count)) /* If not null */

				{
					cairo_save(cr);
					cairo_set_source_rgb (cr,
							tgroup->text_color[priv->daytime_mode].red/65535.0,
							tgroup->text_color[priv->daytime_mode].green/65535.0,
							tgroup->text_color[priv->daytime_mode].blue/65535.0);
					cairo_text_extents (cr, vector[j], &extents);
					/* Gets the radius of a circle that encompasses the
					 * rectangle of text on screen */

					rad = sqrt(pow(extents.width,2)+pow(extents.height,2))/2.0;
					cairo_move_to (cr,
							priv->xc + (priv->radius - tgroup->text_inset*priv->radius - rad) * cos (counter) - extents.width/2.0-extents.x_bearing,
							priv->yc + (priv->radius - tgroup->text_inset*priv->radius - rad) * sin (counter) + extents.height/2.0);
					cairo_show_text (cr, vector[j]);
					cairo_restore(cr);
				}
				/* minor ticks */

				if ((tgroup->num_min_ticks > 0) && (j < (tgroup->num_maj_ticks-1)))
				{
					cairo_save (cr); /* stack-pen-size */

					cairo_set_source_rgb (cr,
							tgroup->min_tick_color[priv->daytime_mode].red/65535.0,
							tgroup->min_tick_color[priv->daytime_mode].green/65535.0,
							tgroup->min_tick_color[priv->daytime_mode].blue/65535.0);
					inset = tgroup->min_tick_length * priv->radius;
					mintick_inset = priv->radius * tgroup->min_tick_inset;
					lwidth = (priv->radius/10)*tgroup->min_tick_width < 1 ? 1: (priv->radius/10)*tgroup->min_tick_width;
					cairo_set_line_width (cr, lwidth);
					for (k=1;k<=tgroup->num_min_ticks;k++)
					{
						if (priv->rotation == MTX_ROT_CW)
							subcounter = (k*deg_per_minor_tick)*(M_PI/180.0);
						else
							subcounter = -(k*deg_per_minor_tick)*(M_PI/180.0);
						cairo_move_to (cr,
								priv->xc + (priv->radius - mintick_inset) * cos (counter+subcounter),
								priv->yc + (priv->radius - mintick_inset) * sin (counter+subcounter));
						cairo_line_to (cr,
								priv->xc + (priv->radius - mintick_inset - inset) * cos (counter+subcounter),
								priv->yc + (priv->radius - mintick_inset - inset) * sin (counter+subcounter));
						cairo_stroke (cr);
					}
					cairo_restore (cr); /* stack-pen-size */

				}
				if (priv->rotation == MTX_ROT_CW)
					counter += (deg_per_major_tick)*(M_PI/180);
				else
					counter -= (deg_per_major_tick)*(M_PI/180);
			}
			g_strfreev(vector);
		}
		cairo_stroke (cr);

		/* Polygons */

		for (i=0;i<priv->polygons->len;i++)
		{
			poly = g_array_index(priv->polygons,MtxPolygon *, i);
			if (poly->layer != layer)
				continue;
			cairo_set_source_rgb(cr,
					poly->color[priv->daytime_mode].red/65535.0,
					poly->color[priv->daytime_mode].green/65535.0,
					poly->color[priv->daytime_mode].blue/65535.0);
			lwidth = priv->radius*poly->line_width < 1 ? 1: priv->radius*poly->line_width;
			cairo_set_line_width (cr, lwidth);
			cairo_set_line_join(cr,(cairo_line_join_t)poly->join_style);
			switch (poly->line_style)
			{
				case GDK_LINE_SOLID:
					cairo_set_dash(cr,0,0,0);
					break;
				case GDK_LINE_ON_OFF_DASH:
					cairo_set_dash(cr,dashes,2,0);
					break;
				default:
					break;
			}
			switch (poly->type)
			{
				case MTX_CIRCLE:
					cairo_arc(cr,
							priv->xc+((MtxCircle *)poly->data)->x*priv->radius,
							priv->yc+((MtxCircle *)poly->data)->y*priv->radius,
							((MtxCircle *)poly->data)->radius*priv->radius,
							0,2*M_PI);
					break;
				case MTX_RECTANGLE:
					cairo_rectangle(cr,
							priv->xc+((MtxRectangle *)poly->data)->x*priv->radius,
							priv->yc+((MtxRectangle *)poly->data)->y*priv->radius,
							((MtxRectangle *)poly->data)->width*priv->radius,
							((MtxRectangle *)poly->data)->height*priv->radius);
					break;
				case MTX_ARC:
					cairo_save(cr);
					cairo_translate(cr,
							priv->xc+(((MtxArc *)poly->data)->x*priv->radius),
							priv->yc+(((MtxArc *)poly->data)->y*priv->radius));
					cairo_scale(cr,
							((MtxArc *)poly->data)->width*priv->radius,
							((MtxArc *)poly->data)->height*priv->radius);
					cairo_arc(cr,
							0.0,
							0.0,
							1.0,
							((MtxArc *)poly->data)->start_angle * (M_PI/180.0),(((MtxArc *)poly->data)->sweep_angle+((MtxArc *)poly->data)->start_angle)*(M_PI/180));
					if (poly->filled)
					{
						cairo_line_to(cr,0,0);
						cairo_close_path(cr);
					}
					cairo_restore(cr);
					break;
				case MTX_GENPOLY:
					num_points = ((MtxGenPoly *)poly->data)->num_points;
					if (num_points < 1)
						break;
					cairo_move_to(cr,
							priv->xc + (((MtxGenPoly *)poly->data)->points[0].x * priv->radius),
							priv->yc + (((MtxGenPoly *)poly->data)->points[0].y * priv->radius));
					for (j=1;j<num_points;j++)
					{
						cairo_line_to(cr,
								priv->xc + (((MtxGenPoly *)poly->data)->points[j].x * priv->radius),
								priv->yc + (((MtxGenPoly *)poly->data)->points[j].y * priv->radius));
					}
					cairo_close_path(cr);
					break;
				default:
					break;
			}
			if (poly->filled)
				cairo_fill(cr);
			else
				cairo_stroke(cr);
		}
		/* Render all the text blocks */

		for (i=0;i<priv->t_blocks->len;i++)
		{
			tblock = g_array_index(priv->t_blocks,MtxTextBlock *, i);
			if (tblock->layer != layer)
				continue;
			cairo_set_source_rgb (cr,
					tblock->color[priv->daytime_mode].red/65535.0,
					tblock->color[priv->daytime_mode].green/65535.0,
					tblock->color[priv->daytime_mode].blue/65535.0);

			tmpbuf = g_utf8_strup(tblock->font,-1);
			if (g_strrstr(tmpbuf,"BOLD"))
				weight = CAIRO_FONT_WEIGHT_BOLD;
			else
				weight = CAIRO_FONT_WEIGHT_NORMAL;
			if (g_strrstr(tmpbuf,"OBLIQUE"))
				slant = CAIRO_FONT_SLANT_OBLIQUE;
			else if (g_strrstr(tmpbuf,"ITALIC"))
				slant = CAIRO_FONT_SLANT_ITALIC;
			else
				slant = CAIRO_FONT_SLANT_NORMAL;
			g_free(tmpbuf);
			cairo_select_font_face (cr, tblock->font, slant, weight);

			cairo_set_font_size (cr, (priv->radius * tblock->font_scale));
			cairo_text_extents (cr, tblock->text, &extents);
			cairo_move_to (cr,
					priv->xc-(extents.width/2 + extents.x_bearing)+(tblock->x_pos*priv->radius),
					priv->yc-(extents.height/2 + extents.y_bearing)+(tblock->y_pos*priv->radius));
			cairo_show_text (cr, tblock->text);
		}
		cairo_stroke(cr);
	}

	/* Tattletale (ghost needle showing peak value) */

	if (priv->show_tattletale)
	{
		if (priv->clamped == CLAMP_UPPER)
			val = priv->ubound;
		else if (priv->clamped == CLAMP_LOWER)
			val = priv->lbound;
		else
			val = priv->peak;
		tmpf = (val-priv->lbound)/(priv->ubound-priv->lbound);

		if (priv->rotation == MTX_ROT_CW)
			tattle_pos = (priv->start_angle+(tmpf*priv->sweep_angle))*(M_PI/180);
		else
			tattle_pos = ((priv->start_angle+priv->sweep_angle)-(tmpf*priv->sweep_angle))*(M_PI/180);
		if (priv->daytime_mode == MTX_DAY)
		{
			cairo_set_source_rgba (cr, priv->colors[GAUGE_COL_NEEDLE_DAY].red/65535.0,
					priv->colors[GAUGE_COL_NEEDLE_DAY].green/65535.0,
					priv->colors[GAUGE_COL_NEEDLE_DAY].blue/65535.0,
					priv->tattletale_alpha);
		}
		else
		{
			cairo_set_source_rgba (cr, priv->colors[GAUGE_COL_NEEDLE_NITE].red/65535.0,
					priv->colors[GAUGE_COL_NEEDLE_NITE].green/65535.0,
					priv->colors[GAUGE_COL_NEEDLE_NITE].blue/65535.0,
					priv->tattletale_alpha);
		}
		cairo_set_line_width (cr, 1);

		t_width = priv->needle_width * priv->radius;
		t_tail = priv->needle_tail * priv->radius;
		t_tip = priv->needle_length * priv->radius;
		tip_width = priv->needle_tip_width * priv->radius;
		tail_width = priv->needle_tail_width * priv->radius;
		xc = priv->xc;
		yc = priv->yc;

		priv->tattle_coords[0].x = xc + ((t_tip) * cos (tattle_pos))+((tip_width) * -sin(tattle_pos));
		priv->tattle_coords[0].y = yc + ((t_tip) * sin (tattle_pos))+((tip_width) * cos(tattle_pos));
		priv->tattle_coords[1].x = xc + ((t_tip) * cos (tattle_pos))+((tip_width) * sin(tattle_pos));
		priv->tattle_coords[1].y = yc + ((t_tip) * sin (tattle_pos))+((tip_width) * -cos(tattle_pos));

		if (t_tail < 0)
		{
			priv->tattle_coords[2].x = xc + ((t_tail) * -cos (tattle_pos))+((t_width) * sin(tattle_pos));
			priv->tattle_coords[2].y = yc + ((t_tail) * -sin (tattle_pos))+((t_width) * -cos(tattle_pos));
		}
		else
		{
			priv->tattle_coords[2].x = xc + (t_width) * sin(tattle_pos);
			priv->tattle_coords[2].y = yc + (t_width) * -cos(tattle_pos);
		}

		priv->tattle_coords[3].x = xc + ((t_tail) * -cos (tattle_pos))+((tail_width) * sin (tattle_pos));
		priv->tattle_coords[3].y = yc + ((t_tail) * -sin (tattle_pos))+((tail_width) * -cos (tattle_pos));
		priv->tattle_coords[4].x = xc + ((t_tail) * -cos (tattle_pos))+((tail_width) * -sin (tattle_pos));
		priv->tattle_coords[4].y = yc + ((t_tail) * -sin (tattle_pos))+((tail_width) * cos (tattle_pos));
		if (t_tail < 0)
		{
			priv->tattle_coords[5].x = xc + ((t_tail) * -cos (tattle_pos))+((t_width) * -sin(tattle_pos));
			priv->tattle_coords[5].y = yc + ((t_tail) * -sin (tattle_pos))+((t_width) * cos(tattle_pos));
		}
		else
		{
			priv->tattle_coords[5].x = xc + (t_width) * -sin (tattle_pos);
			priv->tattle_coords[5].y = yc + (t_width) * cos (tattle_pos);
		}
		priv->needle_polygon_points = 6;

		cairo_move_to (cr, priv->tattle_coords[0].x,priv->tattle_coords[0].y);
		cairo_line_to (cr, priv->tattle_coords[1].x,priv->tattle_coords[1].y);
		cairo_line_to (cr, priv->tattle_coords[2].x,priv->tattle_coords[2].y);
		cairo_line_to (cr, priv->tattle_coords[3].x,priv->tattle_coords[3].y);
		cairo_line_to (cr, priv->tattle_coords[4].x,priv->tattle_coords[4].y);
		cairo_line_to (cr, priv->tattle_coords[5].x,priv->tattle_coords[5].y);
		cairo_fill_preserve (cr);
		cairo_stroke(cr);
	}
	cairo_destroy (cr);
	/* SAVE copy of this on tmp pixmap */

	//widget = GTK_WIDGET(gauge); <*>

	cr = gdk_cairo_create(priv->tmp_pixmap);
	gdk_cairo_set_source_pixmap(cr,priv->bg_pixmap,0,0);
	cairo_rectangle(cr,0,0,allocation.width,allocation.height);
	cairo_fill(cr);
	cairo_destroy(cr);
	priv->last_alert_index = -1;
}


/*!
 \brief handler called when a button is pressed,  currently unused
 \param widget is the pointer to the gauge widget
 \param event is the pointer to the GdkEventButton struct containing
 details on the button(s) pressed
 \returns FALSE so other handlers run
 */
gboolean mtx_gauge_face_button_press (GtkWidget *widget,GdkEventButton *event)

{
	GtkWidget *parent = gtk_widget_get_parent(widget);
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(widget);
	GdkWindowEdge edge = (GdkWindowEdge)-1;
	/*printf("gauge button event\n");*/

	/* Right side of window */

	if (event->x > (priv->w-10))
	{
		/* Upper portion */

		if (event->y < 10)
			edge = GDK_WINDOW_EDGE_NORTH_EAST;
		/* Lower portion */

		else if (event->y > (priv->h-10))
			edge = GDK_WINDOW_EDGE_SOUTH_EAST;
		else
			edge = (GdkWindowEdge)-1;
	}
	/* Left Side of window */

	else if (event->x < 10)
	{
		/* If it's in the middle portion */

		/* Upper portion */

		if (event->y < 10)
			edge = GDK_WINDOW_EDGE_NORTH_WEST;
		/* Lower portion */

		else if (event->y > (priv->h-10))
			edge = GDK_WINDOW_EDGE_SOUTH_WEST;
		else
			edge = (GdkWindowEdge)-1;
	}
	else
		edge = (GdkWindowEdge)-1;


	if (event->type == GDK_BUTTON_PRESS)
	{
		switch (event->button)
		{
			case 1: /* left button */

				if ((edge != GDK_WINDOW_EDGE_NORTH_WEST) &&
						(edge != GDK_WINDOW_EDGE_NORTH_EAST) &&
						(edge != GDK_WINDOW_EDGE_SOUTH_WEST) &&
						(edge != GDK_WINDOW_EDGE_SOUTH_EAST) && (GTK_IS_WINDOW(parent)))
				{
					gtk_window_begin_move_drag (GTK_WINDOW(gtk_widget_get_toplevel(widget)),
							event->button,
							event->x_root,
							event->y_root,
							event->time);
				}
				else if (GTK_IS_WINDOW(parent))
				{

					gtk_window_begin_resize_drag (GTK_WINDOW(gtk_widget_get_toplevel(widget)),
							edge,
							event->button,
							event->x_root,
							event->y_root,
							event->time);
				}
				else
				{
					/* Grab click coords, used by
					 * gaugedesigner to ease development
					 */

					priv->last_click_x = (event->x-priv->xc)/priv->xc;
					priv->last_click_y = (event->y-priv->yc)/priv->yc;
				}

				break;
			case 3: /* right button */
				if (GTK_IS_WINDOW(parent))
				{
					/*
					gtk_widget_destroy(widget);
					gtk_main_quit();
					*/
				}
				/* Added api call to do this*/

				else if (priv->show_tattletale)
				{
					priv->peak = priv->lbound;
					generate_gauge_background(MTX_GAUGE_FACE(widget));
				}
				break;
		}
	}
	/*printf("gauge button event ENDING\n");*/

	return FALSE;
}


/*!
 \brief handler called when a button is released,  currently unused
 \param gauge is the pointer to the gauge widget
 \param event is the pointer to a GdkEventButton structure containing
 details on the button(s) released
 \returns FALSE
 */
gboolean mtx_gauge_face_button_release (GtkWidget *gauge,GdkEventButton *event)

{
	/*printf("button release\n");*/

	return FALSE;
}


/*!
 \brief handler called when there is motion of the gauge
 \param gauge is the pointer to the gauge widget
 \param event is the pointer to a GdkEventMotion structure containing
 details on the motion event
 \returns FALSE
 */
gboolean mtx_gauge_face_motion_event (GtkWidget *gauge,GdkEventMotion *event)
{
	/* We don't care, but return FALSE to propogate properly */

	/*printf("motion in gauge, returning false\n");*/

	return FALSE;
}


/*!
 \brief handler called when there is keyboard events on the gauge
 \param gauge is the pointer to the gauge widget
 \param event is the pointer to a GdkEventKey structure containing
 details on the keyboard event
 \returns FALSE
 */
gboolean mtx_gauge_face_key_event (GtkWidget *gauge,GdkEventKey *event)
{
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	/* We don't care, but return FALSE to propogate properly */

	switch (event->keyval)
	{
		case GDK_T:
		case GDK_t:
			if (priv->show_tattletale)
				priv->show_tattletale = FALSE;
			else
				priv->show_tattletale = TRUE;
			generate_gauge_background(MTX_GAUGE_FACE(gauge));
			mtx_gauge_face_redraw_canvas (MTX_GAUGE_FACE(gauge));
			break;
		case GDK_A:
		case GDK_a:
			if (priv->antialias)
				priv->antialias = FALSE;
			else
				priv->antialias = TRUE;
			generate_gauge_background(MTX_GAUGE_FACE(gauge));
			mtx_gauge_face_redraw_canvas (MTX_GAUGE_FACE(gauge));

		case GDK_R:
		case GDK_r:
			priv->peak = priv->lbound;
			break;
			/*
		case GDK_Q:
		case GDK_q:
			gtk_main_quit();
			break;
			*/
		default:
			break;

	}
	return FALSE;
}


/*!
 \brief sets the INITIAL sizeof the widget
 \param widget is the pointer to the gauge widget
 \param requisition is the pointer to the GdkRequisition structure to set
 the vars within
 \returns void
 */
void mtx_gauge_face_size_request(GtkWidget *widget, GtkRequisition *requisition)
{
	g_return_if_fail(widget != NULL);
	g_return_if_fail(MTX_IS_GAUGE_FACE(widget));
	g_return_if_fail(requisition != NULL);

	requisition->width = 75;
	requisition->height = 75;
}


/*!
  \brief Calculates a box that contains all the passed points
  \param coords is a pointer to a list of MtxPoint structures
  \param count is the number of MtxPoint structures
  \param box is a pointer to be filled with the bounding box coordinates
  */
void calc_bounding_box(MtxPoint *coords, gint count, GdkRectangle *box)
{
	gint x_min = G_MAXINT;
	gint y_min = G_MAXINT;
	gint x_max = G_MININT;
	gint y_max = G_MININT;
	gint i = 0;
	for (i=0;i<count;i++)
	{
		if (coords[i].x < x_min)
			x_min = coords[i].x - 2;
		if (coords[i].x > x_max)
			x_max = coords[i].x + 2;
		if (coords[i].y < y_min)
			y_min = coords[i].y - 2;
		if (coords[i].y > y_max)
			y_max = coords[i].y + 2;
	}
	box->x = x_min;
	box->y = y_min;
	box->width = x_max-x_min;
	box->height = y_max-y_min;
}
