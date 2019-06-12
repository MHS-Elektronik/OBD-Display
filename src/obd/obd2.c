/*******************************************************************************
                            obd2.c  -  description
                             -------------------
    begin             : 19.01.2019
    last modify       : 26.04.2019
    copyright         : (C) 2019 by MHS-Elektronik GmbH & Co. KG, Germany
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
#include <string.h>
#include "util.h"
#include "obd_types.h"
#include "obd_db.h"
#include "isotp.h"
#include "obd2.h"

/* <*>
static void Obd2GetAktivPidCount(TObd2 *obd)
{
int cnt, idx;
TOBD2Data *current_data;

if (!obd)
  return;
cnt = 0;
for (idx = 0; idx < obd->CurrentDataSize; idx++)
  {
  current_data = &obd->CurrentData[idx];
  if (!current_data->Cmd)
    continue;
  if (current_data->Cmd->Flags & 0x80)
    continue;
  if ((current_data->Supported) && (current_data->Enabled))
    cnt++;
  }
obd->AktivPidCount = cnt;
} */


static TOBD2Data *Obd2GetNext(TObd2 *obd)
{
int cnt, idx;
TOBD2Data *current_data;

if (!obd)
  return(NULL);
cnt = 0;
idx = obd->AktivIndex;
for (idx++; idx < obd->CurrentDataSize; idx++)
  {
  cnt++;
  current_data = &obd->CurrentData[idx];
  if (!current_data->Cmd)
    continue;
  if (current_data->Cmd->Flags & 0x80)
    continue;
  if ((current_data->Supported) && (current_data->Enabled))
    {
    obd->AktivIndex = idx;
    return(current_data);
    }
  }
for (idx = 0; ++cnt <= obd->CurrentDataSize; obd->AktivIndex++)
  {
  current_data = &obd->CurrentData[idx];
  if (!current_data->Cmd)
    continue;
  if (current_data->Cmd->Flags & 0x80)
    continue;
  if ((current_data->Supported) && (current_data->Enabled))
    {
    obd->AktivIndex = idx;
    return(current_data);
    }
  }
obd->AktivIndex = 0;
return(NULL);
}


static int32_t Obd2GetData(TObd2 *obd, TOBD2Data *obd_data)
{
const TOBD2Cmd *cmd;
uint8_t *rx_data;
uint32_t rx_size;
int32_t res;

res = 0;
if (!(cmd = obd_data->Cmd))
  return(-1);
if (IsotpSendReceive(obd->Isotp, cmd->Payload, 2, &rx_data, &rx_size, 1000) < 0)
  return(-1);
// Datenlänge Prüfen
if (cmd->DataLength)
  {
  if (rx_size != cmd->DataLength)
    res = -1;
  }
else
  {
  if (rx_size < 3)
    res = -1;
  }
if (!res)
  {
  // Mode / SID prüfen
  if (cmd->Flags & 0x01)
    {
    if (rx_data[0] != (OBD2CmdGetMode(cmd) | 0x40))
      res = -1;
    }
  else
    {
    if ((rx_data[0] != (OBD2CmdGetMode(cmd) | 0x40)) || (rx_data[1] != OBD2CmdGetPID(cmd)))
      res = -1;
    }
  }
if (!res)
  {
  if (cmd->ResponseDecoder)
    (cmd->ResponseDecoder)(obd_data, rx_data, rx_size);
  if (cmd->Scale > 0.0f)
    obd_data->ObdValue1 = obd_data->ObdValue1 * cmd->Scale + cmd->Offset;
  obd_data->Status = 0x01;
  obd_data->Update = 0xFF;
  }
mhs_queue_data_free(rx_data);
return(res);
}


static int32_t Obd2GetSupported(TObd2 *obd, TOBD2Data *obd_data, int obd_data_size)
{
uint8_t pid, pid_idx;
int32_t res, idx;
TOBD2Data *obd_data_item;
uint32_t supported_pids[4];

if (!obd)
  return(-1);
for (idx = 0; idx < 4; idx++)
  supported_pids[idx] = 0;
res = 0;
pid_idx = 0;
for (idx = 0; idx < obd_data_size; idx++)
  {
  obd_data_item = &obd_data[idx];
  if (!obd_data_item->Cmd)
    continue;
  if (!(obd_data_item->Cmd->Flags & 0x80))
    continue;
  if (Obd2GetData(obd, obd_data_item) < 0)
    {
    //res = -1; <*> noch prüfen ?
    break;
    }
  if (pid_idx >= 4)
    {
    res = -1;
    break;
    }
  supported_pids[pid_idx++] = obd_data_item->Value.U32;
  if (mhs_sleep_ex((TMhsEvent *)obd->Thread, 100))  // <*> Zeit
    {
    res = -1;
    break;
    }
  }
if (!res)
  {
  if (!pid_idx)
    res = -1;
  }
if (!res)
  {
  for (idx = 0; idx < obd_data_size; idx++)
    {
    obd_data_item = &obd_data[idx];
    if (!obd_data_item->Cmd)
      continue;
    pid = OBD2CmdGetPID(obd_data_item->Cmd);

    if (pid <= 0x20)
      {
      if (supported_pids[0] & (1 << (0x20 - pid)))  // Supported PIDs 1 - 20
        obd_data_item->Supported = 1;
      }
    else if (pid <= 0x40)
      {
      if (supported_pids[1] & (1 << (0x40 - pid))) // Supported PIDs 21 - 40
        obd_data_item->Supported = 1;
      }
    else if (pid <= 0x60)
      {
      if (supported_pids[2] & (1 << (0x60 - pid))) // Supported PIDs 41 - 60
        obd_data_item->Supported = 1;
      }
    else
      {
      if (supported_pids[3] & (1 << (0x80 - pid))) // Supported PIDs 61 - 80
        obd_data_item->Supported = 1;
      }
    }
  }
return(res);
}


static int32_t Obd2ReadObdData(TObd2 *obd, TOBD2Data *obd_data, int obd_data_size)
{
int32_t res, idx;
 TOBD2Data *obd_data_item;

if (!obd)
  return(-1);
res = 0;
for (idx = 0; idx < obd_data_size; idx++)
  {
  obd_data_item = &obd_data[idx];
  if (!obd_data_item->Cmd)
    continue;
  if ((obd_data_item->Cmd->Flags & 0x80))
    continue;
  if ((!obd_data_item->Supported) || (!obd_data->Enabled))
    continue;  
  if (Obd2GetData(obd, obd_data_item) < 0)
    continue;
    /*{ 
    //res = -1; <*> noch prüfen ?
    break;
    } */
  if (mhs_sleep_ex((TMhsEvent *)obd->Thread, 100))  // <*> Zeit
    {
    res = -1;
    break;
    }
  }
return(res);
}


static void Obd2ThreadExecute(TMhsThread *thread)
{
int idx;
TObd2 *obd;
TOBD2Data *obd_data;
uint32_t events;

if (!(obd = (TObd2 *)thread->Data))
  return;
while (thread->Run)
  {
  events = mhs_wait_for_event((TMhsEvent *)obd->Thread, 100);
  if (events & MHS_TERMINATE)
    break;
  switch (obd->Status)
    {
    case OBD2_INIT :
           {
           obd->Status = OBD2_SPPORTED_PID_SCAN;
           }
    case OBD2_SPPORTED_PID_SCAN :
           {
           if (Obd2GetSupported(obd, obd->CurrentData, obd->CurrentDataSize) >= 0)
             {
             if (Obd2GetSupported(obd, obd->VehicleInformation, obd->VehicleInformationSize) >= 0)
               {
               if (Obd2ReadObdData(obd, obd->VehicleInformation, obd->VehicleInformationSize) >= 0)
                 obd->Status = OBD2_START_PID_SCAN;
               }
             }
           break;
           }
    case OBD2_START_PID_SCAN :
           {
           if (obd->Proc)
             (obd->Proc)(obd, OBD2_EVENT_CONNECT, obd->UserData);
           for (idx = 0; idx < obd->CurrentDataSize; idx++)
             {
             obd_data = &obd->CurrentData[idx];
             obd_data->Status = 0x00;
             obd_data->Update = 0x00;
             }
           obd->AktivIndex = 0;
           obd->Status = OBD2_PID_SCAN;
           }
    case OBD2_PID_SCAN :
           {
           if (events & OBD2_DTC_READ_CMD)
             {
             if (Obd2ReadObdData(obd, &obd->Dtc, 1) >= 0)
               {
               if (obd->Proc)
                 (obd->Proc)(obd, OBD2_EVENT_DTC_READ_FINISH, obd->UserData);
               }
             else
               {
               if (obd->Proc)
                 (obd->Proc)(obd, OBD2_EVENT_DTC_READ_ERROR, obd->UserData);
               }
             }
           else
             {
             if ((obd_data = Obd2GetNext(obd)))
               (void)Obd2GetData(obd, obd_data);  // <*> Fehlerauswertung
             }
           break;
           }
    }
  }
}


static void Obd2DataFree(TOBD2Data *obd)
{
if (!obd->Cmd)
  return;
if (obd->Cmd->ResponseType == OBD2TypeString)
  safe_free(obd->ObdString);
else if (obd->Cmd->ResponseType == OBD2TypeDTC)
  safe_free(obd->ObdDTCs.DtcNo);
}


TObd2 *Obd2Create(struct TIsotp *iso_tp, TObd2EventCB proc, void *user_data)
{
TObd2 *obd;
TOBD2Data *obd_data;
const TOBD2Cmd *cmd;
uint8_t pid;
int size, vehicle_inf_size;

if (!iso_tp)
  return(NULL);
if (!(obd = (TObd2 *)mhs_malloc0(sizeof(TObd2))))
  return(NULL);
obd->Isotp = iso_tp;
obd->Proc = proc;
obd->UserData = user_data;

// *** Größe für CurrentData & VehicleInformation bestimmen
size = 0;
vehicle_inf_size = 0;
for (cmd = OBD2Db; cmd->Name; cmd++)
  {
  pid = OBD2CmdGetPID(cmd);
  if (OBD2CmdGetMode(cmd) == 1)
    {
    if (pid > size)
      size = pid;
    }
  else if (OBD2CmdGetMode(cmd) == 9)
    {
    if (pid > vehicle_inf_size)
      vehicle_inf_size = pid;
    }
  }
size++;
vehicle_inf_size++;
// *** Speicher für "CurrentData" allokieren
if (!(obd->CurrentData = (TOBD2Data *)mhs_malloc0((size * sizeof(TOBD2Data)))))
  {
  mhs_free(obd);
  return(NULL);
  }
// *** "CurrentData" mit OBD Datenbank verknüpfen
obd->CurrentDataSize = size;
for (cmd = OBD2Db; cmd->Name; cmd++)
  {
  if (OBD2CmdGetMode(cmd) == 1)
    {
    pid = OBD2CmdGetPID(cmd);
    obd_data = &obd->CurrentData[pid];
    obd_data->Cmd = cmd;
    if (cmd->Flags & 0x10)
      obd_data->Enabled = 1;
    if (cmd->Flags & 0x20)
      obd_data->EnableLock = 1;
    }
  }
// *** Speicher für "VehicleInformation" allokieren
if (!(obd->VehicleInformation = (TOBD2Data *)mhs_malloc0((vehicle_inf_size * sizeof(TOBD2Data)))))
  {
  Obd2Destroy(&obd);
  return(NULL);
  }
// *** "VehicleInformation" mit OBD Datenbank verknüpfen
obd->VehicleInformationSize = vehicle_inf_size;
for (cmd = OBD2Db; cmd->Name; cmd++)
  {
  if (OBD2CmdGetMode(cmd) == 9)
    {
    pid = OBD2CmdGetPID(cmd);
    obd_data = &obd->VehicleInformation[pid];
    obd_data->Enabled = 1;
    obd_data->Cmd = cmd;
    if (cmd->Flags & 0x10)
      obd_data->Enabled = 1;
    if (cmd->Flags & 0x20)
      obd_data->EnableLock = 1;
    }
  }
// *** DTC - Dignostic Trouble Codes
for (cmd = OBD2Db; cmd->Name; cmd++)
  {
  if (OBD2CmdGetMode(cmd) == 3)
    {
    obd->Dtc.Cmd = cmd;
    obd->Dtc.Enabled = 1;
    break;
    }
  }

if (!(obd->Thread = mhs_create_thread(Obd2ThreadExecute, obd, MHS_THREAD_PRIORITY_NORMAL, 0)))
  Obd2Destroy(&obd);
else
  mhs_event_set_event_mask((TMhsEvent *)obd->Thread, MHS_ALL_EVENTS);
return(obd);
}


void Obd2Destroy(TObd2 **obd)
{
TObd2 *o;
int idx;

if (!obd)
  return;
if (!(o = *obd))
  return;
if (o->CurrentData)
  {
  if (o->Thread)
    {
    (void)mhs_join_thread(o->Thread, 1000);
    mhs_destroy_thread(&o->Thread, 0);
    }
  for (idx =0; idx < o->CurrentDataSize; idx++)
    Obd2DataFree(&o->CurrentData[idx]);
  mhs_free(o->CurrentData);
  }
if (o->VehicleInformation)
  {
  for (idx =0; idx < o->VehicleInformationSize; idx++)
    Obd2DataFree(&o->VehicleInformation[idx]);
  mhs_free(o->VehicleInformation);
  }
mhs_free(o);
*obd = NULL;
}


void Obd2Start(TObd2 *obd)
{
if (!obd)
  return;
if (!obd->Thread)
  return;
if (!obd->Thread->Run)
  mhs_run_thread(obd->Thread);
obd->AktivIndex = 0;
obd->Status = OBD2_INIT;
mhs_event_set((TMhsEvent *)obd->Thread, OBD2_START_CMD);
}


void Obd2Stop(TObd2 *obd)
{
TOBD2Data *obd_data;
int idx;

if (!obd)
  return;
if (obd->Thread)
  (void)mhs_join_thread(obd->Thread, 1000);
// *** CurrentData
for (idx = 0; idx < obd->CurrentDataSize; idx++)
  {
  obd_data = &obd->CurrentData[idx];
  obd_data->Enabled = 0x00;
  obd_data->Status = 0x00;
  obd_data->Update = 0x00;
  }
obd->AktivIndex = 0;
// *** Vehicle Information
for (idx =0; idx < obd->VehicleInformationSize; idx++)
  Obd2DataFree(&obd->VehicleInformation[idx]);
// *** DTC - Dignostic Trouble Codes
Obd2DataFree(&obd->Dtc);

obd->Status = OBD2_INIT;
}


void Obd2ReadDtc(TObd2 *obd)
{
if (!obd)
  return;
mhs_event_set((TMhsEvent *)obd->Thread, OBD2_DTC_READ_CMD);
}


int32_t Obd2DataGetAndClearStatus(TOBD2Data *obd_data)
{
if (obd_data)
  return(OBD2_ERR_PID_NOT_FOUND);
if (!obd_data->Supported)
  return(OBD2_ERR_PID_NOT_SUPPORTED);
else if (!obd_data->Enabled)
  return(OBD2_ERR_PID_DISABLED);
else if (!obd_data->Status)
  return(OBD2_ERR_VALUE_INVALID);
else
  {
  if (obd_data->Update & 0x01)
    {
    obd_data->Update &= 0xFE;
    return(OBD2_VALUE_UPDATE);
    }
  else
    return(OBD2_VALUE_OK);
  }
}


TOBD2Data *Obd2GetByPid(TObd2 *obd, uint8_t mode, uint8_t pid, int32_t *status)
{
int32_t s, idx, obd_data_size;
TOBD2Data *obd_data, *obd_data_item;

if (!obd)
  return(NULL);
switch (mode)
  {
  case 1 : {
           obd_data_size = obd->CurrentDataSize;
           obd_data = obd->CurrentData;
           break;
           }
  case 3 : {
           obd_data_size = 1;
           obd_data = &obd->Dtc;
           break;
           }
  case 9 : {
           obd_data_size = obd->VehicleInformationSize;
           obd_data = obd->VehicleInformation;
           break;
           }
  default : return(NULL);
  }
for (idx = 0; idx < obd_data_size; idx++)
  {
  obd_data_item = &obd_data[idx];
  if (!obd_data_item->Cmd)
    continue;
  if (pid == OBD2CmdGetPID(obd_data_item->Cmd))
    {
    if (!obd_data_item->Supported)
      s = OBD2_ERR_PID_NOT_SUPPORTED;
    else if (!obd_data_item->Enabled)
      s = OBD2_ERR_PID_DISABLED;
    else if (!obd_data_item->Status)
      s = OBD2_ERR_VALUE_INVALID;
    else
      {
      if (obd_data_item->Update & 0x80)
        {
        obd_data_item->Update &= 0x7F;
        s = OBD2_VALUE_UPDATE;
        }
      else
        s = OBD2_VALUE_OK;
      }
    if (status)
      *status = s;
    return(obd_data_item);
    }
  }
if (status)
  *status = OBD2_ERR_PID_NOT_FOUND;
return(NULL);
}


uint32_t Obd2ValueGetAsU32(TObd2 *obd,  uint8_t mode, uint8_t pid, int32_t *status)
{
TOBD2Data *obd_data;

if ((obd_data = Obd2GetByPid(obd, mode, pid, status)))
  return(obd_data->Value.U32);
else
  return(0);
}


double Obd2ValueGetAsReal(TObd2 *obd, uint8_t mode, uint8_t pid, int32_t *status)
{
TOBD2Data *obd_data;

if ((obd_data = Obd2GetByPid(obd, mode, pid, status)))
  return(obd_data->ObdValue1);
else
  return(0);
}


char *Obd2ValueGetAsString(TObd2 *obd, uint8_t mode, uint8_t pid, int32_t *status)
{
char *str;
TOBD2Data *obd_data;

str = NULL;
if ((obd_data = Obd2GetByPid(obd, mode, pid, status)))
  {
  if (obd_data->Cmd->ResponseType == OBD2TypeString)
    {
    mhs_enter_critical(obd->Thread);
    str = mhs_strdup(obd_data->ObdString);
    mhs_leave_critical(obd->Thread);
    }
  }
return(str);
}


uint16_t *Obd2ValueGetDTCs(TObd2 *obd, int32_t *status, uint32_t *size)
{
uint16_t *dtcs;
uint32_t count;
size_t mem_size;
TOBD2Data *obd_data;

if (!size)
  return(NULL);
dtcs = NULL;
count = 0;
if ((obd_data = Obd2GetByPid(obd, 3, 0, status)))
  {
  if (obd_data->Cmd->ResponseType == OBD2TypeDTC)
    {
    if ((obd_data->ObdDTCs.DtcNo) && ((count = obd_data->ObdDTCs.Size)))
      {
      mem_size = count * sizeof(uint16_t);
      dtcs = (uint16_t *)malloc(mem_size);
      mhs_enter_critical(obd->Thread);
      memcpy(dtcs, obd_data->ObdDTCs.DtcNo, mem_size);  // <*> g austauschen
      mhs_leave_critical(obd->Thread);
      }
    }
  }
*size = count;
return(dtcs);
}