/*****************************************************************************************************************/
/*                    C - File für ptop                              */
/*---------------------------------------------------------------------------------------------------------------*/
/*            Taskname: server + client                                */
/*            Dateiname: config.c                                  */
/*            Autor:  B&R                                        */
/*            Erstelldatum: April 2005                                 */
/*            Classtime: 10 - 30000 ms                                 */
/*---------------------------------------------------------------------------------------------------------------*/
/*  Funktion:                                                  */
/*          - Auslesen von Konfigurationsinformationen aus Datenmodul "ptopconf"               */
/*                                                         */
/*****************************************************************************************************************/

#ifdef _DEFAULT_INCLUDES
#include <AsDefault.h>
#endif

#include <astime.h>
#include <bur/plc.h>
#include <bur/plctypes.h>
#include <sys_lib.h>
#include <dataobj.h>
#include "allgserv.h"


/**************************************************************************************************************
* GetConfData: Ermittlung der Konfigurationsstruktur
**************************************************************************************************************/

BOOL GetConfData(
void* pPtopConf,    /* Übergabepointer für Daten */
UINT uiLen,       /* Länge der Übergabestruktur*/
UINT uiId)
{
DatObjInfo_typ DatObjInfoFub;
UINT* puiID = NULL;
void* pStartPtr = NULL; 
  
  /* --- Datenmodul vorhanden */
  DatObjInfoFub.enable = TRUE;
  
  if (uiId == 1)
  {
    DatObjInfoFub.pName = (UDINT) CONF_DAT_OBJ_SERV; 
  }
  else if (uiId == 2)
  {
    DatObjInfoFub.pName = (UDINT) CONF_DAT_OBJ_CLIENT; 
  }
  else
  {
    return FALSE;
  }
  
  do
  {
    DatObjInfo(&DatObjInfoFub);
  } while (DatObjInfoFub.status == 0xFFFF);
  
  if (DatObjInfoFub.status != 0)
  {
    return FALSE; 
  }
  
  if (DatObjInfoFub.len < uiLen)
  {
    return FALSE;
  }
  
  pStartPtr = (void*) DatObjInfoFub.pDatObjMem;
  puiID = (UINT*) pStartPtr;
  
  if (*puiID == uiId)
  {
/*    pPtopConf = (void*) (pStartPtr + sizeof(UINT));*/
    brsmemcpy((UDINT) pPtopConf, (UDINT) pStartPtr + sizeof(UINT), uiLen);
    return TRUE;
  }
    
  return FALSE;
}
