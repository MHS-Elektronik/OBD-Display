#ifndef __GTK_DIGITAL_SHOW_H__
#define __GTK_DIGITAL_SHOW_H__

#include "gtk-ex-frame.h"

#define GTK_DIGITAL_MIN_LIMIT_COLOR  1
#define GTK_DIGITAL_MIN_WARN_COLOR   2
#define GTK_DIGITAL_MAX_LIMIT_COLOR  3
#define GTK_DIGITAL_MAX_WARN_COLOR   4

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define GTK_DIGITAL_SHOW_TYPE (gtk_digital_show_get_type())
#define GTK_DIGITAL_SHOW(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), GTK_DIGITAL_SHOW_TYPE, GtkDigitalShow))
#define GTK_DIGITAL_SHOW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), GTK_DIGITAL_SHOW_TYPE, GtkDigitalShowClass))
#define GTK_IS_GTK_DIGITAL_SHOW(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), GTK_DIGITAL_SHOW_TYPE))
#define GTK_IS_GTK_DIGITAL_SHOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_DIGITAL_SHOW_TYPE))
#define GTK_DIGITAL_SHOW_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), GTK_DIGITAL_SHOW_TYPE, GtkDigitalShowClass))

/*
 * Main object structure
 */
typedef struct _GtkDigitalShow GtkDigitalShow;

struct _GtkDigitalShow
  {
  GtkExFrame parent;

  GtkWidget *base_vbox;
  GtkWidget *desc_label;
  GtkWidget *unit_label;
  GtkWidget *value_lcd;
  GtkWidget *value_pbar;
  GdkColor fg_color;
  GdkColor bg_color;
  GdkColor min_limit_fg_color;
  GdkColor min_limit_bg_color;
  GdkColor min_warn_fg_color;
  GdkColor min_warn_bg_color;
  GdkColor max_warn_fg_color;
  GdkColor max_warn_bg_color;
  GdkColor max_limit_fg_color;
  GdkColor max_limit_bg_color;
  gboolean show_pbar;
  gchar *description;
  gchar *unit;
  gdouble min;
  gdouble min_limit;
  gdouble min_warn;
  gdouble max_warn;
  gdouble max_limit;
  gdouble max;
  guint digits;
  gdouble value;

  gboolean negative;
  guint char_size;
  };

/*
 * Class definition
 */
typedef struct _GtkDigitalShowClass GtkDigitalShowClass;

struct _GtkDigitalShowClass
  {
  GtkFrameClass parent_class;
  };


/*
 * Public methods
 */
GType gtk_digital_show_get_type (void)G_GNUC_CONST;

GtkWidget *gtk_digital_show_new(void) G_GNUC_MALLOC;
GtkWidget *gtk_digital_show_new_with_config(const gchar *desc, const gchar *unit, guint digits, gboolean show_pbar,
             gdouble min, gdouble max, gdouble value, GdkColor *fg_color, GdkColor *bg_color) G_GNUC_MALLOC;

void gtk_digital_show_modify_fg(GtkDigitalShow *digital, gint color_index, GdkColor *fg_color);
void gtk_digital_show_modify_bg(GtkDigitalShow *digital, gint color_index, GdkColor *bg_color);
void gtk_digital_show_visible_pbar(GtkDigitalShow *digital, gboolean show_pbar);
void gtk_digital_show_set_description(GtkDigitalShow *digital, const gchar *description);
void gtk_digital_show_set_unit(GtkDigitalShow *digital, const gchar *uint);
void gtk_digital_show_set_digits(GtkDigitalShow *digital, guint digits);
void gtk_digital_show_set_value_range(GtkDigitalShow *digital, gdouble min, gdouble max);
void gtk_digital_show_set_value(GtkDigitalShow *digital, gdouble value);

G_END_DECLS

#endif
