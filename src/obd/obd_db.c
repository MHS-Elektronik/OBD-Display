/*******************************************************************************
                           obd_db.c  -  description
                             -------------------
    begin             : 19.01.2019
    copyright         : (C) 2019 by MHS-Elektronik GmbH & Co. KG, Germany
                               http://www.mhs-elektronik.de
    autho             : Klaus Demlehner, klaus@mhs-elektronik.de
 *******************************************************************************

This file is based on "OBDII.c" from Ethan Vaughan.
The project page: https://github.com/ejvaughan/obdii

*/
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


static const struct TObdShow ShowRaw32Hex = {
  //  Mode1    |   FmtStr1   | Units1 | Mode2        |   FmtStr2   | Units2
  OBD_SHOW_RAW,    "0x%08X",    NULL,  OBD_SHOW_NONE,     NULL,      NULL};

static const struct TObdShow ShowRaw16Hex = {
  //  Mode1    |   FmtStr1   | Units1 | Mode2        |   FmtStr2   | Units2
  OBD_SHOW_RAW,    "0x%04X",    NULL,  OBD_SHOW_NONE,     NULL,      NULL};

static const struct TObdShow ShowRaw8Hex = {
  //  Mode1    |   FmtStr1   | Units1 | Mode2        |   FmtStr2   | Units2
  OBD_SHOW_RAW,    "0x%02X",    NULL,  OBD_SHOW_NONE,     NULL,      NULL};

static const struct TObdShow ShowRaw8 = {
  //  Mode1    |   FmtStr1   | Units1 | Mode2        |   FmtStr2   | Units2
  OBD_SHOW_RAW,    "%3u",       NULL,  OBD_SHOW_NONE,     NULL,      NULL};

static const struct TObdShow ShowPercent3Digits = {
  //  Mode1    |   FmtStr1   | Units1 | Mode2        |   FmtStr2   | Units2
  OBD_SHOW_REAL,   "%3.0f",        "%",  OBD_SHOW_NONE,     NULL,      NULL};

static const struct TObdShow ShowPercent3Digits1 = {
  //  Mode1    |   FmtStr1   | Units1 | Mode2        |   FmtStr2   | Units2
  OBD_SHOW_REAL,   "%5.1f",      "%",  OBD_SHOW_NONE,     NULL,      NULL};

static const struct TObdShow ShowPercentP3Digits1 = {
  //  Mode1    |   FmtStr1   | Units1 | Mode2        |   FmtStr2   | Units2
  OBD_SHOW_REAL,  "%+5.1f",      "%",  OBD_SHOW_NONE,     NULL,      NULL};

static const struct TObdShow ShowTemperature3Digits = {
  //  Mode1    |   FmtStr1   | Units1 | Mode2        |   FmtStr2   | Units2
  OBD_SHOW_REAL,   "% 4.0f",      "°C",  OBD_SHOW_NONE,     NULL,      NULL};

static const struct TObdShow ShowkPa3Digits = {
  //  Mode1    |   FmtStr1   | Units1 | Mode2        |   FmtStr2   | Units2
  OBD_SHOW_REAL,   "%3.0f",      "kPa",  OBD_SHOW_NONE,     NULL,      NULL};

static const struct TObdShow ShowRPM5Digits = {
  //  Mode1    |   FmtStr1   | Units1 | Mode2        |   FmtStr2   | Units2
  OBD_SHOW_REAL,   "%5.0f",      "RPM",   OBD_SHOW_NONE,     NULL,      NULL};

static const struct TObdShow ShowKmh3Digits = {
  //  Mode1    |   FmtStr1   | Units1 | Mode2        |   FmtStr2   | Units2
  OBD_SHOW_REAL,   "%3.0f",      "km/h",  OBD_SHOW_NONE,     NULL,      NULL};

static const struct TObdShow ShowTimingAdvance = {
  //  Mode1    |   FmtStr1   | Units1       | Mode2        |   FmtStr2   | Units2
  OBD_SHOW_REAL,  "% 4.2f",   "° before TDC", OBD_SHOW_NONE,     NULL,      NULL};

static const struct TObdShow ShowKm5Digits = {
  //  Mode1    |   FmtStr1   | Units1 | Mode2        |   FmtStr2   | Units2
  OBD_SHOW_REAL,   "%5.0f",       "km",   OBD_SHOW_NONE,     NULL,      NULL};

static const struct TObdShow ShowGramsSec = {
  //  Mode1    |   FmtStr1   | Units1     | Mode2        |   FmtStr2   | Units2
  OBD_SHOW_REAL,   "%6.2f",   "grams/sec", OBD_SHOW_NONE,     NULL,      NULL};

static const struct TObdShow ShowOxSensorVPercent = {
  //  Mode1    |   FmtStr1   | Units1 | Mode2        |   FmtStr2   | Units2
  OBD_SHOW_REAL,   "%4.2f",     "V",   OBD_SHOW_REAL,     "%3.0f",       "%"};

static const struct TObdShow ShowSek5Digits = {
  //  Mode1    |   FmtStr1   | Units1 | Mode2        |   FmtStr2   | Units2
  OBD_SHOW_REAL,   "%5.0f",      "Sek.",   OBD_SHOW_NONE,     NULL,      NULL};

static const struct TObdShow ShowkPa3Digits2 = {
  //  Mode1    |   FmtStr1   | Units1 | Mode2        |   FmtStr2   | Units2
  OBD_SHOW_REAL,   "%5.2f",    "kPa",  OBD_SHOW_NONE,     NULL,      NULL};

static const struct TObdShow ShowkPa5Digits = {
  //  Mode1    |   FmtStr1   | Units1 | Mode2        |   FmtStr2   | Units2
  OBD_SHOW_REAL,   "%5.0f",      "kPa",  OBD_SHOW_NONE,     NULL,      NULL};

static const struct TObdShow ShowOxSensorRatioV = {
  //  Mode1    |   FmtStr1   | Units1 | Mode2        |   FmtStr2   | Units2
  OBD_SHOW_REAL,   "%5.2f",   "ratio", OBD_SHOW_REAL,    "%5.2f",     "V"};

static const struct TObdShow ShowCount3Digits = {
  //  Mode1    |   FmtStr1   | Units1 | Mode2        |   FmtStr2   | Units2
  OBD_SHOW_REAL,   "%3.0f",     "Count", OBD_SHOW_NONE,     NULL,      NULL};

static const struct TObdShow  ShowPa3Digits1 = {
  //  Mode1    |   FmtStr1   | Units1 | Mode2        |   FmtStr2   | Units2
  OBD_SHOW_REAL,   "%5.1f",     "Pa",  OBD_SHOW_NONE,     NULL,      NULL};

static const struct TObdShow ShowOxSensorRatioMA = {
  //  Mode1    |   FmtStr1   | Units1 | Mode2        |   FmtStr2   | Units2
  OBD_SHOW_REAL,   "%5.2f",   "ratio", OBD_SHOW_REAL,    "%5.2f",     "mA"};

static const struct TObdShow ShowTemperature4Digits1 = {
  //  Mode1    |   FmtStr1   | Units1 | Mode2        |   FmtStr2   | Units2
  OBD_SHOW_REAL,   "%6.1f",     "°C",  OBD_SHOW_NONE,     NULL,      NULL};

static const struct TObdShow ShowVoltage3Digits3 = {
  //  Mode1    |   FmtStr1   | Units1 | Mode2        |   FmtStr2   | Units2
  OBD_SHOW_REAL,   "%6.3f",     "V",   OBD_SHOW_NONE,     NULL,      NULL};

static const struct TObdShow ShowMin5Digits = {
  //  Mode1    |   FmtStr1   | Units1 | Mode2        |   FmtStr2   | Units2
  OBD_SHOW_REAL,   "%5.0f",      "Min.",  OBD_SHOW_NONE,     NULL,      NULL};

static const struct TObdShow ShowRatio1Digits3 = {
  //  Mode1    |   FmtStr1   | Units1 | Mode2        |   FmtStr2   | Units2
  OBD_SHOW_REAL,   "%6.3f",   "ratio",  OBD_SHOW_NONE,     NULL,      NULL};

static const struct TObdShow ShowPercent5Digits = {
  //  Mode1    |   FmtStr1   | Units1 | Mode2        |   FmtStr2   | Units2
  OBD_SHOW_REAL,   "%5.0f",        "%",  OBD_SHOW_NONE,      NULL,      NULL};

/*
Mode Bit
      0 - 3
      4 -> Default Enable
      5 -> Enable Lock
      6 -> Wert ist eine Konstante, nur einmal abfragen, kein Zyklisches lesen
      7 -> Supported PIDs Message
*/

const TOBD2Cmd OBD2Db[] = {                                                                                                      // Scale 0 disables Scale/Offset function
// *************************************** M O D E 1 *******************************************
//  Name                                                              | Format String         | Mode | PID | Response Type    |Flags|Len| Scale | Offset
  {"Supported PIDs in the range 01 - 20",                             &ShowRaw32Hex,           {0x01, 0x00}, OBD2TypeBitfield, 0xB0, 6, 1.0f,            0.0f, &OBD2DecU32Raw},
  {"Monitor status since DTCs cleared",                               &ShowRaw32Hex,           {0x01, 0x01}, OBD2TypeBitfield, 0x30, 6, 1.0f,            0.0f, &OBD2DecU32Raw},
  {"Freeze DTC",                                                      &ShowRaw16Hex,           {0x01, 0x02}, OBD2TypeOther,    0x00, 4, 1.0f,            0.0f, &OBD2DecNop}, // <*> Decoder ?
  {"Fuel system status",                                              &ShowRaw16Hex,           {0x01, 0x03}, OBD2TypeBitfield, 0x00, 4, 1.0f,            0.0f, &OBD2DecU16Raw},
  {"Calculated engine load",                                          &ShowPercent3Digits1,    {0x01, 0x04}, OBD2TypeNumeric,  0x00, 3, 0.392156863f,    0.0f, &OBD2DecU8},
  {"Engine coolant temperature",                                      &ShowTemperature3Digits, {0x01, 0x05}, OBD2TypeNumeric,  0x00, 3, 1.0f,          -40.0f, &OBD2DecU8},
  {"Short term fuel trim—Bank 1",                                     &ShowPercentP3Digits1,   {0x01, 0x06}, OBD2TypeNumeric,  0x00, 3, 0.78125f,     -100.0f, &OBD2DecU8},
  {"Long term fuel trim—Bank 1",                                      &ShowPercentP3Digits1,   {0x01, 0x07}, OBD2TypeNumeric,  0x00, 3, 0.78125f,     -100.0f, &OBD2DecU8},
  {"Short term fuel trim—Bank 2",                                     &ShowPercentP3Digits1,   {0x01, 0x08}, OBD2TypeNumeric,  0x00, 3, 0.78125f,     -100.0f, &OBD2DecU8},
  {"Long term fuel trim—Bank 2",                                      &ShowPercentP3Digits1,   {0x01, 0x09}, OBD2TypeNumeric,  0x00, 3, 0.78125f,     -100.0f, &OBD2DecU8},
  {"Fuel pressure (gauge pressure)",                                  &ShowkPa3Digits,         {0x01, 0x0A}, OBD2TypeNumeric,  0x00, 3, 3.0f,            0.0f, &OBD2DecU8},
  {"Intake manifold absolute pressure",                               &ShowkPa3Digits,         {0x01, 0x0B}, OBD2TypeNumeric,  0x00, 3, 1.0f,            0.0f, &OBD2DecU8},
  {"Engine RPM",                                                      &ShowRPM5Digits,         {0x01, 0x0C}, OBD2TypeNumeric,  0x00, 4, 0.25f,           0.0f, &OBD2DecU16},
  {"Vehicle speed",                                                   &ShowKmh3Digits,         {0x01, 0x0D}, OBD2TypeNumeric,  0x00, 3, 1.0f,            0.0f, &OBD2DecU8},
  {"Timing advance",                                                  &ShowTimingAdvance,      {0x01, 0x0E}, OBD2TypeNumeric,  0x00, 3, 0.5f,          -64.0f, &OBD2DecU8},
  {"Intake air temperature",                                          &ShowTemperature3Digits, {0x01, 0x0F}, OBD2TypeNumeric,  0x00, 3, 1.0f,          -40.0f, &OBD2DecU8},
  {"MAF air flow rate",                                               &ShowGramsSec,           {0x01, 0x10}, OBD2TypeNumeric,  0x00, 4, 0.01f,           0.0f, &OBD2DecU16},
  {"Throttle position",                                               &ShowPercent3Digits,     {0x01, 0x11}, OBD2TypeNumeric,  0x00, 3, 0.392156863f,    0.0f, &OBD2DecU8},
  {"Commanded secondary air status",                                  &ShowRaw8Hex,            {0x01, 0x12}, OBD2TypeBitfield, 0x00, 3, 1.0f,            0.0f, &OBD2DecU8Raw},
  {"Oxygen sensors present",                                          &ShowRaw8Hex,            {0x01, 0x13}, OBD2TypeBitfield, 0x00, 3, 1.0f,            0.0f, &OBD2DecU8Raw},
  {"Oxygen sensor 1",                                                 &ShowOxSensorVPercent,   {0x01, 0x14}, OBD2TypeOther,    0x00, 4, 0.0f,            0.0f, &OBD2DecOxSensorValues},
  {"Oxygen sensor 2",                                                 &ShowOxSensorVPercent,   {0x01, 0x15}, OBD2TypeOther,    0x00, 4, 0.0f,            0.0f, &OBD2DecOxSensorValues},
  {"Oxygen sensor 3",                                                 &ShowOxSensorVPercent,   {0x01, 0x16}, OBD2TypeOther,    0x00, 4, 0.0f,            0.0f, &OBD2DecOxSensorValues},
  {"Oxygen sensor 4",                                                 &ShowOxSensorVPercent,   {0x01, 0x17}, OBD2TypeOther,    0x00, 4, 0.0f,            0.0f, &OBD2DecOxSensorValues},
  {"Oxygen sensor 5",                                                 &ShowOxSensorVPercent,   {0x01, 0x18}, OBD2TypeOther,    0x00, 4, 0.0f,            0.0f, &OBD2DecOxSensorValues},
  {"Oxygen sensor 6",                                                 &ShowOxSensorVPercent,   {0x01, 0x19}, OBD2TypeOther,    0x00, 4, 0.0f,            0.0f, &OBD2DecOxSensorValues},
  {"Oxygen sensor 7",                                                 &ShowOxSensorVPercent,   {0x01, 0x1A}, OBD2TypeOther,    0x00, 4, 0.0f,            0.0f, &OBD2DecOxSensorValues},
  {"Oxygen sensor 8",                                                 &ShowOxSensorVPercent,   {0x01, 0x1B}, OBD2TypeOther,    0x00, 4, 0.0f,            0.0f, &OBD2DecOxSensorValues},
  {"OBD standards this vehicle conforms to",                          &ShowRaw8Hex,            {0x01, 0x1C}, OBD2TypeBitfield, 0x00, 3, 1.0f,            0.0f, &OBD2DecU8},
  {"Oxygen sensors present in 4 banks",                               &ShowRaw8Hex,            {0x01, 0x1D}, OBD2TypeBitfield, 0x00, 3, 1.0f,            0.0f, &OBD2DecU8},
  {"Auxiliary input status",                                          &ShowRaw8Hex,            {0x01, 0x1E}, OBD2TypeBitfield, 0x00, 3, 1.0f,            0.0f, &OBD2DecU8},
  {"Run time since engine start",                                     &ShowSek5Digits,         {0x01, 0x1F}, OBD2TypeNumeric,  0x00, 4, 1.0f,            0.0f, &OBD2DecU16},
  {"Supported PIDs in the range 21 - 40",                             &ShowRaw32Hex,           {0x01, 0x20}, OBD2TypeBitfield, 0xB0, 6, 1.0f,            0.0f, &OBD2DecU32Raw},
  {"Distance traveled with malfunction indicator lamp on",            &ShowKm5Digits,          {0x01, 0x21}, OBD2TypeNumeric,  0x00, 4, 1.0f,            0.0f, &OBD2DecU16},
  {"Fuel rail pressure (relative to mainfold vacuum",                 &ShowkPa3Digits2,        {0x01, 0x22}, OBD2TypeNumeric,  0x00, 4, 0.079f,          0.0f, &OBD2DecU16},
  {"Fuel rail gauge pressure (diesel, or gasoline direct injection)", &ShowkPa5Digits,         {0x01, 0x23}, OBD2TypeNumeric,  0x00, 4, 10.0f,           0.0f, &OBD2DecU16},
  {"Oxygen sensor 1",                                                 &ShowOxSensorRatioV,     {0x01, 0x24}, OBD2TypeOther,    0x00, 6, 0.0f,            0.0f, &OBD2DecOxSensorValues2},
  {"Oxygen sensor 2",                                                 &ShowOxSensorRatioV,     {0x01, 0x25}, OBD2TypeOther,    0x00, 6, 0.0f,            0.0f, &OBD2DecOxSensorValues2},
  {"Oxygen sensor 3",                                                 &ShowOxSensorRatioV,     {0x01, 0x26}, OBD2TypeOther,    0x00, 6, 0.0f,            0.0f, &OBD2DecOxSensorValues2},
  {"Oxygen sensor 4",                                                 &ShowOxSensorRatioV,     {0x01, 0x27}, OBD2TypeOther,    0x00, 6, 0.0f,            0.0f, &OBD2DecOxSensorValues2},
  {"Oxygen sensor 5",                                                 &ShowOxSensorRatioV,     {0x01, 0x28}, OBD2TypeOther,    0x00, 6, 0.0f,            0.0f, &OBD2DecOxSensorValues2},
  {"Oxygen sensor 6",                                                 &ShowOxSensorRatioV,     {0x01, 0x29}, OBD2TypeOther,    0x00, 6, 0.0f,            0.0f, &OBD2DecOxSensorValues2},
  {"Oxygen sensor 7",                                                 &ShowOxSensorRatioV,     {0x01, 0x2A}, OBD2TypeOther,    0x00, 6, 0.0f,            0.0f, &OBD2DecOxSensorValues2},
  {"Oxygen sensor 8",                                                 &ShowOxSensorRatioV,     {0x01, 0x2B}, OBD2TypeOther,    0x00, 6, 0.0f,            0.0f, &OBD2DecOxSensorValues2},
  {"Commanded EGR",                                                   &ShowPercent3Digits,     {0x01, 0x2C}, OBD2TypeNumeric,  0x00, 3, 0.392156863f,    0.0f, &OBD2DecU8},
  {"EGR error",                                                       &ShowPercentP3Digits1,   {0x01, 0x2D}, OBD2TypeNumeric,  0x00, 3, 0.78125f,     -100.0f, &OBD2DecU8},
  {"Commanded evaporative purge",                                     &ShowPercent3Digits,     {0x01, 0x2E}, OBD2TypeNumeric,  0x00, 3, 0.392156863f,    0.0f, &OBD2DecU8},
  {"Fuel tank level input",                                           &ShowPercent3Digits,     {0x01, 0x2F}, OBD2TypeNumeric,  0x00, 3, 0.392156863f,    0.0f, &OBD2DecU8},
  {"Warm-ups since codes cleared",                                    &ShowCount3Digits,       {0x01, 0x30}, OBD2TypeNumeric,  0x00, 3, 1.0f,            0.0f, &OBD2DecU8},
  {"Distance traveled since codes cleared",                           &ShowKm5Digits,          {0x01, 0x31}, OBD2TypeNumeric,  0x00, 4, 1.0f,            0.0f, &OBD2DecU16},
  {"Evaporative system vapor pressure",                               &ShowPa3Digits1,         {0x01, 0x32}, OBD2TypeNumeric,  0x00, 4, 0.25f,           0.0f, &OBD2DecS16},
  {"Absolute barometric pressure",                                    &ShowkPa3Digits,         {0x01, 0x33}, OBD2TypeNumeric,  0x00, 3, 1.0f,            0.0f, &OBD2DecU8},
  {"Oxygen sensor 1",                                                 &ShowOxSensorRatioMA,    {0x01, 0x34}, OBD2TypeOther,    0x00, 6, 0.0f,            0.0f, &OBD2DecOxSensorValues3},
  {"Oxygen sensor 2",                                                 &ShowOxSensorRatioMA,    {0x01, 0x35}, OBD2TypeOther,    0x00, 6, 0.0f,            0.0f, &OBD2DecOxSensorValues3},
  {"Oxygen sensor 3",                                                 &ShowOxSensorRatioMA,    {0x01, 0x36}, OBD2TypeOther,    0x00, 6, 0.0f,            0.0f, &OBD2DecOxSensorValues3},
  {"Oxygen sensor 4",                                                 &ShowOxSensorRatioMA,    {0x01, 0x37}, OBD2TypeOther,    0x00, 6, 0.0f,            0.0f, &OBD2DecOxSensorValues3},
  {"Oxygen sensor 5",                                                 &ShowOxSensorRatioMA,    {0x01, 0x38}, OBD2TypeOther,    0x00, 6, 0.0f,            0.0f, &OBD2DecOxSensorValues3},
  {"Oxygen sensor 6",                                                 &ShowOxSensorRatioMA,    {0x01, 0x39}, OBD2TypeOther,    0x00, 6, 0.0f,            0.0f, &OBD2DecOxSensorValues3},
  {"Oxygen sensor 7",                                                 &ShowOxSensorRatioMA,    {0x01, 0x3A}, OBD2TypeOther,    0x00, 6, 0.0f,            0.0f, &OBD2DecOxSensorValues3},
  {"Oxygen sensor 8",                                                 &ShowOxSensorRatioMA,    {0x01, 0x3B}, OBD2TypeOther,    0x00, 6, 0.0f,            0.0f, &OBD2DecOxSensorValues3},
  {"Catalyst temperature, bank 1, sensor 1",                          &ShowTemperature4Digits1,{0x01, 0x3C}, OBD2TypeNumeric,  0x00, 4, 0.1f,          -40.0f, &OBD2DecU16},
  {"Catalyst temperature, bank 2, sensor 1",                          &ShowTemperature4Digits1,{0x01, 0x3D}, OBD2TypeNumeric,  0x00, 4, 0.1f,          -40.0f, &OBD2DecU16},
  {"Catalyst temperature, bank 1, sensor 2",                          &ShowTemperature4Digits1,{0x01, 0x3E}, OBD2TypeNumeric,  0x00, 4, 0.1f,          -40.0f, &OBD2DecU16},
  {"Catalyst temperature, bank 2, sensor 2",                          &ShowTemperature4Digits1,{0x01, 0x3F}, OBD2TypeNumeric,  0x00, 4, 0.1f,          -40.0f, &OBD2DecU16},
  {"Supported PIDs in the range 41 - 60",                             &ShowRaw32Hex,           {0x01, 0x40}, OBD2TypeBitfield, 0xB0, 6, 1.0f,            0.0f, &OBD2DecU32Raw},
  {"Monitor status this drive cycle",                                 &ShowRaw32Hex,           {0x01, 0x41}, OBD2TypeBitfield, 0x00, 6, 1.0f,            0.0f, &OBD2DecU32Raw},
  {"Control module voltage",                                          &ShowVoltage3Digits3,    {0x01, 0x42}, OBD2TypeNumeric,  0x00, 4, 0.001f,          0.0f, &OBD2DecU16},
  {"Absolute load value",                                             &ShowPercent5Digits,     {0x01, 0x43}, OBD2TypeNumeric,  0x00, 4, 0.392156863f, 0.0f, &OBD2DecU16},
  {"Fuel–Air commanded equivalence ratio",                            &ShowRatio1Digits3,      {0x01, 0x44}, OBD2TypeNumeric,  0x00, 4, 0.000030518f, 0.0f, &OBD2DecU16},
  {"Relative throttle position",                                      &ShowPercent3Digits,     {0x01, 0x45}, OBD2TypeNumeric,  0x00, 3, 0.392156863f, 0.0f, &OBD2DecU8},
  {"Ambient air temperature",                                         &ShowTemperature3Digits, {0x01, 0x46}, OBD2TypeNumeric,  0x00, 3, 1.0f,       -40.0f, &OBD2DecU8},
  {"Absolute throttle position B",                                    &ShowPercent3Digits,     {0x01, 0x47}, OBD2TypeNumeric,  0x00, 3, 0.392156863f, 0.0f, &OBD2DecU8},
  {"Absolute throttle position C",                                    &ShowPercent3Digits,     {0x01, 0x48}, OBD2TypeNumeric,  0x00, 3, 0.392156863f, 0.0f, &OBD2DecU8},
  {"Accelerator pedal position D",                                    &ShowPercent3Digits,     {0x01, 0x49}, OBD2TypeNumeric,  0x00, 3, 0.392156863f, 0.0f, &OBD2DecU8},
  {"Accelerator pedal position E",                                    &ShowPercent3Digits,     {0x01, 0x4A}, OBD2TypeNumeric,  0x00, 3, 0.392156863f, 0.0f, &OBD2DecU8},
  {"Accelerator pedal position F",                                    &ShowPercent3Digits,     {0x01, 0x4B}, OBD2TypeNumeric,  0x00, 3, 0.392156863f, 0.0f, &OBD2DecU8},
  {"Commanded throttle actuator",                                     &ShowPercent3Digits,     {0x01, 0x4C}, OBD2TypeNumeric,  0x00, 3, 0.392156863f, 0.0f, &OBD2DecU8},
  {"Time run with MIL on",                                            &ShowMin5Digits,         {0x01, 0x4D}, OBD2TypeNumeric,  0x00, 4, 1.0f,         0.0f, &OBD2DecU16},
  {"Time since trouble codes cleared",                                &ShowMin5Digits,         {0x01, 0x4E}, OBD2TypeNumeric,  0x00, 4, 1.0f,         0.0f, &OBD2DecU16},
// *************************************** M O D E 3 *******************************************
  {"Get DTCs",                                                        NULL,                    {0x03, 0x00}, OBD2TypeDTC,      0x01, 0, 0.0f,         0.0f, &OBD2DecDTCs},
// *************************************** M O D E 9 *******************************************
  {"Supported PIDs",                                                  &ShowRaw32Hex,           {0x09, 0x00}, OBD2TypeBitfield, 0xB0, 6, 0.0f,         0.0f, &OBD2DecU32Raw},
  {"VIN message count",                                               &ShowRaw8,               {0x09, 0x01}, OBD2TypeNumeric,  0x70, 3, 0.0f,         0.0f, &OBD2DecU8Raw},
  {"Get VIN",                                                         NULL,                    {0x09, 0x02}, OBD2TypeString,   0x70, 0, 0.0f,         0.0f, &OBD2DecString},
  {"ECU name message count",                                          &ShowRaw8,               {0x09, 0x09}, OBD2TypeNumeric,  0x70, 3, 0.0f,         0.0f, &OBD2DecU8Raw},
  {"Get ECU name",                                                    NULL,                    {0x09, 0x0A}, OBD2TypeString,   0x70, 0, 0.0f,         0.0f, &OBD2DecString},
  {NULL,                                                              NULL,                    {0x00, 0x00}, 0,                0x00, 0, 0.0f,         0.0f, NULL}};
