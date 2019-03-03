#ifndef __DTC_DB_H__
#define __DTC_DB_H__

#ifdef __cplusplus
  extern "C" {
#endif

typedef struct _TDtcListItem TDtcListItem;

struct _TDtcListItem
  {
  TDtcListItem *Next;
  char DtcNo[6];
  char *Description;
  };

TDtcListItem *DtcDbLoad(const char *file_name);
void DtcDbFree(TDtcListItem **dtc_db);

TDtcListItem *DtcDbGetItem(TDtcListItem *dtc_db, const char *dtc_str);

#ifdef __cplusplus
  }
#endif

#endif
