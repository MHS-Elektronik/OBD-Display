#ifndef __GTK_LCD_H__
#define __GTK_LCD_H__

G_BEGIN_DECLS

typedef struct _GtkLcdClass GtkLcdClass;
typedef struct _GtkLcd      GtkLcd;

#define GTK_TYPE_LCD            (gtk_lcd_get_type ())
#define GTK_LCD(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_LCD, GtkLcd))
#define GTK_LCD_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_LCD, GtkLcdClass))
#define GTK_IS_LCD(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_LCD))
#define GTK_IS_LCD_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_LCD))
#define GTK_LCD_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_LCD, GtkLcdClass))

GType gtk_lcd_get_type(void) G_GNUC_CONST;

GtkWidget *gtk_lcd_new(void) G_GNUC_MALLOC;
GtkWidget *gtk_lcd_new_with_config(gboolean negativ, guint size, guint digits) G_GNUC_MALLOC;

void gtk_lcd_set_fg(GtkLcd *lcd, GdkColor *color);
void gtk_lcd_set_value(GtkLcd *lcd, gdouble value);
void gtk_lcd_config(GtkLcd *lcd, gboolean negativ, guint size, guint digits);

G_END_DECLS

#endif /* !__CLOCK_LCD_H__ */
