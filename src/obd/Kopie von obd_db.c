/*******************************************************************************
                           obd_db.c  -  description
                             -------------------
    begin             : 19.01.2019
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
#include "obd_db.h"
#include "obd_decode.c"
                                                                                                                                       // Scale 0 disables Scale/Offset function
const TOBD2Cmd OBD2Db[] = {                                                                                                      // Length | Scale | Offset
//  Name                                                              |    Unit       | Formatm String     | Mode | PID | Response Type    |
  {"Supported PIDs in the range 01 - 20",                             NULL,           "%08X",              {0x01, 0x00}, OBD2TypeBitfield, 6, 1.0f,            0.0f, &OBD2DecU32Raw},
  {"Monitor status since DTCs cleared",                               NULL,           "%08X",              {0x01, 0x01}, OBD2TypeBitfield, 6, 1.0f,            0.0f, &OBD2DecU32Raw},
  {"Freeze DTC",                                                      NULL,           "%04X",              {0x01, 0x02}, OBD2TypeOther,    4, 1.0f,            0.0f, &OBD2DecNop}, // <*> Decoder ?
  {"Fuel system status",                                              NULL,           "%04X",              {0x01, 0x03}, OBD2TypeBitfield, 4, 1.0f,            0.0f, &OBD2DecU16Raw},
  {"Calculated engine load",                                          "%",            "%5.1f",             {0x01, 0x04}, OBD2TypeNumeric,  3, 0.392156863f,    0.0f, &OBD2DecU8},
  {"Engine coolant temperature",                                      "°C",           "% 3f",              {0x01, 0x05}, OBD2TypeNumeric,  3, 1.0f,          -40.0f, &OBD2DecU8},
  {"Short term fuel trim—Bank 1",                                     "%",            "%+5.1f",            {0x01, 0x06}, OBD2TypeNumeric,  3, 0.78125f,     -100.0f, &OBD2DecU8},
  {"Long term fuel trim—Bank 1",                                      "%",            "%+5.1f",            {0x01, 0x07}, OBD2TypeNumeric,  3, 0.78125f,     -100.0f, &OBD2DecU8},
  {"Short term fuel trim—Bank 2",                                     "%",            "%+5.1f",            {0x01, 0x08}, OBD2TypeNumeric,  3, 0.78125f,     -100.0f, &OBD2DecU8},
  {"Long term fuel trim—Bank 2",                                      "%",            "%+5.1f",            {0x01, 0x09}, OBD2TypeNumeric,  3, 0.78125f,     -100.0f, &OBD2DecU8},
  {"Fuel pressure (gauge pressure)",                                  "kPa",          "%3f",               {0x01, 0x0A}, OBD2TypeNumeric,  3, 3.0f,            0.0f, &OBD2DecU8},
  {"Intake manifold absolute pressure",                               "kPa",          "%3f",               {0x01, 0x0B}, OBD2TypeNumeric,  3, 1.0f,            0.0f, &OBD2DecU8},
  {"Engine RPM",                                                      "RPM",          "%5f",               {0x01, 0x0C}, OBD2TypeNumeric,  4, 0.25f,           0.0f, &OBD2DecU16},
  {"Vehicle speed",                                                   "km/h",         "%3f",               {0x01, 0x0D}, OBD2TypeNumeric,  3, 1.0f,            0.0f, &OBD2DecU8},
  {"Timing advance",                                                  "° before TDC", "% 4.2f",            {0x01, 0x0E}, OBD2TypeNumeric,  3, 0.5f,          -64.0f, &OBD2DecU8},
  {"Intake air temperature",                                          "°C",           "% 3f",              {0x01, 0x0F}, OBD2TypeNumeric,  3, 1.0f,          -40.0f, &OBD2DecU8},
  {"MAF air flow rate",                                               "grams/sec",    "%6.2f",             {0x01, 0x10}, OBD2TypeNumeric,  4, 0.01f,           0.0f, &OBD2DecU16},
  {"Throttle position",                                               "%",            "%3f",               {0x01, 0x11}, OBD2TypeNumeric,  3, 0.392156863f,    0.0f, &OBD2DecU8},
  {"Commanded secondary air status",                                  NULL,           "%02X",              {0x01, 0x12}, OBD2TypeBitfield, 3, 1.0f,            0.0f, &OBD2DecU8Raw},
  {"Oxygen sensors present",                                          NULL,           "%02X",              {0x01, 0x13}, OBD2TypeBitfield, 3, 1.0f,            0.0f, &OBD2DecU8Raw},
  {"Oxygen sensor 1",                                                 NULL,           "%4.2fV %3f%%",      {0x01, 0x14}, OBD2TypeOther,    4, 0.0f,            0.0f, &OBD2DecOxSensorValues},
  {"Oxygen sensor 2",                                                 NULL,           "%4.2fV %3f%%",      {0x01, 0x15}, OBD2TypeOther,    4, 0.0f,            0.0f, &OBD2DecOxSensorValues},
  {"Oxygen sensor 3",                                                 NULL,           "%4.2fV %3f%%",      {0x01, 0x16}, OBD2TypeOther,    4, 0.0f,            0.0f, &OBD2DecOxSensorValues},
  {"Oxygen sensor 4",                                                 NULL,           "%4.2fV %3f%%",      {0x01, 0x17}, OBD2TypeOther,    4, 0.0f,            0.0f, &OBD2DecOxSensorValues},
  {"Oxygen sensor 5",                                                 NULL,           "%4.2fV %3f%%",      {0x01, 0x18}, OBD2TypeOther,    4, 0.0f,            0.0f, &OBD2DecOxSensorValues},
  {"Oxygen sensor 6",                                                 NULL,           "%4.2fV %3f%%",      {0x01, 0x19}, OBD2TypeOther,    4, 0.0f,            0.0f, &OBD2DecOxSensorValues},
  {"Oxygen sensor 7",                                                 NULL,           "%4.2fV %3f%%",      {0x01, 0x1A}, OBD2TypeOther,    4, 0.0f,            0.0f, &OBD2DecOxSensorValues},
  {"Oxygen sensor 8",                                                 NULL,           "%4.2fV %3f%%",      {0x01, 0x1B}, OBD2TypeOther,    4, 0.0f,            0.0f, &OBD2DecOxSensorValues},
  {"OBD standards this vehicle conforms to",                          NULL,           "%02X",              {0x01, 0x1C}, OBD2TypeBitfield, 3, 1.0f,            0.0f, &OBD2DecU8},
  {"Oxygen sensors present in 4 banks",                               NULL,           "%02X",              {0x01, 0x1D}, OBD2TypeBitfield, 3, 1.0f,            0.0f, &OBD2DecU8},
  {"Auxiliary input status",                                          NULL,           "%02X",              {0x01, 0x1E}, OBD2TypeBitfield, 3, 1.0f,            0.0f, &OBD2DecU8},
  {"Run time since engine start",                                     "Sek.",         "%5f",               {0x01, 0x1F}, OBD2TypeNumeric,  4, 1.0f,            0.0f, &OBD2DecU16},
  {"Supported PIDs in the range 21 - 40",                             NULL,           "%08X",              {0x01, 0x20}, OBD2TypeBitfield, 6, 1.0f,            0.0f, &OBD2DecU32Raw},
  {"Distance traveled with malfunction indicator lamp on",            "km",           "%5f",               {0x01, 0x21}, OBD2TypeNumeric,  4, 1.0f,            0.0f, &OBD2DecU16},
  {"Fuel rail pressure (relative to mainfold vacuum",                 "kPa",          "%5.2f",             {0x01, 0x22}, OBD2TypeNumeric,  4, 0.079f,          0.0f, &OBD2DecU16},
  {"Fuel rail gauge pressure (diesel, or gasoline direct injection)", "kPa",          "%5f",               {0x01, 0x23}, OBD2TypeNumeric,  4, 10.0f,           0.0f, &OBD2DecU16},
  {"Oxygen sensor 1",                                                 NULL,           "%5.2fratio %5.2fV", {0x01, 0x24}, OBD2TypeOther,    6, 0.0f,            0.0f, &OBD2DecOxSensorValues2},
  {"Oxygen sensor 2",                                                 NULL,           "%5.2fratio %5.2fV", {0x01, 0x25}, OBD2TypeOther,    6, 0.0f,            0.0f, &OBD2DecOxSensorValues2},
  {"Oxygen sensor 3",                                                 NULL,           "%5.2fratio %5.2fV", {0x01, 0x26}, OBD2TypeOther,    6, 0.0f,            0.0f, &OBD2DecOxSensorValues2},
  {"Oxygen sensor 4",                                                 NULL,           "%5.2fratio %5.2fV", {0x01, 0x27}, OBD2TypeOther,    6, 0.0f,            0.0f, &OBD2DecOxSensorValues2},
  {"Oxygen sensor 5",                                                 NULL,           "%5.2fratio %5.2fV", {0x01, 0x28}, OBD2TypeOther,    6, 0.0f,            0.0f, &OBD2DecOxSensorValues2},
  {"Oxygen sensor 6",                                                 NULL,           "%5.2fratio %5.2fV", {0x01, 0x29}, OBD2TypeOther,    6, 0.0f,            0.0f, &OBD2DecOxSensorValues2},
  {"Oxygen sensor 7",                                                 NULL,           "%5.2fratio %5.2fV", {0x01, 0x2A}, OBD2TypeOther,    6, 0.0f,            0.0f, &OBD2DecOxSensorValues2},
  {"Oxygen sensor 8",                                                 NULL,           "%5.2fratio %5.2fV", {0x01, 0x2B}, OBD2TypeOther,    6, 0.0f,            0.0f, &OBD2DecOxSensorValues2},
  {"Commanded EGR",                                                   "%",            "%3f",               {0x01, 0x2C}, OBD2TypeNumeric,  3, 0.392156863f,    0.0f, &OBD2DecU8},
  {"EGR error",                                                       "%",            "%+3f",              {0x01, 0x2D}, OBD2TypeNumeric,  3, 0.78125f,     -100.0f, &OBD2DecU8},
  {"Commanded evaporative purge",                                     "%",            "%3f",               {0x01, 0x2E}, OBD2TypeNumeric,  3, 0.392156863f,    0.0f, &OBD2DecU8},
  {"Fuel tank level input",                                           "%",            "%3f",               {0x01, 0x2F}, OBD2TypeNumeric,  3, 0.392156863f,    0.0f, &OBD2DecU8},
  {"Warm-ups since codes cleared",                                    "Count",        "%3f",               {0x01, 0x30}, OBD2TypeNumeric,  3, 1.0f,            0.0f, &OBD2DecU8},
  {"Distance traveled since codes cleared",                           "km",           "%5f",               {0x01, 0x31}, OBD2TypeNumeric,  4, 1.0f,            0.0f, &OBD2DecU16},
  {"Evaporative system vapor pressure",                               "Pa",           "%5.1f",             {0x01, 0x32}, OBD2TypeNumeric,  4, 0.25f,           0.0f, &OBD2DecS16},
  {"Absolute barometric pressure",                                    "kPa",          "%3f",               {0x01, 0x33}, OBD2TypeNumeric,  3, 1.0f,            0.0f, &OBD2DecU8},
  {"Oxygen sensor 1",                                                 NULL,           "%5.2fratio %5.2fmA",{0x01, 0x34}, OBD2TypeOther,    6, 0.0f,            0.0f, &OBD2DecOxSensorValues3},
  {"Oxygen sensor 2",                                                 NULL,           "%5.2fratio %5.2fmA",{0x01, 0x35}, OBD2TypeOther,    6, 0.0f,            0.0f, &OBD2DecOxSensorValues3},
  {"Oxygen sensor 3",                                                 NULL,           "%5.2fratio %5.2fmA",{0x01, 0x36}, OBD2TypeOther,    6, 0.0f,            0.0f, &OBD2DecOxSensorValues3},
  {"Oxygen sensor 4",                                                 NULL,           "%5.2fratio %5.2fmA",{0x01, 0x37}, OBD2TypeOther,    6, 0.0f,            0.0f, &OBD2DecOxSensorValues3},
  {"Oxygen sensor 5",                                                 NULL,           "%5.2fratio %5.2fmA",{0x01, 0x38}, OBD2TypeOther,    6, 0.0f,            0.0f, &OBD2DecOxSensorValues3},
  {"Oxygen sensor 6",                                                 NULL,           "%5.2fratio %5.2fmA",{0x01, 0x39}, OBD2TypeOther,    6, 0.0f,            0.0f, &OBD2DecOxSensorValues3},
  {"Oxygen sensor 7",                                                 NULL,           "%5.2fratio %5.2fmA",{0x01, 0x3A}, OBD2TypeOther,    6, 0.0f,            0.0f, &OBD2DecOxSensorValues3},
  {"Oxygen sensor 8",                                                 NULL,           "%5.2fratio %5.2fmA",{0x01, 0x3B}, OBD2TypeOther,    6, 0.0f,            0.0f, &OBD2DecOxSensorValues3},
  {"Catalyst temperature, bank 1, sensor 1",                          "°C",           "%6.1f",             {0x01, 0x3C}, OBD2TypeNumeric,  4, 0.1f,          -40.0f, &OBD2DecU16},
  {"Catalyst temperature, bank 2, sensor 1",                          "°C",           "%6.1f",             {0x01, 0x3D}, OBD2TypeNumeric,  4, 0.1f,          -40.0f, &OBD2DecU16},
  {"Catalyst temperature, bank 1, sensor 2",                          "°C",           "%6.1f",             {0x01, 0x3E}, OBD2TypeNumeric,  4, 0.1f,          -40.0f, &OBD2DecU16},
  {"Catalyst temperature, bank 2, sensor 2",                          "°C",           "%6.1f",             {0x01, 0x3F}, OBD2TypeNumeric,  4, 0.1f,          -40.0f, &OBD2DecU16},
  {"Supported PIDs in the range 41 - 60",                             NULL,           "%08X",              {0x01, 0x40}, OBD2TypeBitfield, 6, 1.0f,            0.0f, &OBD2DecU32Raw},
  {"Monitor status this drive cycle",                                 NULL,           "%08X",              {0x01, 0x41}, OBD2TypeBitfield, 6, 1.0f,            0.0f, &OBD2DecU32Raw},
  {"Control module voltage",                                          "V",            "%6.3f",             {0x01, 0x42}, OBD2TypeNumeric,  4, 0.001f,          0.0f, &OBD2DecU16},
  {"Absolute load value",                                             "%",            "%5f",               {0x01, 0x43}, OBD2TypeNumeric,  4, 0.392156863f, 0.0f, &OBD2DecU16},
  {"Fuel–Air commanded equivalence ratio",                            "ratio",        "%5.3f",             {0x01, 0x44}, OBD2TypeNumeric,  4, 0.000030518f, 0.0f, &OBD2DecU16},
  {"Relative throttle position",                                      "%",            "%3f",               {0x01, 0x45}, OBD2TypeNumeric,  3, 0.392156863f, 0.0f, &OBD2DecU8},
  {"Ambient air temperature",                                         "°C",           "% 3f",              {0x01, 0x46}, OBD2TypeNumeric,  3, 1.0f,       -40.0f, &OBD2DecU8},
  {"Absolute throttle position B",                                    "%",            "%3f",               {0x01, 0x47}, OBD2TypeNumeric,  3, 0.392156863f, 0.0f, &OBD2DecU8},
  {"Absolute throttle position C",                                    "%",            "%3f",               {0x01, 0x48}, OBD2TypeNumeric,  3, 0.392156863f, 0.0f, &OBD2DecU8},
  {"Accelerator pedal position D",                                    "%",            "%3f",               {0x01, 0x49}, OBD2TypeNumeric,  3, 0.392156863f, 0.0f, &OBD2DecU8},
  {"Accelerator pedal position E",                                    "%",            "%3f",               {0x01, 0x4A}, OBD2TypeNumeric,  3, 0.392156863f, 0.0f, &OBD2DecU8},
  {"Accelerator pedal position F",                                    "%",            "%3f",               {0x01, 0x4B}, OBD2TypeNumeric,  3, 0.392156863f, 0.0f, &OBD2DecU8},
  {"Commanded throttle actuator",                                     "%",            "%3f",               {0x01, 0x4C}, OBD2TypeNumeric,  3, 0.392156863f, 0.0f, &OBD2DecU8},
  {"Time run with MIL on",                                            "Min.",         "%5f",               {0x01, 0x4D}, OBD2TypeNumeric,  4, 1.0f,         0.0f, &OBD2DecU16},
  {"Time since trouble codes cleared",                                "Min.",         "%5f",               {0x01, 0x4E}, OBD2TypeNumeric,  4, 1.0f,         0.0f, &OBD2DecU16},
  { NULL,                                                            NULL,NULL,{ 0x00, 0x00 }, 0,                         0, 0, 0, NULL }};
