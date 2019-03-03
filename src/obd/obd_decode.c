/*******************************************************************************
                        obd_decode.c  -  description
                             -------------------
    begin             : 19.01.2019
    copyright         : (C) 2019 by MHS-Elektronik GmbH & Co. KG, Germany
                               http://www.mhs-elektronik.de
    autho             : Klaus Demlehner, klaus@mhs-elektronik.de
 *******************************************************************************

This file is based on "OBDII.c" from Ethan Vaughan.
The project page: https://github.com/ejvaughan/obdii

Note: This file is directly included by "obd_db.c" is not a part of the make file!

*/
/***************************************************************************
 *                                                                         *
 *   This program is free software, you can redistribute it and/or modify  *
 *   it under the terms of the MIT License <LICENSE.TXT or                 *
 *   http://opensource.org/licenses/MIT>                                   *
 *                                                                         *
 ***************************************************************************/


static void OBD2DecU8Raw(TOBD2Data *obd_data, uint8_t *data, uint8_t len)
{
obd_data->ObdU32Value = data[2];
}


static void OBD2DecU16Raw(TOBD2Data *obd_data, uint8_t *data, uint8_t len)
{
obd_data->ObdU32Value = data[2] << 8 | data[3];
}


static void OBD2DecU32Raw(TOBD2Data *obd_data, uint8_t *data, uint8_t len)
{
obd_data->ObdU32Value = (data[2] << 24) | (data[3] << 16) | (data[4] << 8) | data[5];
}


static void OBD2DecU8(TOBD2Data *obd_data, uint8_t *data, uint8_t len)
{
obd_data->ObdValue1 = data[2];
}


static void OBD2DecU16(TOBD2Data *obd_data, uint8_t *data, uint8_t len)
{
obd_data->ObdValue1 = data[2] << 8 | data[3];
}


static void OBD2DecS16(TOBD2Data *obd_data, uint8_t *data, uint8_t len)
{
obd_data->ObdValue1 = ((signed char)data[2] << 8 | data[3]);
}


static void OBD2DecOxSensorValues(TOBD2Data *obd_data, uint8_t *data, uint8_t len)
{
obd_data->ObdValue1 = data[2] / 200.0f;
obd_data->ObdValue2 = data[3] * 0.78125f - 100.0f;
}


static void OBD2DecOxSensorValues2(TOBD2Data *obd_data, uint8_t *data, uint8_t len)
{
obd_data->ObdValue1 = 8.0 / 65536.0 * (data[4] << 8 | data[5]);
obd_data->ObdValue2 = 2.0 / 65536.0 * (data[2] << 8 | data[3]);
}


static void OBD2DecOxSensorValues3(TOBD2Data *obd_data, uint8_t *data, uint8_t len)
{
obd_data->ObdValue1 = 2.0 / 65536.0 * (data[2] << 8 | data[3]);
obd_data->ObdValue2 = (data[4] << 8 | data[5]) / 256.0 - 128.0;
}


static void OBD2DecNop(TOBD2Data *obd_data, uint8_t *data, uint8_t len)
{
}


static void OBD2DecDTCs(TOBD2Data *obd_data, uint8_t *data, uint8_t len)
{
uint32_t dtc_count;
uint8_t dtc_h, dtc_l;
uint16_t *dtc;

len -= 1;
data++;
if (len % 2)
  return;
dtc_count = len / 2;
if (!(dtc = mhs_malloc(dtc_count * sizeof(uint16_t))))
  return;
obd_data->ObdDTCs.Size = dtc_count;
obd_data->ObdDTCs.DtcNo = dtc;
for (; dtc_count; dtc_count--)
  {
  dtc_h = *data++;
  dtc_l = *data++;
  *dtc++ = ((uint16_t)dtc_h << 8) | dtc_l;
  }
}


static void OBD2DecString(TOBD2Data *obd_data, uint8_t *data, uint8_t len)
{
char *str;

len -= 2;
safe_free(obd_data->ObdString);
if ((str = mhs_malloc(len + 1)))
  {
  strncpy(str, (char *)&data[2], len);
  str[len] = '\0';
  obd_data->ObdString = str;
  }
}
