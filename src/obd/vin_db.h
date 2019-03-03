#ifndef __VIN_DB_H__
#define __VIN_DB_H__

#ifdef __cplusplus
  extern "C" {
#endif


const char *VinGetCountry(const char *vin);
const char *VinGetRegion(const char *vin);
char *VinGetVds(const char *vin);
const char *VinGetYear(const char *vin);
const char VinGetAssemblyPlant(const char *vin);
const char *VinGetSerialNo(const char *vin);
const char *VinGetManufacturer(const char *vin);

int VinWmiDbLoad(const char *file_name);
void VinWmiDbFree(void);

#ifdef __cplusplus
  }
#endif

#endif
