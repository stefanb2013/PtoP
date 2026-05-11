/*************************************************************************
*
*	Modul:		    ptoplib.c
*	Beschreibung:	Library - C File für ptop
*	Version:		1.00
*	Taskzyklus:		-
*
**************************************************************************
*	History:
*   Name        Datum     Version	Änderung
*   --------------------------------------------------------------------
*	B&R		25.07.05			Datei angelegt
*
*************************************************************************/

#ifdef _DEFAULT_INCLUDES
#include <AsDefault.h>
#endif

#include "ptoplibinc.h"
#include "stdio.h"
#include "../client/allgclnt.h"
#include "../client/ptpcl.h"

static PvAdmin_typ PvAdmin;

/************************************************************************
*   Initialisierungsfunktion
*************************************************************************/

void PtopInit(PtopInit_typ* inst)
{
  UDINT udLen;
  UINT  uiState;
  UDINT *pudAdr = NULL;
  USINT szHelp[80];

  if (inst == NULL)
  {
    return;
  }

  if (inst->enable == 0)
  {
    return;
  }

  /* --- Lifecheck ----------------------------------------------------- */
  brsmemset((UDINT) &szHelp, 0, sizeof(szHelp));
  brsstrcpy((UDINT) &szHelp, (UDINT) "client:Lebensueberwachung");
  uiState = PV_xgetadr((char*) &szHelp, (UDINT*) &PvAdmin.udLifeCheckAdr,(UDINT*) &PvAdmin.udLifeCheckLen);
  if (uiState != 0)
  {
    inst->status = STATE_PV_LIFE_NOT_FOUND;
    return;
  }

  /* --- Anzahl der parametrierten Stationen --------------------------- */
  PvAdmin.uiMaxServer = PvAdmin.udLifeCheckLen;

  /* --- Neuanmeldung von Clients -------------------------------------- */
  brsmemset((UDINT) &szHelp, 0, sizeof(szHelp));
  brsstrcpy((UDINT) &szHelp, (UDINT) "client:NeuAnmeldungHand");
  uiState = PV_xgetadr((char*) &szHelp, (UDINT*) &PvAdmin.udRestartClientAdr, (UDINT*) &PvAdmin.udRestartClientLen);
  if (uiState != 0)
  {
    inst->status = STATE_PV_RESTART_NOT_FOUND;
    return;
  }

  /* --- Startadresse der Variablenverwaltung -------------------------- */
  brsmemset((UDINT)&szHelp, 0, sizeof(szHelp));
  brsstrcpy((UDINT)&szHelp, (UDINT) "client:pVariablenVerwaltungStart");
  uiState = PV_xgetadr((char*) &szHelp, (UDINT*) &pudAdr, (UDINT*) &udLen);
  if (uiState != 0)
  {
    inst->status = STATE_PV_PVADR_NOT_FOUND;
    return;
  }
  PvAdmin.udPvAdr = *pudAdr;

  /* --- Startadresse der Variablenverwaltung -------------------------- */
  brsmemset((UDINT) &szHelp, 0, sizeof(szHelp));
  brsstrcpy((UDINT) &szHelp, (UDINT) "client:pClientVerwaltungStart");
  uiState = PV_xgetadr((char*) &szHelp, (UDINT*) &pudAdr, (UDINT*) &udLen);
  if (uiState != 0)
  {
    inst->status = STATE_PV_CLIENTADR_NOT_FOUND;
    return;
  }
  PvAdmin.udClientAdr = *pudAdr;

}


/************************************************************************
*   Kommunikation zu Servers x neustarten
*************************************************************************/

void PtopRebuildConn(PtopRebuildConn_typ* inst)
{
  BOOL *pbRestart = NULL;

  if (inst == NULL)
  {
    return;
  }

  if (inst->enable == FALSE)
  {
    return;
  }

  if (inst->ServerId > PvAdmin.uiMaxServer)
  {
    return;
  }

  if ((void*) PvAdmin.udRestartClientAdr == NULL)
  {
    return;
  }

  pbRestart = (BOOL*) (PvAdmin.udRestartClientAdr) + inst->ServerId;

  /* --- Neuanmeldung für Client x durchführen -------------------- */
  *pbRestart = 1;

}


/************************************************************************
*   Kommunikation zu allen Servern neustarten
*************************************************************************/

void PtopAllRebuildConn(PtopAllRebuildConn_typ* inst)
{
  Client_List_Typ* pClientList;
  UINT i;
  BOOL *pbRestart = NULL;

  if (inst == NULL)
  {
    return;
  }

  if (inst->enable == FALSE)
  {
    return;
  }

  if ((void*)PvAdmin.udRestartClientAdr == NULL)
  {
    return;
  }

  /* --- Serverliste durchsuchen und alle parametrierten PVs auf TRUE setzen */
  for (i = 0, pClientList = (Client_List_Typ*) PvAdmin.udClientAdr; i < PvAdmin.uiMaxServer; i++, pClientList++)
  {
    if (pClientList->IPAdresse > 0)
    {
      pbRestart = (BOOL*) (PvAdmin.udRestartClientAdr) + i;
      *pbRestart = 1;
    }
  }

}


/************************************************************************
*   Lifecheck von Server x ermitteln
*************************************************************************/

void PtopLifeCheckServer(PtopLifeCheckServer_typ* inst)
{
  USINT* pLifeCheck = NULL;

  if (inst == NULL)
  {
    return;
  }

  if (inst->ServerId > MAX_SERVER)
  {
    return;
  }

  if ((void*) PvAdmin.udLifeCheckAdr == NULL)
  {
    return;
  }

  pLifeCheck = (USINT*)PvAdmin.udLifeCheckAdr;
  pLifeCheck += inst->ServerId;

  inst->Lebensuebeberwachung = *pLifeCheck;

}


/************************************************************************
*   Status von allen Servern
*************************************************************************/

void PtopLifeAllServer(PtopLifeAllServer_typ* inst)
{
  UINT uiLen = 0;

  if (inst == NULL)
  {
    return;
  }

  if (((void*) inst->Lebensueberwachung == NULL) || (inst->Feldgroesse == 0))
  {
    return;
  }

  if ((void*) PvAdmin.udLifeCheckAdr == NULL)
  {
    return;
  }

  uiLen = inst->Feldgroesse;
  if (inst->Feldgroesse > PvAdmin.uiMaxServer)
  {
    uiLen = PvAdmin.uiMaxServer;
  }

  brsmemcpy((UDINT) &inst->Lebensueberwachung, (UDINT) &PvAdmin.udLifeCheckAdr, uiLen);

}


/************************************************************************
*   Status von Servers x ermitteln
*************************************************************************/

void PtopGetStateOfServer(PtopGetStateOfServer_typ* inst)
{
  Client_List_Typ* pClientList;
  PvList_Typ* pPvList;
  UINT i;

  if (inst == NULL)
  {
    return;
  }

  if (((void*) PvAdmin.udPvAdr == NULL) || ((void*) PvAdmin.udClientAdr == NULL))
  {
    return;
  }

  if (inst->ServerId > MAX_SERVER)
  {
    return;
  }

  pClientList =  (Client_List_Typ*)PvAdmin.udClientAdr + inst->ServerId;
  inst->ServerStatus = pClientList->LifeCheck;

  inst->PvStatus = STATE_PV_OK;

  for (i = 0, pPvList = (PvList_Typ*) PvAdmin.udPvAdr; i < MAX_VARIABLEN; i++, pPvList++)
  {
    if (pPvList->Client == inst->ServerId)
    {
      if ((void*) pPvList->pPv == NULL)
      {
        inst->PvStatus = STATE_PV_NOT_FOUND;
        return;
      }

      if (pPvList->Status != STATE_PV_OK)
      {
        inst->PvStatus = pPvList->Status;
        return;
      }
    }
  }



}
