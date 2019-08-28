/*******************************************************************************
                          open_xc.c  -  description
                             ------------------
    begin             : 05.08.2019
    last modify       : 08.02.2019
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
#include <stdio.h>
#include <glib.h>
#include <string.h>
#include "util.h"
#include "obd2.h"
#include "sock_lib.h"
#include "open_xc.h"

/*
Vehicle Messages:

steering_wheel_angle         numerical, -600 to +600 degrees
torque_at_transmission       numerical, -500 to 1500 Nm
engine_speed                 numerical, 0 to 16382 RPM
vehicle_speed                numerical, 0 to 655 km/h (this will be positive even if going in reverse as it's not a velocity, although you can
                             use the gear status to figure out direction)
accelerator_pedal_position   percentage
parking_brake_status         boolean, (true == brake engaged)
brake_pedal_status           boolean (True == pedal pressed)
transmission_gear_position   states: first, second, third, fourth, fifth, sixth, seventh, eighth, ninth, tenth, reverse, neutral
gear_lever_position          states: neutral, park, reverse, drive, sport, low, first, second, third, fourth, fifth, sixth, seventh, eighth, ninth, tenth
odometer                     Numerical, km 0 to 16777214.000 km, with about .2m resolution
ignition_status              states: off, accessory, run, start
fuel_level                   percentage
fuel_consumed_since_restart  numerical, 0 - 4294967295.0 L (this goes to 0 every time the vehicle restarts, like a trip meter)
door_status                  Value is State: driver, passenger, rear_left, rear_right.
headlamp_status              boolean, true is on
high_beam_status             boolean, true is on
windshield_wiper_status      boolean, true is on
latitude                     numerical, -89.0 to 89.0 degrees with standard GPS accuracy
longitude                    numerical, -179.0 to 179.0 degrees with standard GPS accuracy

engine_load
engine_coolant_temperature
barometric_pressure
commanded_throttle_position
throttle_position
fuel_level
intake_air_temperature
intake_manifold_pressure
running_time
fuel_pressure
mass_airflow
accelerator_pedal_position
ethanol_fuel_percentage
engine_oil_temperature
engine_torque */

// Sample:
//  {"name": "steering_wheel_angle", "value": 45}



struct TVehicleMessage
  {
  const char *Name;
  guint Interval;
  uint8_t Pid;
  double Multiplier;
  int Prec;
  };

static const struct TVehicleMessage VehicleMessages[] = {
  // Name                       | Interval |                Pid             | Multiplier | Prec
  {"steering_wheel_angle",           0,                      0,                  1.0,        0},
  {"torque_at_transmission",         0,     PID_ENGINE_TORQUE_DEMANDED,          1.0,        0}, // PID ?
  {"engine_speed",                   1,     PID_RPM,                             1.0,        0},
  {"vehicle_speed",                  1,     PID_SPEED,                           1.0,        0},
  {"accelerator_pedal_position",     0,     PID_ACC_PEDAL_POS_D,                 1.0,        0}, // PID ?
  {"parking_brake_status",           0,                      0,                  1.0,        0},
  {"brake_pedal_status",             0,                      0,                  1.0,        0},
  {"transmission_gear_position",     0,                      0,                  1.0,        0},
  {"gear_lever_position",            0,                      0,                  1.0,        0},
  {"odometer",                       0,                      0,                  1.0,        0},
  {"ignition_status",                0,                      0,                  1.0,        0},
  {"fuel_consumed_since_restart",    0,                      0,                  1.0,        0},
  {"door_status",                    0,                      0,                  1.0,        0},
  {"headlamp_status",                0,                      0,                  1.0,        0},
  {"high_beam_status",               0,                      0,                  1.0,        0},
  {"windshield_wiper_status",        0,                      0,                  1.0,        0},
  {"latitude",                       0,                      0,                  1.0,        0},
  {"longitude",                      0,                      0,                  1.0,        0},
  {"engine_load",                    1,     PID_ENGINE_LOAD,                     1.0,        0},
  {"engine_coolant_temperature",     1,     PID_COOLANT_TEMP,                    1.0,        0},
  {"barometric_pressure",            0,     PID_BAROMETRIC,                      1.0,        0},
  {"commanded_throttle_position",    0,     PID_COMMANDED_THROTTLE_ACTUATOR,     1.0,        0}, // PID ?
  {"throttle_position",              0,     PID_THROTTLE,                        1.0,        0}, // PID ?
  {"fuel_level",                     1,     PID_FUEL_LEVEL,                      1.0,        0},
  {"intake_air_temperature",         1,     PID_INTAKE_TEMP,                     1.0,        0},
  {"intake_manifold_pressure",       1,     PID_INTAKE_MAP,                      1.0,        0},
  {"running_time",                   0,     PID_RUNTIME,                         1.0,        0},
  {"fuel_pressure",                  0,     PID_FUEL_PRESSURE,                   1.0,        0},
  {"mass_airflow",                   0,                      0,                  1.0,        0},
  {"ethanol_fuel_percentage",        0,     PID_ETHANOL_FUEL,                    1.0,        0},
  {"engine_oil_temperature",         0,     PID_ENGINE_OIL_TEMP,                 1.0,        0},
  {"engine_torque",                  0,     PID_ENGINE_TORQUE_DEMANDED,          1.0,        0}, // PID ?
  {NULL,                             0,                      0,                  1.0,        0}};


typedef struct _TOpenXcCmd TOpenXcCmd;

struct _TOpenXcCmd
  {
  const char *Command;
  void (*Proc)(TOpenXc *open_xc, TSockLibClient *client, TOpenXcCmd *cmd);
  const char *String;
  };

/*
{"command":"version","unix_time":0,"bypass":false,"bus":0,"enabled":false}
{"command_response": "version", "message": "v6.0-dev(default)", "status": true}

{"command":"device_id","unix_time":0,"bypass":false,"bus":0,"enabled":false}
{"command_response": "device_id", "message":"0012345678", "status": true}

{"command":"platform","unix_time":0,"bypass":false,"bus":0,"enabled":false}
{ "command_response": "platform", "message":"FORDBOARD", "status": true}
*/

static const char JsonCommandStr[] = "command";
static const char JsonCommandResponseStr[] = "command_response";

static void OpenXcDefCmdExec(TOpenXc *open_xc, TSockLibClient *client, TOpenXcCmd *cmd);

static const TOpenXcCmd OpenXcCmdList[] = {
   // Commando     | Exec.-Procedure   | String
   {"version",       OpenXcDefCmdExec,  "v1.0 (Linux/Pi)"},
   {"device_id",     OpenXcDefCmdExec,  "0012345678"},  // <*> ???
   {"platform",      OpenXcDefCmdExec,  "Tiny-CAN & Pi"},
   {NULL, NULL}};


#define skip_space(p) while(*(p) == ' ') (p)++

static int JsonStart(char **str)
{
char *s;

if (!str)
  return(-1);
if (!(s = *str))
  return(-1);
skip_space(s);
if (*s != '{')
  return(-1);   // Fehler
s++;
skip_space(s);
if (*s == '\0')
  return(-1);
*str = s;
return(0);
}


static int JsonExtractNameValue(char **str, char **name, char **value)
{
char *s, *n, *v;
char c;
int quotation_mark, res, mode;

if ((!str) || (!name) || (!value))
  return(-1);
s = *str;
n = NULL;
v = NULL;
quotation_mark = 0;
mode = 0;
res = -1;
while ((c = *s))
  {
  if (quotation_mark)
    {
    if (c == '\"')
      {
      *s = '\0';
      quotation_mark = 0;
      }
    s++;
    }
  else
    {
    switch (c)
      {
      case ':'  : {
                  mode = 2;
                  break;
                  }
      case '\"' : {
                  quotation_mark = 1;
                  if (mode)
                    v = s + 1;
                  else
                    n = s + 1;
                  break;
                  }
      case '}'  : {
                  res = 0;
                  *s = '\0';
                  break;
                  }
      case ','  : {
                  res = 1;
                  *s = '\0';
                  break;
                  }
      case ' '  : {
                  *s = '\0';
                  break;
                  }
      default   : {
                  if (mode == 0)
                    {
                    n = s;
                    mode = 1;
                    }
                  else if (mode == 2)
                    {
                    v = s;
                    mode = 3;
                    }
                  }
      }
    s++;
    }
  if (res > -1)
    break;
  }
*str = s;
if ((n) && (*n == '\0'))
  *name = NULL;
else
  *name = n;
if ((v) && (*v == '\0'))
  *value = NULL;
else
  *value = v;
if (quotation_mark)
  return(-1);
return(res);
}


static void OpenXcDefCmdExec(TOpenXc *open_xc, TSockLibClient *client, TOpenXcCmd *cmd)
{
(void)open_xc;
char buf[200];
int l;

sprintf(buf, "{\"%s\": \"%s\", \"message\": \"%s\", \"status\": true}",
     JsonCommandResponseStr, cmd->Command, cmd->String);
l = strlen(buf);
buf[l] = '\0';
(void)SockLibWrite(client, buf, l);
}


static void OpenXcExecuteCommand(TOpenXc *open_xc, TSockLibClient *client, const char *cmd)
{
TOpenXcCmd *cmd_list;
const char *p;

for (cmd_list = (TOpenXcCmd *)OpenXcCmdList; (p = cmd_list->Command); cmd_list++)
  {
  if (!strcasecmp(p, cmd))
    {
    if (cmd_list->Proc)
      (cmd_list->Proc)(open_xc, client, cmd_list);
    break;
    }
  }
}


static void OpenXcProcessRxData(TOpenXc *open_xc, TSockLibClient *client, char *str, int32_t size)
{
char *name, *value;

if (JsonStart(&str) < 0)
  return;
JsonExtractNameValue(&str, &name, &value);
if (!strcasecmp(name, JsonCommandStr))
  OpenXcExecuteCommand(open_xc, client, value);
}


static void OpenXcVehicleMessages(TOpenXc *open_xc)
{
struct TVehicleMessage *vmsg;
char *msg_str;
const char *n;
int32_t msg_size, size;
double value;

msg_str = open_xc->MsgBuffer;
msg_size = 0;
for (vmsg = (struct TVehicleMessage *)VehicleMessages; (n = vmsg->Name); vmsg++)
  {
  if (vmsg->Interval)
    {
    value = Obd2ValueGetAsReal(open_xc->Obd, 1, vmsg->Pid, NULL);
    value *= vmsg->Multiplier;
    size = sprintf(msg_str, "{\"name\": \"%s\", \"value\": %.*f}", n, vmsg->Prec, value);
    msg_str += size;
    *msg_str++ = '\0';
    msg_size += (size + 1);
    }
  }
SockLibWriteAll(open_xc->SockLib, open_xc->MsgBuffer, msg_size);
}


static int32_t OpenXcNewClientCB(TSockLib *sock_lib, TSockLibClient *client, gpointer user_data)
{
TOpenXc *open_xc;

if (!(open_xc = (TOpenXc *)user_data))
  return(0);
return(0);
}


static int32_t OpenExEventCB(TSockLib *sock_lib, uint32_t event, TSockLibClient *client, char *data, int32_t size, gpointer user_data)
{
TOpenXc *open_xc;

if (!(open_xc = (TOpenXc *)user_data))
  return(0);
// ** SOCK_LIB_EVENT
if ((event & 0xFF00) == SOCK_LIB_EVENT)
  OpenXcVehicleMessages(open_xc);
// ** SOCK_LIB_READ
else if (event == SOCK_LIB_READ)
  OpenXcProcessRxData(open_xc, client, data, size);
return(0);
}


static void OpenXcThreadExecute(TMhsThread *thread)
{
TOpenXc *open_xc;

if (!(open_xc = (TOpenXc *)thread->Data))
  return;
while (thread->Run)
  {
  if (mhs_sleep_ex((TMhsEvent *)thread, 100))
    break;
  SockMakeEvent(open_xc->SockLib, 0x01);
  }
}

/* <*>
if (xc->Thread)
  (void)mhs_join_thread(xc->Thread, 100);
*/

TOpenXc *OpenXcCreate(TObd2 *obd, const char *ip, int port)
{
TOpenXc *xc;
int err;

err = 0;
if (!(xc = (TOpenXc *)g_malloc0(sizeof(TOpenXc))))
  return(NULL);
xc->Obd = obd;
if (!(xc->SockLib = SockLibCreate(ip, port, 0, OpenXcNewClientCB, OpenExEventCB, xc)))
  err = -1;
if (!(xc->Thread = mhs_create_thread(OpenXcThreadExecute, xc, MHS_THREAD_PRIORITY_NORMAL, 0)))
  err = -1;
else
  mhs_event_set_event_mask((TMhsEvent *)obd->Thread, MHS_ALL_EVENTS);
if (!xc->Thread->Run)  // <*> verschieben
  mhs_run_thread(xc->Thread);
if (err)
  OpenXcDestroy(&xc);
return(xc);
}


void OpenXcDestroy(TOpenXc **open_xc)
{
TOpenXc *xc;

if (!open_xc)
  return;
if (!(xc = *open_xc))
  return;
if (xc->Thread)
  {
  (void)mhs_join_thread(xc->Thread, 1000);
  mhs_destroy_thread(&xc->Thread, 0);
  }
SockLibDestroy(&xc->SockLib);
g_free(xc);
}

