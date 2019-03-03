/*
 * Copyright (C) 2002-2012 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

/*!
  \file include/defines.h
  \ingroup Headers
  \brief Special definitions, macros and platform magic
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __DEFINES_H__
#define __DEFINES_H__

#define OBJ_GET(object, name) g_object_get_data(G_OBJECT(object),name)
#define OBJ_SET(object, name, data) g_object_set_data(G_OBJECT(object),name,data)
#define OBJ_SET_FULL(object, name, data, func) g_object_set_data_full(G_OBJECT(object),name,data,(GDestroyNotify)func)

/* g_dataset_get/set macros */

#define DATA_GET(dataset, name) g_dataset_get_data(dataset,name)
#define DATA_SET(dataset, name, data) g_dataset_set_data(dataset,name,data)
#define DATA_SET_FULL(dataset, name, data, func) g_dataset_set_data_full(dataset,name,data,(GDestroyNotify)func)

#endif

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
