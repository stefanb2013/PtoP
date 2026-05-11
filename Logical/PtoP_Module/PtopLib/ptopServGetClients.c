/********************************************************************
 * COPYRIGHT -- Bernecker + Rainer
 ********************************************************************
 * Library: PtopLib
 * File: ptopServGetClients.c
 * Author: B&R
 * Created: February 26, 2014
 *******************************************************************/

#ifdef _DEFAULT_INCLUDES
#include <AsDefault.h>
#endif

#include <bur/plctypes.h>
#include <astime.h>
#include "ptoplibinc.h"
#include "stdio.h"
#include "../server/ptpserv.h"
#include "../client/allgclnt.h"

static USINT chFileWriteBuffer[20000];   /* Schreibbuffer für Filehandling */

void BuildTimeStampForFile (DATE_AND_TIME TimeStamp, STRING* pReturnTimeStamp, BOOL withDate);
void ResetFileBuffer();
void WriteOverviewLine(ClientListeEntry_typ sClientList, UINT iActClient, USINT* pBuffer, UINT* piActBuffer);
BOOL WritePvs(PVListEntry_typ sPv, USINT* pBuffer, INT iActServer, UINT* piActBuffer);
void WriteClientMain(PVListEntry_typ* pPVList, USINT* pWriteBuffer, UINT iActClient, UINT* piActBuffer);

/************************************************************************
*   Ermittlung der Clients + PVs auf einem Server
*************************************************************************/

void PtopServGetClients(struct PtopServGetClients* inst)
{
  USINT szHelp[80];
  USINT szHelp1[32];
  UINT uiState;
  ClientListeEntry_typ    *pClientList;
  PVListEntry_typ         *pPVList;
  INT i;
  INT iPv;
  BOOL bRetVal;
  UDINT* pudAdr;
  
  if (inst->enable == FALSE)
  {
    return;
  }
  
  inst->status = ERR_FUB_BUSY;
  
  switch (inst->sIntern.iStep)
  {
    case SERV_CLIENTS_IDLE:
      {
        if (inst->BuildFile == TRUE)
        {
          inst->BuildFile = FALSE;
          
          inst->sIntern.iWriteFilePos = 0;
          inst->sInfos.sErr.bErr = FALSE;
          inst->sIntern.iStep = SERV_CLIENTS_GET_PV_ADR_1;
        }
        break;
      }
    
    case SERV_CLIENTS_GET_PV_ADR_1:
      {
        szHelp[0] = 0;

        brsstrcpy((UDINT) &szHelp, (UDINT) "server:pClientListeStart");
        uiState = PV_xgetadr((char*) &szHelp, (UDINT*) &pudAdr,(UDINT*) &inst->sIntern.sServerInfo.iServerLen);
        if (uiState != 0)
        {          
          inst->sInfos.sErr.status = STATE_PV_PVADR_NOT_FOUND;
          inst->sInfos.sErr.step = inst->sIntern.iStep;
          
          inst->sIntern.iStep = SERV_CLIENTS_ERR;
        }
        else
        {
          inst->sIntern.sServerInfo.udAdrServer = *pudAdr;
          inst->sIntern.iStep = SERV_CLIENTS_GET_PV_ADR_2; 
        }
        
        break;
      }
    
    case SERV_CLIENTS_GET_PV_ADR_2:
      {
        szHelp[0] = 0;

        brsstrcpy((UDINT) &szHelp, (UDINT) "server:pPVListeStart");
        uiState = PV_xgetadr((char*) &szHelp, (UDINT*) &pudAdr,(UDINT*) &inst->sIntern.sServerInfo.iPvListeLen);
        if (uiState != 0)
        {
          inst->sInfos.sErr.status = STATE_PV_PVADR_NOT_FOUND;
          inst->sInfos.sErr.step = inst->sIntern.iStep;
          
          inst->sIntern.iStep = SERV_CLIENTS_ERR;
        }
        else
        {
          inst->sIntern.sServerInfo.udAdrPvListe = *pudAdr;
          inst->sIntern.iStep = SERV_CLIENTS_ORGANIZE_FILE; 
        }
        break;
      }
    
    
    case SERV_CLIENTS_ORGANIZE_FILE:
      {
        // Zeitstempel ermitteln
        inst->sIntern.sFubCaller.DtGetTime_Fub.enable = 1;
        DTGetTime(&inst->sIntern.sFubCaller.DtGetTime_Fub);
        if (inst->sIntern.sFubCaller.DtGetTime_Fub.status != ERR_FUB_BUSY)
        {
          szHelp1[0] = 0;
          BuildTimeStampForFile(inst->sIntern.sFubCaller.DtGetTime_Fub.DT1, (STRING*) &szHelp1, 1);
          brsstrcpy((UDINT) &inst->sInfos.szFileName, (UDINT) "ptop_server_");
          brsstrcat((UDINT) &inst->sInfos.szFileName, (UDINT) &szHelp1);
          brsstrcat((UDINT) &inst->sInfos.szFileName, (UDINT) ".csv");
          
          inst->sIntern.iStep = SERV_CLIENTS_CREATE_FILE; 
        }
        
        break;
      }
    
    case SERV_CLIENTS_CREATE_FILE:
      {
        inst->sIntern.sFubCaller.FileCreate_Fub.enable = 1;
        inst->sIntern.sFubCaller.FileCreate_Fub.pDevice = (UDINT) &inst->szDeviceName;
        inst->sIntern.sFubCaller.FileCreate_Fub.pFile = (UDINT) &inst->sInfos.szFileName;
        FileCreate(&inst->sIntern.sFubCaller.FileCreate_Fub);
        if (inst->sIntern.sFubCaller.FileCreate_Fub.status != ERR_FUB_BUSY)
        {
          if (inst->sIntern.sFubCaller.FileCreate_Fub.status == ERR_OK)
          {
            inst->sIntern.iStep = SERV_CLIENTS_CREATE_CL_OVERVIEW;
          }
          else
          {
            inst->sInfos.sErr.status = inst->sIntern.sFubCaller.FileCreate_Fub.status;
            inst->sInfos.sErr.step = inst->sIntern.iStep;
            inst->sIntern.iStep = SERV_CLIENTS_ERR;
          }
        }
        break;
      }
    
    case SERV_CLIENTS_CREATE_CL_OVERVIEW:
      {
        ResetFileBuffer();
        
        /* Erstellung der Übersichtsliste */
        pClientList = (ClientListeEntry_typ*) inst->sIntern.sServerInfo.udAdrServer;
        for (i=0; i < MAX_SERVER; i++, pClientList++)
        {
          WriteOverviewLine(*pClientList, i + 1, (USINT*) &chFileWriteBuffer, &inst->sIntern.iBufferPos);          
        }
        inst->sIntern.iGetBufLen = brsstrlen((UDINT)&chFileWriteBuffer);
        /* mind. ein Client ist eingetragen? */
        if (inst->sIntern.iGetBufLen != 0)
        {
          inst->sIntern.iStep = SERV_CLIENTS_WRITE_CL_OVERVIEW;
        }
        else
        {
          /* nichts gefunden -> Ende...*/
          inst->sIntern.iStep = SERV_CLIENTS_CLOSE_FILE;
        }
        
        break;
      }
    
    case SERV_CLIENTS_WRITE_CL_OVERVIEW:
      {
        inst->sIntern.sFubCaller.FileWrite_Fub.enable = 1;
        inst->sIntern.sFubCaller.FileWrite_Fub.ident = inst->sIntern.sFubCaller.FileCreate_Fub.ident;
        inst->sIntern.sFubCaller.FileWrite_Fub.len = inst->sIntern.iGetBufLen;
        inst->sIntern.sFubCaller.FileWrite_Fub.offset = inst->sIntern.iWriteFilePos;
        inst->sIntern.sFubCaller.FileWrite_Fub.pSrc = (UDINT) &chFileWriteBuffer;
        FileWrite(&inst->sIntern.sFubCaller.FileWrite_Fub);
        if (inst->sIntern.sFubCaller.FileWrite_Fub.status != ERR_FUB_BUSY)
        {
          if (inst->sIntern.sFubCaller.FileWrite_Fub.status == ERR_OK)
          {
            inst->sIntern.iWriteFilePos +=inst->sIntern.iGetBufLen;
            
            /* Init PV schreiben */
            inst->sIntern.iActClient = 1;/* start bei 1*/            
            inst->sIntern.iActPv = 0;
            
            inst->sIntern.iStep = SERV_CLIENTS_CREATE_PV_LIST;
          }
          else
          {
            inst->sInfos.sErr.status = inst->sIntern.sFubCaller.FileWrite_Fub.status;
            inst->sInfos.sErr.step = inst->sIntern.iStep;
            inst->sIntern.iStep = SERV_CLIENTS_ERR;
          }
        }
        
        break;
      }
    
    case SERV_CLIENTS_CREATE_PV_LIST:
      {
        ResetFileBuffer();
        inst->sIntern.iBufferPos = 0;
        
        pPVList = ((PVListEntry_typ*) inst->sIntern.sServerInfo.udAdrPvListe) + inst->sIntern.iActPv;
        
        if (pPVList->pPv != 0)
        {
          // Client Überschrift eintragen:
          WriteClientMain(pPVList, (USINT*) &chFileWriteBuffer, inst->sIntern.iActClient, &inst->sIntern.iBufferPos);
          
          for (iPv=0; iPv < MAX_CLIENT_VARIABLEN; iPv++, pPVList++)
          {
            /* Clients starten mit 1 -> +1*/
            /* es gibt keine Lücken in Clientzuordnung -> Abbruch*/
            bRetVal = WritePvs(*pPVList, (USINT*) &chFileWriteBuffer, inst->sIntern.iActClient, &inst->sIntern.iBufferPos);
            if (bRetVal == TRUE)
            {
              break;
            }
          }
          inst->sIntern.iActClient++;
          inst->sIntern.iGetBufLen = brsstrlen((UDINT)&chFileWriteBuffer);
          inst->sIntern.iStep = SERV_CLIENTS_WRITE_PV_LIST;
        }
        else
        {
          /* Ende: */
          inst->sIntern.iStep = SERV_CLIENTS_CLOSE_FILE;
        }
        break;
      }
    
    case SERV_CLIENTS_WRITE_PV_LIST:
      {
        inst->sIntern.sFubCaller.FileWrite_Fub.enable = 1;
        inst->sIntern.sFubCaller.FileWrite_Fub.ident = inst->sIntern.sFubCaller.FileCreate_Fub.ident;
        inst->sIntern.sFubCaller.FileWrite_Fub.len = inst->sIntern.iGetBufLen;
        inst->sIntern.sFubCaller.FileWrite_Fub.offset = inst->sIntern.iWriteFilePos;
        inst->sIntern.sFubCaller.FileWrite_Fub.pSrc = (UDINT) &chFileWriteBuffer;
        FileWrite(&inst->sIntern.sFubCaller.FileWrite_Fub);
        if (inst->sIntern.sFubCaller.FileWrite_Fub.status != ERR_FUB_BUSY)
        {
          if (inst->sIntern.sFubCaller.FileWrite_Fub.status == ERR_OK)
          {
            inst->sIntern.iWriteFilePos += inst->sIntern.iGetBufLen;
            
            if (inst->sIntern.iActClient < MAX_ANZAHL_CLIENTS)
            {
              inst->sIntern.iStep = SERV_CLIENTS_CREATE_PV_LIST;
            }
            else
            {
              inst->sIntern.iStep = SERV_CLIENTS_CLOSE_FILE;
            }
            
          }
          else
          {
            inst->sInfos.sErr.status = inst->sIntern.sFubCaller.FileWrite_Fub.status;
            inst->sInfos.sErr.step = inst->sIntern.iStep;
            inst->sIntern.iStep = SERV_CLIENTS_ERR;
          }
        }
        break;
      }
    
    case SERV_CLIENTS_CLOSE_FILE:
      {
        inst->sIntern.sFubCaller.FileClose_Fub.enable = TRUE;
        inst->sIntern.sFubCaller.FileClose_Fub.ident = inst->sIntern.sFubCaller.FileCreate_Fub.ident;
        FileClose(&inst->sIntern.sFubCaller.FileClose_Fub);
        if (inst->sIntern.sFubCaller.FileClose_Fub.status != ERR_FUB_BUSY)
        {
          if (inst->sIntern.sFubCaller.FileClose_Fub.status == ERR_OK)
          {
            inst->sIntern.iStep = SERV_CLIENTS_IDLE;
            inst->status = ERR_OK;
          }
          else
          {
            inst->sInfos.sErr.status = inst->sIntern.sFubCaller.FileClose_Fub.status;
            inst->sInfos.sErr.step = inst->sIntern.iStep;
          
            inst->sIntern.iStep = SERV_CLIENTS_ERR;
          }
        }
        
        break;
      }
    
    case SERV_CLIENTS_ERR:
      {
        
        /* File schliessen */
        inst->sIntern.sFubCaller.FileClose_Fub.enable = TRUE;
        inst->sIntern.sFubCaller.FileClose_Fub.ident = inst->sIntern.sFubCaller.FileCreate_Fub.ident;
        FileClose(&inst->sIntern.sFubCaller.FileClose_Fub);
        inst->sInfos.sErr.bErr = TRUE;
        
        inst->sIntern.iStep = SERV_CLIENTS_IDLE;
        
        inst->status = inst->sInfos.sErr.status;
        
        break;
      }
  }
  
}


/* Zeitstempel für File bauen */
void BuildTimeStampForFile (DATE_AND_TIME TimeStamp, STRING* pReturnTimeStamp, BOOL withDate)
{
  DTStructure DTStruct;
  USINT szVal[5];  
  szVal[0] = 0;
  
  DT_TO_DTStructure(TimeStamp, (UDINT) &DTStruct);
  
  // Datum miterzeugen
  if (withDate == TRUE)
  {    
    
    // Jahr
    brsitoa(DTStruct.year, (UDINT) &szVal);
    brsstrcpy((UDINT) pReturnTimeStamp, (UDINT) &szVal);    
    
    //Monat
    brsitoa(DTStruct.month, (UDINT) &szVal);
    if (brsstrlen((UDINT) &szVal)== 1)
    {
      brsstrcat((UDINT) pReturnTimeStamp, (UDINT) "0");
      brsstrcat((UDINT) pReturnTimeStamp, (UDINT) &szVal);
    }
    else
    {
      brsstrcat((UDINT) pReturnTimeStamp, (UDINT) &szVal);
    }
    
    brsitoa(DTStruct.day, (UDINT) &szVal);
    // Tag
    if (brsstrlen((UDINT)&szVal)== 1)
    {
      brsstrcat((UDINT) pReturnTimeStamp, (UDINT) "0");
      brsstrcat((UDINT) pReturnTimeStamp, (UDINT) &szVal);
    }
    else
    {
      brsstrcat((UDINT) pReturnTimeStamp, (UDINT) &szVal);
    }
    brsstrcat((UDINT) pReturnTimeStamp, (UDINT) "_");
  }  
  
  // Stunden
  brsitoa(DTStruct.hour, (UDINT) &szVal);
  if (brsstrlen((UDINT)&szVal)== 1)
  {    
    brsstrcat((UDINT) pReturnTimeStamp, (UDINT) "0");
    brsstrcat((UDINT) pReturnTimeStamp, (UDINT) &szVal);
  }
  else
  {
    brsstrcat((UDINT) pReturnTimeStamp, (UDINT) &szVal);
  }  
  // Minuten
  brsitoa(DTStruct.minute, (UDINT) &szVal);
  if (brsstrlen((UDINT)&szVal)== 1)
  {
    brsstrcat((UDINT) pReturnTimeStamp, (UDINT) "0");
  }  
  brsstrcat((UDINT) pReturnTimeStamp, (UDINT) &szVal);  
  brsitoa(DTStruct.second, (UDINT) &szVal);
  if (brsstrlen((UDINT)&szVal)== 1)
  {
    brsstrcat((UDINT) pReturnTimeStamp, (UDINT) "0");
  }
  brsstrcat((UDINT) pReturnTimeStamp, (UDINT) &szVal);
}


/* Zurücksetzen des Schreibbuffers */
void ResetFileBuffer()
{
  brsmemset((UDINT) &chFileWriteBuffer, 0, sizeof(chFileWriteBuffer));
}


/* Erstellung einer Zeile innerhalb des Server Files: */
void WriteOverviewLine(ClientListeEntry_typ sClientList, UINT iActClient, USINT* pWriteBuffer, UINT* piActBuffer)
{
  USINT szTemp[10];
  UDINT pBuffer;
  UINT iLen;
  
  pBuffer = (UDINT) &pWriteBuffer[*piActBuffer];
  
  if (sClientList.IpAdr[0] != 0)
  {
    szTemp[0] = 0;  
    brsstrcpy((UDINT) pBuffer, (UDINT) "Client;");
    brsstrcat((UDINT) pBuffer, (UDINT) &sClientList.IpAdr);
    brsstrcat((UDINT) pBuffer, (UDINT) ";");
    brsstrcat((UDINT) pBuffer, (UDINT) "RF=");
    brsitoa(sClientList.Timeout, (UDINT) &szTemp);
    brsstrcat((UDINT) pBuffer, (UDINT) &szTemp);
    brsstrcat((UDINT) pBuffer, (UDINT) ";");
    szTemp[0] = 0;
    brsitoa(iActClient, (UDINT) &szTemp);
    brsstrcat((UDINT) pBuffer, (UDINT) "Index=");
    brsstrcat((UDINT) pBuffer, (UDINT) &szTemp);
    brsstrcat((UDINT) pBuffer, (UDINT) ";");
    brsstrcat((UDINT) pBuffer, (UDINT) "\r\n");    /* Zeilenumbruch */
    
    iLen = brsstrlen((UDINT) pBuffer);
    *piActBuffer =+ iLen;
  }
}

/* Erstellung eines Variablenblocks innerhalb des Server Files: */
BOOL WritePvs(PVListEntry_typ sPv, USINT* pWriteBuffer, INT iActServer, UINT* piActBuffer)
{
  int i;
  USINT szTemp[10];
  BOOL bRetVal;
  UINT iLen;
  UDINT pBuffer;
  
  bRetVal = FALSE;
  
  if (sPv.pPv != 0)
  {
    for (i=0; i < MAX_ANZAHL_CLIENTS;i++)
    {
      if (sPv.Client[i].iClntIndex == iActServer)
      {
        pBuffer = (UDINT) &pWriteBuffer[*piActBuffer];
        
        szTemp[0] = 0;  
        brsstrcpy((UDINT) pBuffer, (UDINT) "Variable: ");
        brsstrcat((UDINT) pBuffer, (UDINT) &sPv.Name);
        brsstrcat((UDINT) pBuffer, (UDINT) "; Sync: ");
        brsitoa(sPv.Tick.Intervall, (UDINT) &szTemp);
        brsstrcat((UDINT) pBuffer, (UDINT) &szTemp);
        brsstrcat((UDINT) pBuffer, (UDINT) "; Hysterese: ");
        brsitoa(sPv.Hysterese, (UDINT) &szTemp);
        brsstrcat((UDINT) pBuffer, (UDINT) &szTemp);
        brsstrcat((UDINT) pBuffer, (UDINT) ";");
        brsstrcat((UDINT) pBuffer, (UDINT) "\r\n");    /* Zeilenumbruch */
            
        iLen = brsstrlen((UDINT) pBuffer);
        *piActBuffer += iLen;
        
        bRetVal = FALSE;
        break;
      }
      else if (sPv.Client[i].iClntIndex == 0)
      {
        bRetVal = TRUE;
        break;
      }
    }
  }
  else
  {
    bRetVal = TRUE;
  }
 
  return bRetVal;
}


void WriteClientMain(PVListEntry_typ* pPVList, USINT* pWriteBuffer, UINT iActClient, UINT* piActBuffer)
{
  int iPv, i;
  USINT szTemp[10];
  UINT iLen;
  UDINT pBuffer;
  PVListEntry_typ* pWorkPv;
  
  
  for (iPv=0; iPv < MAX_CLIENT_VARIABLEN; iPv++)
  {
    pWorkPv = pPVList + iPv;
    if (pWorkPv->pPv != 0)
    {
      for (i=0; i < MAX_ANZAHL_CLIENTS; i++)
      {
        
        if (pWorkPv->Client[i].iClntIndex == iActClient)
        {
          // gefunden : Eine Zeile einfügen
          pBuffer = (UDINT) &pWriteBuffer[*piActBuffer];
        
          szTemp[0] = 0;  
          brsstrcpy((UDINT) pBuffer, (UDINT) "\r\n");
          brsstrcat((UDINT) pBuffer, (UDINT) "Client -  ");
          brsitoa(iActClient, (UDINT) &szTemp);
          brsstrcat((UDINT) pBuffer, (UDINT) &szTemp);
          brsstrcat((UDINT) pBuffer, (UDINT) ";");
          brsstrcat((UDINT) pBuffer, (UDINT) "\r\n");
          
          iLen = brsstrlen((UDINT) pBuffer);
          *piActBuffer += iLen;
          
          return;
        }
        else if (pWorkPv->Client[i].iClntIndex == 0)
        {
          break;
        }        
      }
    }
    else
    {
      // Ende erreicht
      return;
    }
  }
}
