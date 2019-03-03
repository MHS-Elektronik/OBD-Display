/*
 * Copyright (C) 2003 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Linux Megasquirt tuning software
 *
 *
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute, etc. this as long as all the source code
 * is made available for FREE.
 *
 * No warranty is made or implied. You use this program at your own risk.
 */

/* Configfile structs. (derived from an older version of XMMS) */

#ifndef __CONFIGFILE_H__
#define __CONFIGFILE_H__

#include <glib.h>

/* Structures */

/*!
 \brief The ConfigLine struct stores just the key and value for a Line within
 a ConfigSection
 \see ConfigSection
 */
typedef struct
{
        gchar *key;
        gchar *value;
}
ConfigLine;

/*!
 \brief The ConfigSection struct stores the section name anda GList of
 lines
 */
typedef struct
{
        gchar *name;
        GList *lines;
}
ConfigSection;

/*!
 \brief The ConfigFile struct stores a GList of Sections and the filename
 \see ConfigSection
 */
typedef struct
{
        GList *sections;
	gchar * filename;
}
ConfigFile;

/* Structures */

/* Prototypes */
ConfigFile *cfg_new(void);
ConfigFile *cfg_open_file(const gchar * filename);
gboolean cfg_write_file(ConfigFile * cfg, const gchar * filename);
void cfg_free(ConfigFile * cfg);
gboolean cfg_read_string(ConfigFile *cfg, const gchar *section, const gchar *key, gchar ** value);
gboolean cfg_read_int(ConfigFile *cfg, const gchar *section, const gchar *key, gint *value);
gboolean cfg_read_boolean(ConfigFile *cfg, const gchar *section, const gchar *key, gboolean *value);
gboolean cfg_read_float(ConfigFile *cfg, const gchar *section, const gchar *key, gfloat *value);
gboolean cfg_read_double(ConfigFile *cfg, const gchar *section, const gchar *key, gdouble *value);
void cfg_write_string(ConfigFile *cfg, const gchar *section, const gchar *key, const gchar *value);
void cfg_write_int(ConfigFile *cfg, const gchar *section, const gchar *key, gint value);
void cfg_write_boolean(ConfigFile * cfg, const gchar *section, const gchar *key, gboolean value);
void cfg_write_float(ConfigFile * cfg, const gchar *section, const gchar *key, gfloat value);
void cfg_write_double(ConfigFile * cfg, const gchar *section, const gchar *key, gdouble value);
void cfg_remove_key(ConfigFile * cfg, const gchar * section, const gchar * key);
void cfg_clear_section(ConfigFile *cfg, const gchar *section);
/* Prototypes */


#endif


