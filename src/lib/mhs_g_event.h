#ifndef __MHS_G_EVENT_H__
#define __MHS_G_EVENT_H__

#include <glib.h>

typedef gint(*MhsGEventCb)(guint events, gpointer user_data);

GSource *mhs_g_event_source_new(void);
guint mhs_g_event_add_full(gint priority, GSourceFunc function, gpointer data, GDestroyNotify notify);
guint mhs_g_event_add(GSourceFunc function, gpointer data);

gint mhs_g_event_set_mask(guint id, guint mask);
gint mhs_g_reset_events(guint id, guint events);
gint mhs_g_set_events(guint id, guint events);

void mhs_g_init_events(void);
void mhs_g_destroy_events(void);

#endif
