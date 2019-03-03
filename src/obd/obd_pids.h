#ifndef __OBD_PIDS_H__
#define __OBD_PIDS_H__

#ifdef __cplusplus
  extern "C" {
#endif

// Mode 1 PIDs
#define PID_MONITOR_STATUS_DTCS          0x01
#define PID_FREEZE_DTC                   0x02
#define PID_FUEL_SYSTEM_STATUS           0x03
#define PID_ENGINE_LOAD                  0x04
#define PID_COOLANT_TEMP                 0x05
#define PID_SHORT_TERM_FUEL_TRIM_1       0x06
#define PID_LONG_TERM_FUEL_TRIM_1        0x07
#define PID_SHORT_TERM_FUEL_TRIM_2       0x08
#define PID_LONG_TERM_FUEL_TRIM_2        0x09
#define PID_FUEL_PRESSURE                0x0A
#define PID_INTAKE_MAP                   0x0B
#define PID_RPM                          0x0C
#define PID_SPEED                        0x0D
#define PID_TIMING_ADVANCE               0x0E
#define PID_INTAKE_TEMP                  0x0F
#define PID_MAF_FLOW                     0x10
#define PID_THROTTLE                     0x11
#define PID_AUX_INPUT                    0x1E
#define PID_RUNTIME                      0x1F
#define PID_DISTANCE_WITH_MIL            0x21
#define PID_COMMANDED_EGR                0x2C
#define PID_EGR_ERROR                    0x2D
#define PID_COMMANDED_EVAPORATIVE_PURGE  0x2E
#define PID_FUEL_LEVEL                   0x2F
#define PID_WARMS_UPS                    0x30
#define PID_DISTANCE                     0x31
#define PID_EVAP_SYS_VAPOR_PRESSURE      0x32
#define PID_BAROMETRIC                   0x33
#define PID_CATALYST_TEMP_B1S1           0x3C
#define PID_CATALYST_TEMP_B2S1           0x3D
#define PID_CATALYST_TEMP_B1S2           0x3E
#define PID_CATALYST_TEMP_B2S2           0x3F
#define PID_CONTROL_MODULE_VOLTAGE       0x42
#define PID_ABSOLUTE_ENGINE_LOAD         0x43
#define PID_RELATIVE_THROTTLE_POS        0x45
#define PID_AMBIENT_TEMP                 0x46
#define PID_ABSOLUTE_THROTTLE_POS_B      0x47
#define PID_ABSOLUTE_THROTTLE_POS_C      0x48
#define PID_ACC_PEDAL_POS_D              0x49
#define PID_ACC_PEDAL_POS_E              0x4A
#define PID_ACC_PEDAL_POS_F              0x4B
#define PID_COMMANDED_THROTTLE_ACTUATOR  0x4C
#define PID_TIME_WITH_MIL                0x4D
#define PID_TIME_SINCE_CODES_CLEARED     0x4E
#define PID_ETHANOL_FUEL                 0x52
#define PID_FUEL_RAIL_PRESSURE           0x59
#define PID_HYBRID_BATTERY_PERCENTAGE    0x5B
#define PID_ENGINE_OIL_TEMP              0x5C
#define PID_FUEL_INJECTION_TIMING        0x5D
#define PID_ENGINE_FUEL_RATE             0x5E
#define PID_ENGINE_TORQUE_DEMANDED       0x61
#define PID_ENGINE_TORQUE_PERCENTAGE     0x62
#define PID_ENGINE_REF_TORQUE            0x63

// Mode 9 PIDs
#define PID_VIN_MESSAGE_COUNT            0x01   
#define PID_GET_VIN                      0x02
#define PID_ECU_NAME_MESSAGE_COUNT       0x09
#define PID_GET_ECU_NAME                 0x0A

#ifdef __cplusplus
  }
#endif

#endif