#ifndef __C_ETABLE__
#define __C_ETABLE__

#include <gtk/gtk.h>
#include <pango/pango.h>

G_BEGIN_DECLS

#define ET_COL_SET_COLOR   0x01
#define ET_COL_SINGLE_LINE 0x02

typedef gint (*TGetLineCB)(guint index, gpointer user_data, gchar *line, GdkColor *color, guint *flags);

struct TETableDesc
  {
  const gchar *name;
  const gchar *templ;
  int show;
  GdkColor color;
  };

/*
 * Type checking and casting macros
 */
#define ETABLE_TYPE (etable_get_type())
#define ETABLE(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), ETABLE_TYPE, ETable))
#define ETABLE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), ETABLE_TYPE, ETableClass))
#define GTK_IS_ETABLE(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), ETABLE_TYPE))
#define GTK_IS_ETABLE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), ETABLE_TYPE))
#define ETABLE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), ETABLE_TYPE, ETableClass))

/* Private structure type */
typedef struct _ETablePrivate ETablePrivate;

/*
 * Main object structure
 */
struct TETableCol
  {
  guint show;
  gint x_pos;
  GtkWidget *frame;
  GtkLabel *label;
  GdkColor color;
  gchar text[80];
  gchar templ[80];
  }; 
 
typedef struct _ETable ETable;

struct _ETable
  {
  GtkVBox parent;
  gboolean auto_scroll;
  gint row_size;
  gint col_size;
  struct TETableCol *col;
  /*< private > */
  ETablePrivate *priv;
  };

/*
 * Class definition
 */
typedef struct _ETableClass ETableClass;

struct _ETableClass
  {
	GtkVBoxClass parent_class;
  };

/*
 * Public methods
 */
GType etable_get_type (void)G_GNUC_CONST;
GtkWidget *etable_new(TGetLineCB get_line_cb, gpointer user_data, const struct TETableDesc *tab_desc);

void etable_set_row_size(ETable *et, gint row_size);
void etable_set_scroll_to(ETable *et, int mode, unsigned int pos);
gboolean etable_set_view_font(ETable *et, const gchar *font_name);
gboolean etable_set_col_color(ETable *et, gint col_idx, GdkColor *color);
gboolean etable_set_col_show(ETable *et, gint col_idx, gboolean show);
gboolean etable_set_col_header_text(ETable *et, gint col_idx, gchar *text);

G_END_DECLS

#endif
