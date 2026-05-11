/*****************************************************************************************************************/
/*                    C - File für UDP Server                                                                                                                        */ 
/*------------------------------------------------------------------------------------------------------------------------------------------*/
/*            Taskname: server
*            Dateiname: server.c
*            Autor:  B&R
*            Erstelldatum: Mai 2002
*            Classtime: 10 - 30000 ms
-------------------------------------------------------------------------------------------------------------------------------------------*/
/*  Funktion:
*          - Speicher anlegen
*          - Monitorfunktionen
*          - Anlegen der Variablen, Eventauswertung, Lifecheck
*          - Fehler u. Statuskonstanten
* 
*****************************************************************************************************************/
/*  Revisionstext:
*    -   siehe Revisionstext UDP.doc
*
*****************************************************************************************************************/
#ifndef _REPLACE_CONST
#define _REPLACE_CONST    // ab AS 3.0 werden Konstanten default als globale Variablen angelegt
                            // mit dem define _REPLACE_CONST wird dieses Verhalten abgeschaltet!
#endif

#ifdef _DEFAULT_INCLUDES
#include <AsDefault.h>
#endif

#include <bur/plc.h>
#include <bur/plctypes.h>
#include "../processor.h"
#include <astime.h>
#include <asudp.h>
#include <dataobj.h>
#include <standard.h>
#include <sys_lib.h>
#include <asieccon.h>

#if INTEL
  #include <asarcfg.h>
  #include <brsystem.h>
#endif

#include "allgserv.h"
#include "pipe.h"
#include "config.h"
#include "PtPServ.h"
#include "errserv.h"
#include "../diagnostic.h"
#include "funcserver.h"
#include "../techguard.h"
#include "../stephandler.h"


/*****************************************************************************************************************/
/*                      Internal defines                                                                         */
/*****************************************************************************************************************/

#define SERVER_VERSION_TXT  "server: Version V6.50.0"

/*****************************************************************************************************************/
/*                       Prototyping                               */
/*****************************************************************************************************************/

UINT CRCcheck (UDINT* TelegrammStartAdresse, UINT Telegrammlaenge);
UDINT TimerFunction (UDINT *AdrTickCount);
UDINT SwapUdintForIntel(UDINT value);
UINT SwapUintForIntel(UINT value);
UDINT GetOwnIpAddress();
void CopyReceiveBufferToClientConnectRequest(UDINT* pRecvBuf, ClientConnectRequest_typ* pClientConnectRequest);
void CopyClientConnectResponseToSendBuffer(UDINT* pSendBuf, ClientConnectResponse_typ* pClientConnectResponse);
void CopyReceiveBufferToPVOpenRequest(UDINT* pRecvBuf, PVOpenRequest_typ* pOpenRequest);
void CopyPVOpenResponseToSendBuffer(UDINT *pSendBuf, PVOpenResponse_typ* pPVOpenResponse);
void CopyReceiveBufferToClientLink(UDINT* pRecvBuf, ClientLink_typ* pClientLink);
void CopyNAKToSendBuffer(UDINT* pSendBuf, AckResponse_typ* pNAK);
void CopyPVEventToSendBufferEvent(UDINT* pSendBuf, PVEvent_typ* pPVEvent);
void CopyReceiveBufferEventToPVEventAcknowledge();
void CopyReceiveBufferLifeToLifeCheckRequest();
void CopyLifeCheckResponseToSendBuffer();
UINT DeleteExistingClient(INT iClientNr);
void ResetPipe(INT iClientNr);
void CyclicSLSConnectToUst();
void CyclicSLSEventsToPipe(BOOL *pbClientCheck);
void CyclicSLSEventsSend();
void CyclicGetPipeInfo();
void CyclicSLSEventsRecv();
void CyclicSLSLifeCheck();
void CyclicGetLifeStateOfClients();
void GenerateOpenHeaderTelegramm(UDINT *pKennung, USINT iModus);
void GenerateConnHeaderTelegramm(UDINT *pKennung, USINT iModus);
BOOL ValidateDataFromRecvBuf(UDINT *pRecvBufOpen, UINT uiTelLen);
BOOL GetDataFromRecvBufOpen(UDINT* pRecvBufOpen, UINT uiPvCnt, PVOpenRequestX_typ* pPVOpenRequestX, UINT* puiPvEntry);
UINT ExistsPvInList(UDINT udAdrPv, UINT  uiClientIndex, UINT* puiPvOffset, UINT* puiClntOffset);
UINT ExistsPvInListNew(UDINT udNamePv, UINT  uiClientIndex, UINT* puiPvOffset, UINT* puiClntOffset);
void InsertNewPvInList(PVOpenRequestX_typ sPVOpenRequestX, UDINT udPvAdrServer, UDINT udPvLen, UINT uiPvOffset, UINT uiClntOffset, UDINT* pudVal, BOOL bInsertNew);
void CreateHeaderInRespBuf(USINT* pSendBuf, UINT uiPvCnt, UINT uiClientIndex, UINT* pTelLen);
void GetMainDataFromRecvBufOpen(UDINT* pRecvBufOpen, UINT*  puiPvCnt, UINT*  puiClientIndex);
void CreateNewEntryInRespBuf(UDINT* pSendBuf, PVOpenRequestX_typ sPVOpenRequestX, UINT uiPvEntry,  UDINT udPvAdrServer, UDINT udPvLen, UDINT udVal, UINT uiStatus, UINT* pTelLen);
void CreateCrcInRespBuf(UDINT* pSendBuf, UINT* puiSendTelLen);
BOOL CheckPipeData(EventSend_typ* pEventSend);
BOOL GetEventPipeDataNewMode(EventSend_typ* pEventSend, UDINT* pSendBuf, UINT* puiSendTelLen, UINT* puiClientNr);
BOOL GetEventPipeDataOldMode(  EventSend_typ* pEventSend, UINT* puiClientNr, PVEvent_typ* pEvent);
UINT GetPipeInfo(UINT uiNr);
void SetPtrToNull();
UINT CreateDataObjMem(MemManagerServ_type* pMemManger, DatObjCreate_typ* pDataObjCreateFub, DatObjInfo_typ* pDatObjInfoFub, DatObjDelete_typ* pDatObjDeleteFub);
UDINT ValidatePointerServer(UDINT udAdr, UINT  uiLocation, MemManagerDiag_type* pMemDiag, UINT  uiTypeOfMem);
BOOL CheckTelegramHeader(UDINT* pKennung, STRING* pTelegramHeader);
USINT SetMode(UDINT* pTelegrammKennung);
DINT WriteDiagDataToBuRLoggerNormal(BOOL EnableWrite, char* pTask, char* pFubName, char* pStepName, char* pFreeText, UDINT iState);
UDINT GetAbsoluteValue(UDINT NewValue, UDINT OldValue);
UINT WriteStepDataToBuRLogger(Logger_typ* pLogger, ServBurLogCtrlStepsInt_typ* pDetailLogger, ServBurLog_typ* pLogManager, char* pFubName,  char* pStepName, DINT Step, UDINT ActTime);
UINT WriteDataToBuRLogger(Logger_typ* pLogger, ServBurLogCtrlStepsInt_typ* pDetailLogger, ServBurLog_typ* pLogManager, char* pFubName, char* pStepName, char* pAddName, DINT AdditionalStatus, DINT Step, UDINT ActTime);
UINT WriteRegDataToBuRLogger(Logger_typ* pLogger, ServBurLogCtrlStepsInt_typ* pDetailLogger, ServBurLog_typ* pLogManager, char* pFubName, UINT  PvIndex, UINT ClientIndex, DINT Step, UDINT ActTime	);
BOOL GetServerPvListProblems(UDINT* pPvStartAdr, UDINT* pClientStartAdr, PvServListMain_typ* pPvListMain);
UDINT RemoveCompilerWarning(UDINT Param);

/*****************************************************************************************************************/
/*                      Makros                                                                                   */
/*****************************************************************************************************************/

#define ABS(N) ((N<0)?(-N):(N))

/*****************************************************************************************************************/
/*                      Variablendeklaration                                                                     */
/*****************************************************************************************************************/

// Hier muss irgendein neues Pipe Konstrukt gebaut werden!
// AS Variablenliste ändert eine Pointerliste zu einer Referenz auf einen Array um, damit kommen die Fubs nicht klar!
PipeManagement_typ  *pReadPipe[MAX_ANZAHL_CLIENTS_ARR];     /* Lesezeiger der Pipes     */
PipeManagement_typ  *pWritePipe[MAX_ANZAHL_CLIENTS_ARR];    /* Schreibzeiger der Pipes    */ 

/*****************************************************************************************************************
*                 Initialisierung
*****************************************************************************************************************/

_INIT void Initialisierung (void)
{
UINT uiCreateMemState = 0;
DINT iStatus = 0;
  
  RemoveCompilerWarning(LOGGER_START_ENABLE);
  RemoveCompilerWarning(Exist_LoggerEnable[0]);
  
  // - - - Erzeugen eines ptop Logbuchs oder identifizierung des bestehenden Logbuchs
  do
  {
    iStatus = CyclicBurLoggerCreate(&sLogger, LOGGER_NAME_SERV);
  } while (iStatus == ERR_FUB_BUSY);
  
  WriteLoggerMessage((Logger_typ*) &sLogger, 0xF000, (STRING*) "server:Init - Start", 0x7FFFFFFF, PTOP_LOG_SEVERITY_INFO);
  WriteLoggerMessage((Logger_typ*) &sLogger, 0xF000, (STRING*) SERVER_VERSION_TXT, 0x7FFFFFFF, PTOP_LOG_SEVERITY_INFO);
  
  SwVersion = PTOP_VERSION;
  
  // Voreinstellungen für den Datenlogger
  CmdBurLogBook.Settings.NrOfLogEntries = 100;
  CmdBurLogBook.Settings.MeasurementTime = 5000;	// 5 seconds
  CmdBurLogBook.Settings.MeasurementType = LOG_INFINITE;
  CmdBurLogBook.Steps.Connect.Checked = 1;
  CmdBurLogBook.Steps.Lifecheck.Checked = 1;
  brsstrcpy((UDINT) &CmdBurLogBook.Title, (UDINT) "Default");
  CmdBurLogBook.StartLogging = LOGGER_START_ENABLE;
  
  // Port Belegungen
  PortSource[0] = 20001;    /* Senden Server Anmeldung            */
  PortSource[1] = 20002;    /* Empfangen Server Anmeldung           */
  PortSource[2] = 20005;    /* Senden Events Server -> Client         */
  PortSource[3] = 20006;    /* Empfangen Eventbestätigung Client -> Server  */
  PortSource[4] = 20009;    /* Senden Server Lifecheck            */
  PortSource[5] = 20010;    /* Empfangen Server Lifecheck         */
  
  PortTarget[0] = 20003;    /* Empfangen Client Anmeldung           */
  PortTarget[1] = 20004;    /* Senden Client Anmeldung              */
  PortTarget[2] = 20007;    /* Sendeport  Client              */
  PortTarget[3] = 20008;    /* Empfangsport Client              */
  PortTarget[4] = 20011;    /* Sendeport  Client  Lifecheck       */
  PortTarget[5] = 20012;    /* Empfangsport Client  Lifecheck       */
  
  /*  ------------------------ UDP Ports öffnen --------------------------------  */
  for (iPortNr = 0; iPortNr < PORT_CNT; iPortNr++) 
  { 
    UDPopenFub[iPortNr].enable = TRUE;
    UDPopenFub[iPortNr].port = PortSource[iPortNr];
    UdpOpen(&UDPopenFub[iPortNr]);
  }

  /* --- Ermittlung der eigenen IP - Adresse*/
  IpAdrSource = GetOwnIpAddress();

  /* --- Umwandlung der Adresse in einen String und ablegen in interner Verwaltung */
  ConvertIpAdrToAsciiX(IpAdrSource, (UDINT*) &arIpAdrSource);

  /* --- Schrittschaltwerke dürfen nicht laufen, bevor die Schnittstelle geöffnet worden ist ------------------  */
  StepConnect.Step.StepNr    = UDP_SCHRITT_INIT;
  UDPLifeCheck  = LIFE_CHECK_IDLE;
  StepEventRecv.Step.StepNr = UDP_EVENT_IDLE;
  StepEventSend.Step.StepNr = UDP_EVENT_IDLE;

  bConfState = GetConfData((void*) &sPtopConf, sizeof(sPtopConf), 1);
  if (bConfState == FALSE)
  {
    sPtopConf.uiPipeLen = PIPE_SIZE;
    sPtopConf.uiPvPerCycle = MAX_PV_LOOP_CNT;
    sPtopConf.uiPvStored = MAX_CLIENT_VARIABLEN;
    sPtopConf.uiClientStored = MAX_ANZAHL_CLIENTS;
  }
      
  /* === Speicherverwaltung ======================================================================== */
  sMemManager.sPVListeSpeicherPlatz.udLen = sizeof(PVListEntry_typ) * sPtopConf.uiPvStored;
  sMemManager.sClientListeSpeicherPlatz.udLen = sizeof(ClientListeEntry_typ) * sPtopConf.uiClientStored;
  sMemManager.sPipeSpeicherPlatz.udLen = sizeof(PipeManagement_typ) * sPtopConf.uiPipeLen * MAX_ANZAHL_CLIENTS;

  sMemManager.sSendBufOpen.udLen = MAX_UDP_FRAME_LEN;
  sMemManager.sRecvBufOpen.udLen = MAX_UDP_FRAME_LEN;
  sMemManager.sSendBufEvnt.udLen = MAX_UDP_FRAME_LEN;
  sMemManager.sRecvBufEvnt.udLen = MAX_UDP_FRAME_LEN;

  sMemManager.udMemory = sMemManager.sPVListeSpeicherPlatz.udLen + 
  						          sMemManager.sClientListeSpeicherPlatz.udLen + 
			                  sMemManager.sPipeSpeicherPlatz.udLen + 
              			    sMemManager.sSendBufOpen.udLen + 
              			    sMemManager.sRecvBufOpen.udLen + 
			                  sMemManager.sSendBufEvnt.udLen + 
			                  sMemManager.sRecvBufEvnt.udLen;
  
  
  /* === Datenmodul anlegen ======================================================================== */
  uiCreateMemState = CreateDataObjMem(&sMemManager, &DataObjCreateFub, &DatObjInfoFub, &DatObjDeleteFub);
  
  if (uiCreateMemState != 0)
  {
    StatusTmpAlloc[0] = TRUE;
    brsitoa(uiCreateMemState, (UDINT) &HelpString2);
    brsstrcpy((UDINT) &HelpString1, (UDINT) "server:Fehler bei DM - ");
    brsstrcat((UDINT) &HelpString1,(UDINT) & HelpString2);
        
    WriteLoggerMessage((Logger_typ*) &sLogger, 0xF000, (STRING*) &HelpString1, 0x7FFFFFFF, PTOP_LOG_SEVERITY_INFO);
    return;
  }
  
  bDataObjFlag = TRUE;
  
  brsmemset((UDINT) sMemManager.udDOPtr, 0, sMemManager.udMemory);
  
  udAdrOffset = 0;
  pPVListeStart = (PVListEntry_typ*) (sMemManager.udDOPtr + udAdrOffset);
  sMemManager.sPVListeSpeicherPlatz.udAdr = sMemManager.udDOPtr + udAdrOffset;
  udAdrOffset = sMemManager.sPVListeSpeicherPlatz.udLen;
  
  pClientListeStart = (ClientListeEntry_typ*) (sMemManager.udDOPtr + udAdrOffset);
  sMemManager.sClientListeSpeicherPlatz.udAdr = sMemManager.udDOPtr + udAdrOffset;
  udAdrOffset += sMemManager.sClientListeSpeicherPlatz.udLen;
  
  /* --- Speicherallokierung für Pipeverwaltung -------------------------------------------------------- */ 
  sMemManager.sPipeSpeicherPlatz.udAdr = sMemManager.udDOPtr + udAdrOffset;
  for (iClient = 0; iClient < MAX_ANZAHL_CLIENTS; iClient++)
  {     
    PipeStartAdress[iClient] = sMemManager.udDOPtr + udAdrOffset;
    /* --- Stopadresse um Gesamtpipelänge - 1 verschieben --------------------------------------------------- */
    PipeStopAdress[iClient] = sMemManager.udDOPtr + udAdrOffset + sizeof(PipeManagement_typ) * (sPtopConf.uiPipeLen - 1);   
    pReadPipe[iClient] =  (PipeManagement_typ*)PipeStartAdress[iClient];
    pWritePipe[iClient] = (PipeManagement_typ*)PipeStartAdress[iClient];
    
    udAdrOffset += sizeof(PipeManagement_typ) * sPtopConf.uiPipeLen;
  }
  
  /* --- Buffer für neuen Modus allokieren ------------------------------------------------------------- */
  pSendBufOpen = (UDINT*) (sMemManager.udDOPtr + udAdrOffset);
  sMemManager.sSendBufOpen.udAdr = sMemManager.udDOPtr + udAdrOffset;
  sMemManager.sSendBufOpen.udLen = MAX_UDP_FRAME_LEN;
  udAdrOffset += MAX_UDP_FRAME_LEN;
  
  pRecvBufOpen = (UDINT*) (sMemManager.udDOPtr + udAdrOffset);
  sMemManager.sRecvBufOpen.udAdr = sMemManager.udDOPtr + udAdrOffset;
  sMemManager.sRecvBufOpen.udLen = MAX_UDP_FRAME_LEN;
  udAdrOffset += MAX_UDP_FRAME_LEN;
  
  pSendBufEvent = (UDINT*) (sMemManager.udDOPtr + udAdrOffset);
  sMemManager.sSendBufEvnt.udAdr = sMemManager.udDOPtr + udAdrOffset;
  sMemManager.sSendBufEvnt.udLen = MAX_UDP_FRAME_LEN; 
  udAdrOffset += MAX_UDP_FRAME_LEN;
  
  pRecvBufEvent = (UDINT*) (sMemManager.udDOPtr + udAdrOffset);
  sMemManager.sRecvBufEvnt.udAdr = sMemManager.udDOPtr + udAdrOffset;
  sMemManager.sRecvBufEvnt.udLen = MAX_UDP_FRAME_LEN; 
  udAdrOffset += MAX_UDP_FRAME_LEN;
  
  /* --- Status der Speicherverwaltung -> Abbruchbedingung ---------------------------------------------- */
  ClientVerwaltungAktuell = STATUS_CONNECT_REQUEST_INIT;
  bOpenOk = FALSE;
      
  
  brsitoa(sMemManager.udMemory, (UDINT) &HelpString2);
  brsstrcpy((UDINT) &HelpString1, (UDINT) "server:Speicher=");
  brsstrcat((UDINT) &HelpString1, (UDINT) &HelpString2);
  brsstrcat((UDINT) &HelpString1, (UDINT) " bytes");

  /* --- benötigter Speicher ---------------------- */
  WriteLoggerMessage((Logger_typ*) &sLogger, 0xF000, (STRING*) &HelpString1, 0x7FFFFFFF, PTOP_LOG_SEVERITY_INFO);
  
  /*************************************************************************************************************/ 
  /*                     Initialisierung Eventauswertung                   */
  EventAuswertung.ActPV = 0;
  EventAuswertung.ActClient = 0;
  AckFailed = 0;
  SaveIntoPipe = FALSE;
  
  Monitor.Lock = TRUE;          /* PV + Client Monitor wird eingeschalten */
  PipeMonitor.Lock = TRUE;      /* Pipe Monitor wird eingeschalten */ 
  
  brsmemset((UDINT) &PipeLock, 0, sizeof(PipeLock_typ));
  brsmemset((UDINT) &Diagnose.Base.Events, 0, sizeof(DIAG_Event_typ));
  
  /* --- Vorbelegung der Pointer ------------------------------------------------------------------------------ */
  pClientListeWork  = pClientListeStart + 0; 
  
  ClientConfCnt = 0;
  LifeClientFound = 0;

  /* udpSendFlags = udpMSG_DONTROUTE; */
  udpSendFlags = 0; 
  
  /* --- Udp Buffergröße  initialisieren ---------------------------------------------------------------------- */
  udIoCtlParamSendBuf = UDP_BUF_SEND_SIZE;
  udIoCtlParamRecvBuf = UDP_BUF_RECV_SIZE;
  
  bClientCheck = FALSE;
  
  /* Anzahl der tatsächlich vorhandenen Variablen */
  Diagnose.Base.MaxVariables = MAX_CLIENT_VARIABLEN;
  Diagnose.Base.MaxClients = MAX_ANZAHL_CLIENTS;

  /* Speicherbereiche hinterlegen */
  sMemDiag.udStartDM = (UDINT) sMemManager.udDOPtr;
  sMemDiag.udStopDM = (UDINT) sMemManager.udDOPtr + sMemManager.udMemory;

  DTGetTime_Fub.enable = TRUE;
  WriteLoggerMessage((Logger_typ*) &sLogger, 0xF000, (STRING*) "server:Init - Ende", 0x7FFFFFFF, PTOP_LOG_SEVERITY_INFO);
  
  
} /* _INIT void */


 
/*****************************************************************************************************************
*                 Zyklischer Teil
*****************************************************************************************************************/

_CYCLIC void CyclicFunction(void)
{
	
  if (bTest)
  {
    bTest = FALSE;
  }
  
  /* === wenn Speicher nicht allokiert werden konnte wird Programm nicht ausgeführt ================= */
  if ((StatusTmpAlloc[0] != 0) || (bDataObjFlag == FALSE))
  {
    return;
  }
  
  /* - - -  Überprüfung, ob Datenobjekt, welches als Speicher verwendet wird, noch vorhanden ist - - - */
  DatObjInfoFub.enable = TRUE;
  DatObjInfoFub.pName  = (UDINT) DATA_OBJ_MEM_NAME_SERVER;
  DatObjInfo(&DatObjInfoFub);
  if (DatObjInfoFub.status != 0)
  {
    bDataObjFlag = FALSE;
    SetPtrToNull();
    WriteLoggerMessage((Logger_typ*) &sLogger, 0xF000, (STRING*) "server: Speicherverwaltungs - Datenobjekt nicht vorhanden.", DatObjInfoFub.status, PTOP_LOG_SEVERITY_INFO);
    return;
  }
  
  if (Diagnose.LogPointerInfo == TRUE)
  {
    ValidatePointerServer((UDINT) pClientListeStart, 0, (MemManagerDiag_type*) &sMemDiag, 1);	/* Erweiterung, zur Überprüfung von Adressen */
    ValidatePointerServer((UDINT) pPVListeStart, 1, (MemManagerDiag_type*) &sMemDiag, 1);	/* Erweiterung, zur Überprüfung von Adressen */
  }


  /* --- Systemzeit ermitteln ------------------------------ */
  DTGetTime(&DTGetTime_Fub);
  if (DTGetTime_Fub.status == FUB_OK)
  {
    dtActTime = DTGetTime_Fub.DT1;
  }
  
  /* ---- Aufruf des Timers -------------------------------------------------------------------------- */
  udStatusTimer = TimerFunction(&udTimeTick);
  
  /* --- Anmeldung von Client an Server -------------------------------------------------------------- */
  CyclicSLSConnectToUst();
    
  /* === PV & Client Monitor ========================================================================= */
  if (Monitor.Lock == TRUE) /* Monitor EIN - AUS */
  {
    if (Monitor.PVIndex < MAX_CLIENT_VARIABLEN)
    {
      brsmemcpy((UDINT) &PvMonitor, (UDINT)(pPVListeStart + Monitor.PVIndex), sizeof(PVListEntry_typ));
    }
    
    if (Monitor.ClientIndex < MAX_ANZAHL_CLIENTS)
    {
      brsmemcpy((UDINT)&ClientMonitor, (UDINT)(pClientListeStart + Monitor.ClientIndex), sizeof(ClientListeEntry_typ));
    }
  }


  /* === Pipe Monitor ================================================================================= */
  if (PipeMonitor.Lock == TRUE)
  {
    if (PipeMonitor.PipeIndex < sPtopConf.uiPipeLen)
    {
      pPipeMonitor = (PipeManagement_typ*)PipeStartAdress[PipeMonitor.PipeNr] + PipeMonitor.PipeIndex;
    }
  }
     
  /* --- Variablen überprüfen und Events in Pipe eintragen ---------------------------------------------*/
  CyclicSLSEventsToPipe(&bClientCheck);
  
  /* --- Pipe überprüfen und versenden der Daten zu Clients ------------------------------------------- */
  CyclicSLSEventsSend();
  
  /* --- aktuelle Pipezählerstände ermitteln ---------------------------------------------------------- */
  CyclicGetPipeInfo();
      
  /* --- Empfangsbestätigung von Clients auswerten ---------------------------------------------------- */
  CyclicSLSEventsRecv();
  
  /* --- Lifecheck für Server ------------------------------------------------------------------------- */
  CyclicSLSLifeCheck(); 

  /* --- Status des Clients ermitteln ----------------------------------------------------------------- */
  CyclicGetLifeStateOfClients();
  
  /* --- Check if in the server lists are some obious problems */
  GetServerPvListProblems( (UDINT*) pPVListeStart, (UDINT*) pClientListeStart, (PvServListMain_typ*) &Diagnose.PvList);
} /* End Cyclic*/


/************************************************************************************************************************
* === Eventauswertung: Abfrage der gelinkten Variablen und deren Clients
* --- für jede Veränderung über die Hysterese bzw. in einem festlegbaren Zeitraster wird ein Eintrag in die Eventpipe
* --- generiert
*********************************************************************************************************************** */

void CyclicSLSEventsToPipe(
BOOL *pbClientCheck)  /* Variable, bzw. Clients einer Variable prüfen  */
{
PVListEntry_typ *pPvEvent = NULL;
ClientListeEntry_typ *pClntListEvnt = NULL;
UINT uiRetValPipe = 0;      /* Status bei Pipe schreiben u. lesen  */
UINT uiEventCnt = 0;
UINT uiClientCnt = 0; 
  
  /* --- PV's kontrollieren und Werte in Pipes eintragen -------------------------------- */
  for (uiEventCnt = EventAuswertung.ActPV; uiEventCnt < MAX_CLIENT_VARIABLEN; uiEventCnt++ )
  {
    /* --- keine Überprüfung der Clients je PV ---------------------------------------- */
    if (*pbClientCheck == FALSE)
    {     
      Diagnose.Base.Events.SearchPv++;
      
      /* ---  Variablen je Zyklus --------------------------------------------------- */
      EventAuswertung.CntPV++;
      if (EventAuswertung.CntPV > sPtopConf.uiPvPerCycle)
      {
        EventAuswertung.CntPV = 0;
        break;
      }
      
      /* --- Zeiger auf PV Liste Anfang -------------------------------------------- */
      pPvEvent = pPVListeStart + EventAuswertung.ActPV;
      if (Diagnose.LogPointerInfo == TRUE)
      {
        ValidatePointerServer((UDINT) pPvEvent, 60, (MemManagerDiag_type*) &sMemDiag, 1);	/* Erweiterung, zur Überprüfung von Adressen */
      }
      EventAuswertung.ActPV++;    

      /* --- gültige Adresse vorhanden ? ------------------------------------------- */
      if (pPvEvent->pPv != NULL_ADR) 
      {           
        /* --- Überprüfung, ob Wertänderung ausserhalb der Hysterese ------------- */
        switch (pPvEvent->PvLaenge)
        {
          case 1:
            pPVWert08 = (USINT*) pPvEvent->pPv;
            EventAuswertung.Wert = *pPVWert08;
            break;
          
          case 2:
            pPVWert16 = (UINT*) pPvEvent->pPv;
            EventAuswertung.Wert = *pPVWert16;
            break;
          
          case 4:
            pPVWert32 =  (UDINT*) pPvEvent->pPv;
            EventAuswertung.Wert = *pPVWert32;
            break;

          default:
            /* letzter Wert wird zugewiesen (wahrscheinlich 0), damit Hystereseauswertung nicht greift */
            /* -> damit Abbruch */
            EventAuswertung.Wert = pPvEvent->PvWert;
            break;
        }
        
        /*  --- Eintrag in Pipe -------------------------------------------------------------------------- */
        /* -> 1. Hysterese überschritten  ODER  2. Intervall abgelaufen */
        EventAuswertung.DiffWert = GetAbsoluteValue(EventAuswertung.Wert, pPvEvent->PvWert);

        Diagnose.Base.PvCheck[EventAuswertung.ActPV - 1].uiCheckPv++;

        /* --- Falls Hysterese überschritten wurde -------------------------------------------------------- */
        if (EventAuswertung.DiffWert > pPvEvent->Hysterese) 
        {
          Diagnose.Base.PvCheck[EventAuswertung.ActPV - 1].uiCheckPvOk++;
          *pbClientCheck = FALSE;
          EventAuswertung.ActClient = 0;
          
          /* --- alle Clients für eine Variable untersuchen --------------------------------------------- */
          for (uiClientCnt = EventAuswertung.ActClient; uiClientCnt < MAX_ANZAHL_CLIENTS; uiClientCnt++)
          {
            if ((pPvEvent->Client[uiClientCnt].iClntIndex > 0) && (pPvEvent->Client[uiClientCnt].iClntIndex < (MAX_ANZAHL_CLIENTS + 1)))
            {
              pClntListEvnt = pClientListeStart + (pPvEvent->Client[uiClientCnt].iClntIndex - 1);
              
              if (Diagnose.LogPointerInfo == TRUE)
              {
                ValidatePointerServer((UDINT) pClntListEvnt, 71, (MemManagerDiag_type*) &sMemDiag, 1);	/* Erweiterung, zur Überprüfung von Adressen */
              }
              
              /* --- Client f. Variable gefunden -------------------------------------------------------- */
              if (pClntListEvnt->LinkFlag == STATUS_EVENT)
              {
                EventAuswertung.PipeNr = pPvEvent->Client[uiClientCnt].iClntIndex - 1;
                
                /* --- PipeLock[EventAuswertung.PipeNr].Lock = 1: Client gesperrt: nächste Variable,      */
                /* --- welche gesendet werden kann entspricht der letzten, die nicht eingetragen werden konnte  */
                /* --- PipeLock[EventAuswertung.PipeNr].Lock = 0: ungehindertes Eintragen in die Pipe möglich   */
                if((((PipeLock[EventAuswertung.PipeNr].Lock == 1) && 
                   (PipeLock[EventAuswertung.PipeNr].PVIndex == (EventAuswertung.ActPV - 1))) ||
                   (PipeLock[EventAuswertung.PipeNr].Lock == 0)))
                {
                  /* --- -> in Pipe ablegen --------------------------------------------------------------- */
                  SaveIntoPipe = TRUE;
  
                  /* --- nächster Client, selbe PV -------------------------------------------------------- */
                  EventAuswertung.ActClient = uiClientCnt + 1;
                  
                  /* --- ein Client wurde gefunden, falls mehr vorhanden sind, werden diese abgearbeitet -- */
                  *pbClientCheck = TRUE;
                  
                  /* --- Intervallzeit nachstellen, da zu den Clients ein Telegramm gesendet wird --------- */
                  pPvEvent->Tick.Old = udTimeTick;
                  
                  Diagnose.Base.PvCheck[EventAuswertung.ActPV - 1].uiReqPipeEntry++;
                  break;
                }             
              } /* if (pClntListEvnt->LinkFlag == STATUS_EVENT) */
            }
          } /* for (uiClientCnt = EventAuswertung.ActClient; uiClientCnt < MAX_ANZAHL_CLIENTS; uiClientCnt++) */
        } /* if (EventAuswertung.DiffWert > pPvEvent->Hysterese)  */
        /* --- Timer wird überprüft --------------------------------------------- */
        else 
        {
          /* ---  gültiges Intervall ist konfiguriert ------------------------- */
          if (pPvEvent->Tick.Intervall != 0)
          {
            if ((pPvEvent->Tick.Old + (pPvEvent->Tick.Intervall * TIMER_INTERVAL_FKT)) < udTimeTick)
            {
              pPvEvent->Tick.Old = udTimeTick;
              udTickCnt++;
              
              Diagnose.Base.PvCheck[EventAuswertung.ActPV - 1].uiCheckPvTimeOk++;
              *pbClientCheck = FALSE;
              EventAuswertung.ActClient = 0;
              
              /* --- alle Clients für eine Variable untersuchen --------------------------------------------- */
              for (uiClientCnt = EventAuswertung.ActClient; uiClientCnt < MAX_ANZAHL_CLIENTS; uiClientCnt++)
              {
                if ((pPvEvent->Client[uiClientCnt].iClntIndex > 0) && (pPvEvent->Client[uiClientCnt].iClntIndex < (MAX_ANZAHL_CLIENTS + 1)))
                {
                  pClntListEvnt = pClientListeStart +  (pPvEvent->Client[uiClientCnt].iClntIndex - 1);

                  if (Diagnose.LogPointerInfo == TRUE)
                  {
                    ValidatePointerServer((UDINT) pClntListEvnt, 72, (MemManagerDiag_type*) &sMemDiag, 1);	/* Erweiterung, zur Überprüfung von Adressen */
                  }
                  
                  /* --- ein Client wurde für die PV gefunden ------------------------------------ */
                  if (pClntListEvnt->LinkFlag == STATUS_EVENT)
                  {
                    EventAuswertung.PipeNr = pPvEvent->Client[uiClientCnt].iClntIndex - 1;
                    
                    if((((PipeLock[EventAuswertung.PipeNr].Lock == 1) && (PipeLock[EventAuswertung.PipeNr].PVIndex == (EventAuswertung.ActPV - 1))) ||
                       (PipeLock[EventAuswertung.PipeNr].Lock == 0)))
                    {
                      /* --- Eintrag in Pipe ------------------------------------------------ */
                      SaveIntoPipe = TRUE;                    
                      
                      /* --- nächster Client, selbe PV -------------------------------------- */
                      EventAuswertung.ActClient = uiClientCnt + 1;
  
                      /* --- ein Client wurde gefunden, falls restliche vorhanden sind, werden diese abgearbeitet*/
                      *pbClientCheck = TRUE;
                      
                      Diagnose.Base.PvCheck[EventAuswertung.ActPV - 1].uiReqPipeEntry++;
                      break;
                    }
                  }
                }
              } /* for (uiClientCnt = EventAuswertung.ActClient; uiClientCnt < MAX_ANZAHL_CLIENTS; uiClientCnt++) */
            }
          }
        }  /* if (EventAuswertung.DiffWert > pPvEvent->Hysterese)  */       
      } /* if (pPvEvent->pPv != NULL) */
    } 
    /* --- Clients, welche an PV gelinkt sind werden nun durchsucht ---- */
    else if (*pbClientCheck)
    {
      Diagnose.Base.Events.SearchClient++;
      
      *pbClientCheck = FALSE;
      
      /* --- Zeiger auf PV Liste: aktuelle PV -1, da bereits auf nächster PV ----------------------------------- */
      pPvEvent = pPVListeStart + (EventAuswertung.ActPV - 1);
      if (Diagnose.LogPointerInfo == TRUE)
      {
        ValidatePointerServer((UDINT) pPvEvent, 73, (MemManagerDiag_type*) &sMemDiag, 1);	/* Erweiterung, zur Überprüfung von Adressen */
      }

      for (uiClientCnt = EventAuswertung.ActClient; uiClientCnt < MAX_ANZAHL_CLIENTS; uiClientCnt++)
      {
        if ((pPvEvent->Client[uiClientCnt].iClntIndex > 0) && (pPvEvent->Client[uiClientCnt].iClntIndex < (MAX_ANZAHL_CLIENTS+1)))
        {
          pClntListEvnt = pClientListeStart + (pPvEvent->Client[uiClientCnt].iClntIndex - 1);

          if (Diagnose.LogPointerInfo == TRUE)
          {
            ValidatePointerServer((UDINT) pClntListEvnt, 74, (MemManagerDiag_type*) &sMemDiag, 1);	/* Erweiterung, zur Überprüfung von Adressen */
          }

          /* --- Telegrammgenerierung  für weitere Clients  ------------------------------------ */
          if ((pClntListEvnt->LinkFlag == STATUS_EVENT) && (pClntListEvnt->IPAdresse != 0))
          {
            EventAuswertung.PipeNr = pPvEvent->Client[uiClientCnt].iClntIndex - 1;
            
            if(((PipeLock[EventAuswertung.PipeNr].Lock == 1 && (PipeLock[EventAuswertung.PipeNr].PVIndex == (EventAuswertung.ActPV - 1))) ||
              (PipeLock[EventAuswertung.PipeNr].Lock == 0)))
            {             
              EventAuswertung.ActClient = uiClientCnt + 1;
              
              *pbClientCheck = TRUE;
              
              SaveIntoPipe = TRUE;
              
              Diagnose.Base.PvCheck[EventAuswertung.ActPV - 1].uiReqPipeEntry++;
              break;
            }
            else
            {
              Diagnose.Base.PvCheck[EventAuswertung.ActPV - 1].uiPipeLocked++;
              break;
            }
          }
        }
      }

      /* --- kein Client gefunden, mit nächster PV weitermachen --------------------------------------- */
      if (*pbClientCheck == FALSE)
      {
        EventAuswertung.ActClient = 0;
      }
    }
    
    /* --- Anzahl der Clients wurde überschritten -> auf 0 zurücksetzen und nächste PV durchchecken ------ */
    if (EventAuswertung.ActPV + 1 >= MAX_CLIENT_VARIABLEN)
    {
      EventAuswertung.ActClient = 0;
      EventAuswertung.ActPV = 0;
      EventAuswertung.CntPV = 0;
      *pbClientCheck = FALSE;

      /* --- zur Sicherheit wird neu gestartet --------------------------------------------------------- */
      return;
    }

    /* --- Ablegen der gefundenen Werte in die Pipes ----------------------------------------------------- */
    if (SaveIntoPipe)
    {
      SaveIntoPipe = FALSE;
      Diagnose.Base.Events.TryWriteIntoPipe++;
      Diagnose.Base.PvCheck[EventAuswertung.ActPV - 1].uiWriteToPipe++;


      /* --- Daten in Pipe ablegen --------------------------------------------------------------------- */
      PipeData.DataAvailable = 1;
      PipeData.ClientIndex   = EventAuswertung.PipeNr;
      PipeData.PVIndex       = pPvEvent->Client[uiClientCnt].uiClntPvIndex;
      PipeData.Laenge        = pPvEvent->PvLaenge;
      PipeData.Wert          = EventAuswertung.Wert;

      /* --- Eintrag in Pipe --------------------------------------------------------------------------- */
      uiRetValPipe = WriteDataToPipe(PipeData, PipeStartAdress[EventAuswertung.PipeNr], PipeStopAdress[EventAuswertung.PipeNr],
                     &pWritePipe[EventAuswertung.PipeNr], pReadPipe[EventAuswertung.PipeNr]);
      
      /* --- 0 ... erfolgreich; 1 ... kein Platz bzw. Fehler ------------------------------------------ */
      if (uiRetValPipe == 1)
      {
        /* Pipe voll -> bei letzter Variable stehen bleiben, bis in Pipe wieder Platz ist          */
        /* nicht bei Client stehen bleiben, da ansonsten bei Kommunikationsabbruch, alles blockiert ist !*/       
        Diagnose.Base.Events.WriteToPipeFailed++;
        PipeLock[EventAuswertung.PipeNr].Lock = 1;
        PipeLock[EventAuswertung.PipeNr].PVIndex = EventAuswertung.ActPV - 1;
      }
      else
      {
        /* --- letzter Wert wird angepasst ---------------- */
        pPvEvent->PvWert = EventAuswertung.Wert;
        pPvEvent->PvWertCnt++;

        Diagnose.Base.Events.WriteToPipe++;
        PipeLock[EventAuswertung.PipeNr].Lock = 0;
      }
    } /* if (*pbClientCheck == FALSE) */
  } /*  for (uiEventCnt = EventAuswertung.ActPV; uiEventCnt < MAX_CLIENT_VARIABLEN; uiEventCnt++ )*/

}


/*********************************************************************************************
* --- Schrittschaltwerk für Anmeldung 
******************************************************************************************** */

void CyclicSLSConnectToUst()
{
  
  // Stephandler: overhead /initialisierung
  if (StepConnect.StepHandling.Current.bTimeoutElapsed == 1)
  {
    StepConnect.StepHandling.Current.bTimeoutElapsed = 0;
    StepConnect.Step.StepNr = StepConnect.StepHandling.Current.nTimeoutContinueStep;
  }
  StepConnect.StepHandling.Current.nStepNr = (DINT) StepConnect.Step.StepNr;
  brsstrcpy((UDINT) &StepConnect.StepHandling.Current.sStepText, (UDINT) &StepConnect.Step.StepText);
  BrbStepHandler(&StepConnect.StepHandling);
  
  /* ---- Schrittschaltwerk für Initialisierung der Schnittstellen und Anmeldung */
  switch (StepConnect.Step.StepNr)
  {
    
    case UDP_SCHRITT_INIT:
      brsstrcpy((UDINT) &StepConnect.Step.StepText, (UDINT) "UDP_SCHRITT_INIT");	// stephandling: copy actual step
      WriteStepDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration",  "UDP_SCHRITT_INIT", StepConnect.Step.StepNr, udTimeTick);

      StepConnect.Step.InitDone = TRUE;
      StepConnect.Step.StepNr = UDP_SCHRITT_OPEN;
      break;
    
    case UDP_SCHRITT_OPEN:
      brsstrcpy((UDINT) &StepConnect.Step.StepText, (UDINT) "UDP_SCHRITT_OPEN");	// stephandling: copy actual step
      WriteStepDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration",  "UDP_SCHRITT_OPEN", StepConnect.Step.StepNr, udTimeTick);

      /*  ---------------- alle Ports werden solange aufgerufen, bis Status 0 ist -------------------- */
      for (iPortNr = 0; iPortNr < PORT_CNT; iPortNr++) 
      {
        if (UDPopenFub[iPortNr].status != FUB_OK)
        {
          UdpOpen(&UDPopenFub[iPortNr]);
          OpenUDPCnt++;   
        }
      }
      
      if (OpenUDPCnt == 0)
      {
        StepConnect.Step.StepNr  = UDP_SCHRITT_OPTIONS_1_SET;
      }
      
      OpenUDPCnt = 0;     
      break;
      
    case UDP_SCHRITT_OPTIONS_1_SET:
      brsstrcpy((UDINT) &StepConnect.Step.StepText, (UDINT) "UDP_SCHRITT_OPTIONS_1_SET");	// stephandling: copy actual step
      WriteStepDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration",  "UDP_SCHRITT_OPTIONS_1_SET", StepConnect.Step.StepNr, udTimeTick);
      
      for (uiSetOptions = 0; uiSetOptions < PORT_CNT; uiSetOptions++)
      {
        UdpIoctlFub[uiSetOptions].enable  = TRUE;
        UdpIoctlFub[uiSetOptions].ident   = UDPopenFub[uiSetOptions].ident;
        UdpIoctlFub[uiSetOptions].ioctl   = udpSO_SNDBUF_SET;
        UdpIoctlFub[uiSetOptions].pData   = (UDINT) &udIoCtlParamSendBuf;
        UdpIoctlFub[uiSetOptions].datalen = sizeof(udIoCtlParamSendBuf);
        UdpIoctl(&UdpIoctlFub[uiSetOptions]);
      }
      StepConnect.Step.StepNr = UDP_SCHRITT_OPTIONS_1;
      /* -> kein Break notwendig !*/

    case UDP_SCHRITT_OPTIONS_1:
      brsstrcpy((UDINT) &StepConnect.Step.StepText, (UDINT) "UDP_SCHRITT_OPTIONS_1");	// stephandling: copy actual step
      WriteStepDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration",  "UDP_SCHRITT_OPTIONS_1", StepConnect.Step.StepNr, udTimeTick);
    
      for (uiSetOptions = 0; uiSetOptions < PORT_CNT; uiSetOptions++)
      {
        if (UdpIoctlFub[uiSetOptions].status == 0xFFFF)
        {
          UdpIoctl(&UdpIoctlFub[uiSetOptions]);
          uiSetOptionsCnt++;
        }
      }
      
      if (uiSetOptionsCnt == 0)
      {
        StepConnect.Step.StepNr = UDP_SCHRITT_OPTIONS_2_SET;
      }
      uiSetOptionsCnt = 0;
      break;
      
    case UDP_SCHRITT_OPTIONS_2_SET:
      brsstrcpy((UDINT) &StepConnect.Step.StepText, (UDINT) "UDP_SCHRITT_OPTIONS_2_SET");	// stephandling: copy actual step
      WriteStepDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration",  "UDP_SCHRITT_OPTIONS_2_SET", StepConnect.Step.StepNr, udTimeTick);
    
      for (uiSetOptions = 0; uiSetOptions < PORT_CNT; uiSetOptions++)
      {
        UdpIoctlFub[uiSetOptions].enable  = TRUE;
        UdpIoctlFub[uiSetOptions].ident   = UDPopenFub[uiSetOptions].ident;
        UdpIoctlFub[uiSetOptions].ioctl   = udpSO_RCVBUF_SET;
        UdpIoctlFub[uiSetOptions].pData   = (UDINT) &udIoCtlParamRecvBuf;
        UdpIoctlFub[uiSetOptions].datalen = sizeof(udIoCtlParamRecvBuf);
        UdpIoctl(&UdpIoctlFub[uiSetOptions]);
      }
      StepConnect.Step.StepNr = UDP_SCHRITT_OPTIONS_2;
      /* -> kein Break notwendig !*/

    case UDP_SCHRITT_OPTIONS_2:
      brsstrcpy((UDINT) &StepConnect.Step.StepText, (UDINT) "UDP_SCHRITT_OPTIONS_2");	// stephandling: copy actual step
      WriteStepDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration",  "UDP_SCHRITT_OPTIONS_2", StepConnect.Step.StepNr, udTimeTick);
    
      for (uiSetOptions = 0; uiSetOptions < PORT_CNT; uiSetOptions++)
      {
        if (UdpIoctlFub[uiSetOptions].status == 0xFFFF)
        {
          UdpIoctl(&UdpIoctlFub[uiSetOptions]);
          uiSetOptionsCnt++;
        }
      }
      
      if (uiSetOptionsCnt == 0)
      {
        /* --- Schrittschaltwerke initialisieren ------------------------------- */
        StepConnect.Step.StepNr    = UDP_SCHRITT_RECEIVE_SET;
        UDPLifeCheck  = LIFE_CHECK_RESPONSE;
        StepEventRecv.Step.StepNr = UDP_EVENT_RECEIVE_INIT;
        StepEventSend.Step.StepNr  = UDP_EVENT_SEND_INIT;
          
        /* ----- Vorbelegung Server - Fubs: Lifecheck - Empfang ---------------- */
        UDPreceiveLiveFub.enable = TRUE;
        UDPreceiveLiveFub.ident  = UDPopenFub[5].ident;
        UDPreceiveLiveFub.pData  = (UDINT) &arRecvBufLife;
        UDPreceiveLiveFub.datamax = sizeof(arRecvBufLife);
        UDPreceiveLiveFub.pIpAddr = (UDINT) &arRecvLifeIpAdr;
      }
      uiSetOptionsCnt = 0;

      break;

    case UDP_SCHRITT_RECEIVE_SET:
      brsstrcpy((UDINT) &StepConnect.Step.StepText, (UDINT) "UDP_SCHRITT_RECEIVE_SET");	// stephandling: copy actual step
      WriteStepDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration",  "UDP_SCHRITT_RECEIVE_SET", StepConnect.Step.StepNr, udTimeTick);
    
      UDPreceiveFub.enable  = TRUE;
      UDPreceiveFub.ident   = UDPopenFub[0].ident;
      UDPreceiveFub.pData	= (UDINT) pRecvBufOpen;
      UDPreceiveFub.datamax = MAX_UDP_FRAME_LEN;
      UDPreceiveFub.pIpAddr = (UDINT) &arRecvOpenIpAdr;      
      StepConnect.Step.StepNr = UDP_SCHRITT_RECEIVE;
      /* --- hier kein Break  -> Auswertung sofort machen*/

    case UDP_SCHRITT_RECEIVE:
      brsstrcpy((UDINT) &StepConnect.Step.StepText, (UDINT) "UDP_SCHRITT_RECEIVE");	// stephandling: copy actual step
      WriteStepDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration",  "UDP_SCHRITT_RECEIVE", StepConnect.Step.StepNr, udTimeTick);
    
	    UdpRecv(&UDPreceiveFub);
      if (UDPreceiveFub.status == FUB_OK)
      {             
        CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "data from adress", (char*) UDPreceiveFub.pIpAddr, UDPreceiveFub.recvlen, 999, udTimeTick);
        CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
			
        /*  --- empfangenes Telegramm darf nicht von dieser Station sein --------------------------------- */
        sdCmpRes = brsstrcmp((UDINT) UDPreceiveFub.pIpAddr, (UDINT) &arIpAdrSource);
        if (sdCmpRes != 0)
        {
          /* --- Kennung auf Char - Feld umkopieren ---------------------------------------------------- */
          brsmemcpy((UDINT) &arKennung, (UDINT) pRecvBufOpen, 4);
          
          /* --- ClientConnectRequest ------------------------------------------------------------------ */
          if ((CheckTelegramHeader((UDINT*) &arKennung[0], (STRING*) TELEGRAMM_IDENT_CONX) == TRUE) || /* Kennung "CONX" -> neuer Modus */
              (CheckTelegramHeader((UDINT*) &arKennung[0], (STRING*) TELEGRAMM_IDENT_CONN) == TRUE))   /* Kennung "CONN" -> alter Modus */ 
          {
            /* --------------------- Alignment - Intel oder Motorola --------------------------------- */
            CopyReceiveBufferToClientConnectRequest((UDINT*) pRecvBufOpen, &sClientConnectRequest);            
            Diagnose.Base.RegisterPv.ConnRequest++;
            
            CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "CONX or CONN received ", (char*) UDPreceiveFub.pIpAddr, UDPreceiveFub.recvlen, 999, udTimeTick);
            CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
            
            /* --- Cyclic Redundancy Check  */          
            CRCCheckTemp = CRCcheck((UDINT*) pRecvBufOpen, 8);
            if (CRCCheckTemp == sClientConnectRequest.BCC)
            {
              CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "CRC ok ", (char*) UDPreceiveFub.pIpAddr, CRCCheckTemp, 999, udTimeTick);
              CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
              
              /* ------------------------ Client suchen -------------------------------------------- */             
              for (iClient = 0; iClient < MAX_ANZAHL_CLIENTS; iClient++)
              {
                pClientListeWork = (ClientListeEntry_typ*) pClientListeStart + iClient;
                if (Diagnose.LogPointerInfo == TRUE)
                {
                  ValidatePointerServer((UDINT) pClientListeWork, 10, (MemManagerDiag_type*) &sMemDiag, 1);	/* Erweiterung, zur Überprüfung von Adressen */
                }
                
                /* --- Client bereits vorhanden -> Überschreiben der alten PVs  */
                sdCmpRes = brsstrcmp((UDINT) &pClientListeWork->IpAdr, (UDINT) UDPreceiveFub.pIpAddr);
                if (sdCmpRes == 0) 
                { 
                  /* --------------------- Abmelden der Variablen ------------------------------ */
                  ClientVerwaltungEinfuegen = STATUS_CONNECT_REQUEST_RECONNECT;
                  ClientVerwaltungAktuell   = iClient + 1;  
                  
                  CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "client found in list: ", (char*) UDPreceiveFub.pIpAddr, ClientVerwaltungAktuell, 999, udTimeTick);
                  CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
                  break;    
                }
                else 
                { 
                  /* --- erste freie Speicherstelle merken -------------------------------------- */
                  if (pClientListeWork->IpAdr[0] == 0)
                  {
                    if (ClientVerwaltungEinfuegen == STATUS_CONNECT_REQUEST_INIT)
                    {
                      /* --- Client darf nicht 0 sein! -------------------------------------- */
                      ClientVerwaltungEinfuegen = STATUS_CONNECT_REQUEST_ENTRY;
                      ClientVerwaltungAktuell   = iClient + 1;
                    }
                  }
                }
              } /*  for (iClient = 0; iClient < MAX_ANZAHL_CLIENTS; iClient++)*/
               
              /* ------------------ Client eintragen bzw. Neuanmeldung initialisieren -------------- */
              if ((ClientVerwaltungEinfuegen == STATUS_CONNECT_REQUEST_RECONNECT) ||  /* Client bereits vorhanden   */
                    (ClientVerwaltungEinfuegen == STATUS_CONNECT_REQUEST_ENTRY))      /* Neuanmeldung */  
              {
                CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "DeleteExistingClient", "check for client index: ", ClientVerwaltungAktuell, 999, udTimeTick);
                CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
                
                if (ClientVerwaltungEinfuegen == STATUS_CONNECT_REQUEST_RECONNECT)
                { 
                  /* --- Funktion: Löschen der PV's bzw. der Clienteinträge einer Variable ----- */
                  DeleteClientStatus = DeleteExistingClient(ClientVerwaltungAktuell);
									
                  /*  --- mitloggen für Diagnosezwecke ------------------------------------------------------ */
                  if (DeleteClientStatus == 0)
                  {
                    CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "DeleteExistingClient", "nothing found, exit ", ClientVerwaltungAktuell, 999, udTimeTick);
                    CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
                  }
                  else if (DeleteClientStatus == 1)
                  {
                    CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "DeleteExistingClient", "remove client reference from variable(s) ", ClientVerwaltungAktuell, 999, udTimeTick);
                    CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
                  }
                  else if (DeleteClientStatus == 2)
                  {
                    CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "DeleteExistingClient", "delete variable(s) from list ", ClientVerwaltungAktuell, 999, udTimeTick);
                    CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
                  }
                  else if (DeleteClientStatus == 3)
                  {
                    CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "DeleteExistingClient", "client ref (1-100) out of range ", ClientVerwaltungAktuell, 999, udTimeTick);
                    CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
                  }
                  else
                  {
                    CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "DeleteExistingClient", "ELSE ", DeleteClientStatus, 999, udTimeTick);
                    CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
                  }
                }
                else
                {
                  CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "new client added:", (char*) UDPreceiveFub.pIpAddr, ClientVerwaltungAktuell, 999, udTimeTick);
                  CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
                }
                
                /* --------------- ClientDaten neu einfügen --------------------------------------- */
                pClientListeWork = pClientListeStart + ClientVerwaltungAktuell - 1;   // Server Index zum Client zurück
                if (Diagnose.LogPointerInfo == TRUE)
                {
                  ValidatePointerServer((UDINT) pClientListeWork, 11, (MemManagerDiag_type*) &sMemDiag, 1);	/* Erweiterung, zur Überprüfung von Adressen */
                }

                /* --------------- Client wird als angemeldet eingetragen  ------------------------ */
                ConvertAsciiToIpAdr(&pClientListeWork->IPAdresse, (UDINT*) UDPreceiveFub.pIpAddr);  /* Ip Adresse numerisch eintragen*/
                brsstrcpy((UDINT) &pClientListeWork->IpAdr, (UDINT) UDPreceiveFub.pIpAddr);
                pClientListeWork->Timeout     =   sClientConnectRequest.Timeout;
                pClientListeWork->LinkFlag    =   STATUS_OPEN;
                pClientListeWork->ClientIndex =   sClientConnectRequest.ClientIndex;                
                pClientListeWork->Modus = SetMode((UDINT*) &arKennung);  /* Unterscheidung alter und neuer Modus über Telegrammkennung */
                
                Telegrammstatus = ERR_NO;
              }
              else
              {
                Telegrammstatus = ERR_CONN_FAILURE;
              }
  
              /* ---  Response Telegramm generieren ---------------------------------------------------------- */
              GenerateConnHeaderTelegramm((UDINT*) &sClientConnectResponse.Kennung, pClientListeWork->Modus);
              
              /* --- im ClientIndex wird die Verwaltungsnummer des Clients im Server zurückgesendet ---------- */
              sClientConnectResponse.ClientIndex  = ClientVerwaltungAktuell;
              sClientConnectResponse.Status   = Telegrammstatus;
              
              /* --- auf Sendebuffer umkopieren -------------------------------------------------------------- */
              if (Diagnose.LogPointerInfo == TRUE)
              {
                ValidatePointerServer((UDINT) pSendBufOpen, 12, (MemManagerDiag_type*) &sMemDiag, 1);	/* Erweiterung, zur Überprüfung von Adressen */
              }
              
              CopyClientConnectResponseToSendBuffer((UDINT*) pSendBufOpen, &sClientConnectResponse);
              
              /* --- Senden der Daten zu Client -------------------------------------------------------------- */
              UDPsendFub.pHost   = UDPreceiveFub.pIpAddr;  /* zurück zum Absender*/
              UDPsendFub.enable  = TRUE;
              UDPsendFub.ident   = UDPopenFub[1].ident;
              UDPsendFub.pData   = (UDINT)pSendBufOpen;
              UDPsendFub.datalen = sizeof(sClientConnectResponse);
              UDPsendFub.port    = PortTarget[1];
              UDPsendFub.flags   = udpSendFlags;
              UdpSend(&UDPsendFub);
              
              StepConnect.Step.StepNr = UDP_SCHRITT_SEND_CONN;
              
              Diagnose.Base.RegisterPv.ConnResponse++;                             
            }
            
            ClientVerwaltungEinfuegen = STATUS_CONNECT_REQUEST_INIT;
          }
          /* -------------- PVOpenRequest behandeln - Anmeldung der Variablen ALT--------------- */
          else if (CheckTelegramHeader((UDINT*) &arKennung[0], (STRING*) TELEGRAMM_IDENT_OPEN) == TRUE) // alter MODUS
          {
            CopyReceiveBufferToPVOpenRequest((UDINT*) pRecvBufOpen, &sPVOpenRequest); 
            Diagnose.Base.RegisterPv.OpenRequest++;            
            
            pClientListeWork = pClientListeStart + sPVOpenRequest.ClientIndex - 1;
            
            sdCmpRes = brsstrcmp((UDINT) &pClientListeWork->IpAdr, (UDINT) UDPreceiveFub.pIpAddr);
            if (sdCmpRes != 0)
            {
              Diagnose.Base.RegisterPv.OpenRequestIpFailure++;
            }
            if (sdCmpRes == 0)
            {
              /* --- in Anmeldephase ---------------------------------------------------- */
              if (pClientListeWork->LinkFlag == STATUS_OPEN)  
              {                
                brsmemcpy((UDINT) &sPVOpenRequestX.Variablenname, (UDINT) &sPVOpenRequest.Variablenname, PV_LENGTH);
                sPVOpenRequestX.ClientIndex = sPVOpenRequest.ClientIndex;
                sPVOpenRequestX.Hysterese = sPVOpenRequest.Hysterese;
                sPVOpenRequestX.Laenge = sPVOpenRequest.Laenge;
                sPVOpenRequestX.PVIndex = sPVOpenRequest.PVIndex;
                sPVOpenRequestX.SyncTime = sPVOpenRequest.SyncTime;
                
                /* --- Variable existiert -------------------------------------- */
                uiState = PV_xgetadr((char*) &sPVOpenRequestX.Variablenname, &pPvAdrServer, &PvLaengeServer);
                if (Diagnose.LogPointerInfo == TRUE)
                {
                  ValidatePointerServer((UDINT) pClientListeWork, 15, (MemManagerDiag_type*) &sMemDiag, 1);	/* Erweiterung, zur Überprüfung von Adressen */
                }
                            
                if (uiState == FUB_OK)
                {                         
                  Diagnose.Base.RegisterPv.OpenRequestOk++;
                            
                  /* --- Platz in Liste suchen ------------------------------- */
                  uiPvExists = ExistsPvInListNew((UDINT) &sPVOpenRequestX.Variablenname, sPVOpenRequestX.ClientIndex, &uiPvOffset, &uiClntOffset);
                  //uiPvExists = ExistsPvInList(pPvAdrServer, sPVOpenRequestX.ClientIndex, &uiPvOffset, &uiClntOffset);
                  switch (uiPvExists)
                  {
                    case 1: /* --- Variable eintragen */
                      {
                        /* --- Variable in Verwaltung einhängen ---------------------------------------- */
                        InsertNewPvInList(sPVOpenRequestX, pPvAdrServer, PvLaengeServer, uiPvOffset, uiClntOffset, &udVal, TRUE);
                        /* Antwortstruktur ausfüllen */
                        sPVOpenResponse.ClientIndex  =   pClientListeWork->ClientIndex;
                        sPVOpenResponse.PVIndex      =   sPVOpenRequest.PVIndex;
                        sPVOpenResponse.Laenge       =   PvLaengeServer;
                        sPVOpenResponse.PVIdent      =   pPvAdrServer;
                        sPVOpenResponse.Wert         =   udVal;
                        sPVOpenResponse.Status       =   STATUS_SERV_OK;
                        break;
                      }
                                
                    case 2: /* --- Client für vorhandene Variable eintragen */
                      {
                        /* --- Variable in Verwaltung einhängen ---------------------------------------- */
                        InsertNewPvInList(sPVOpenRequestX, pPvAdrServer, PvLaengeServer, uiPvOffset, uiClntOffset, &udVal, FALSE);
                        /* Antwortstruktur ausfüllen */
                        sPVOpenResponse.ClientIndex  =   pClientListeWork->ClientIndex;
                        sPVOpenResponse.PVIndex      =   sPVOpenRequest.PVIndex;
                        sPVOpenResponse.Laenge       =   PvLaengeServer;
                        sPVOpenResponse.PVIdent      =   pPvAdrServer;
                        sPVOpenResponse.Wert         =   udVal;
                        sPVOpenResponse.Status       =   STATUS_SERV_OK;                      
                        break;
                      }
                                
                    case 3: /* --- kein Platz in Variablenliste */
                      {
                        /* Antwortstruktur ausfüllen */
                        sPVOpenResponse.ClientIndex  =   pClientListeWork->ClientIndex;
                        sPVOpenResponse.PVIndex      =   sPVOpenRequest.PVIndex;
                        sPVOpenResponse.Laenge       =   0;
                        sPVOpenResponse.PVIdent      =   NULL_ADR;
                        sPVOpenResponse.Wert         =   0;
                        sPVOpenResponse.Status       =   STATUS_SERV_NO_PV_SPACE;
                        break;
                      }
                                
                    case 4: /* --- kein Platz in Clientliste der Variable */
                      {
                        /* Antwortstruktur ausfüllen */
                        sPVOpenResponse.ClientIndex  =   pClientListeWork->ClientIndex;
                        sPVOpenResponse.PVIndex      =   sPVOpenRequest.PVIndex;
                        sPVOpenResponse.Laenge       =   0;
                        sPVOpenResponse.PVIdent      =   NULL_ADR;
                        sPVOpenResponse.Wert         =   0;
                        sPVOpenResponse.Status       =   STATUS_SERV_NO_PV_SPACE;
                        break;
                      }
                  } 
                }
                else  /* Variable auf RPS nicht vorhanden: */
                {       
                  /* Antwortstruktur ausfüllen */
                  sPVOpenResponse.ClientIndex  =   pClientListeWork->ClientIndex;
                  sPVOpenResponse.PVIndex      =   sPVOpenRequest.PVIndex;
                  sPVOpenResponse.Laenge       =   0;
                  sPVOpenResponse.PVIdent      =   NULL_ADR;
                  sPVOpenResponse.Wert         =   0;
                  sPVOpenResponse.Status       =   STATUS_SERV_NO_VAR;
                }
                
                
                /* Generierung der Kennung des Antworttelegrammes an den Client */
                brsmemcpy((UDINT) &sPVOpenResponse.Kennung, (UDINT) TELEGRAMM_IDENT_OPEN, 4);
         
                /* Backup kopieren - für Diagnose */
                brsmemcpy((UDINT) &sPVOpenResponseSave, (UDINT) &sPVOpenResponse, sizeof(PVOpenResponse_typ));
                
                /* ----------- UDP Sendestruktur belegen ---------------- */
                CopyPVOpenResponseToSendBuffer(pSendBufOpen, &sPVOpenResponse);
                
                /* --- Senden des Telegrammes ------------------------- */
                UDPsendFub.enable  = TRUE;
                UDPsendFub.ident   = UDPopenFub[1].ident;
                UDPsendFub.pData   = (UDINT) pSendBufOpen;
                UDPsendFub.datalen = sizeof(sPVOpenResponse);
                UDPsendFub.port    = PortTarget[1];
                UDPsendFub.pHost   = UDPreceiveFub.pIpAddr;
                UDPsendFub.flags   = udpSendFlags;
                UdpSend(&UDPsendFub);
                  
                Diagnose.Base.RegisterPv.OpenResponse++;
                  
                /* --- Schrittkette: Telegramm wird gesendet ------------------------------------------------ */
                StepConnect.Step.StepNr = UDP_SCHRITT_SEND_OPEN;
              }
            }
          }
          /* -------------- PVOpenRequest behandeln - Anmeldung der Variablen NEU ---------------- */
          else if (CheckTelegramHeader((UDINT*) &arKennung[0], (STRING*) TELEGRAMM_IDENT_OPEX) == TRUE) // neuer MODUS
          {
            Diagnose.Base.RegisterPvX.OpenRequest++;
            CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "OPEX received", (char*) UDPreceiveFub.pIpAddr, UDPreceiveFub.recvlen, 999, udTimeTick);
            CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
            
            /* --- Ermittlung von Daten aus dem Recv - Buffer */
            /* --- Sind die Daten ok - CRC ---------------------------------------------------- */
            if (Diagnose.LogPointerInfo == TRUE)
            {
              ValidatePointerServer((UDINT) pRecvBufOpen, 13, (MemManagerDiag_type*) &sMemDiag, 1);	/* Erweiterung, zur Überprüfung von Adressen */
            }
                  
            bValidate = ValidateDataFromRecvBuf(pRecvBufOpen, UDPreceiveFub.recvlen);
            if (bValidate == TRUE)
            {              
              if (Diagnose.LogPointerInfo == TRUE)
              {
                ValidatePointerServer((UDINT) pClientListeWork, 14, (MemManagerDiag_type*) &sMemDiag, 1);	/* Erweiterung, zur Überprüfung von Adressen */
              }
	    
              /* --- Überprüfung Client Ip-Adresse mit Adresse von Absender ----------------- */
              sdCmpRes = brsstrcmp((UDINT) &pClientListeWork->IpAdr, (UDINT) UDPreceiveFub.pIpAddr);
              if (sdCmpRes == 0)
              {
                /* --- in Anmeldephase ---------------------------------------------------- */
                if (pClientListeWork->LinkFlag == STATUS_OPEN)  
                {
                  uiPvEntry = 0;
                  pPvAdrServer = 0;
                  PvLaengeServer = 0;
                  udVal = 0;
                  uiOpenTelLen = 0;
                  uiClientIndex = 0;
                  
                  CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "UDP_SCHRITT_RECEIVE", "get variables from client", 0, 999, udTimeTick);
                  CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
                  
                  /* --- Anzahl der Variablen und ClientIndex ermitteln ----------------- */
                  GetMainDataFromRecvBufOpen(pRecvBufOpen, &uiPvCnt, &uiClientIndex);
                  
                
                  CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "UDP_SCHRITT_RECEIVE", "variables from client available - cnt", uiPvCnt, 999, udTimeTick);
                  CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
                  
                  /* --- Sendebuffer vorbelegen ----------------------------------------- */
                  CreateHeaderInRespBuf((USINT*) pSendBufOpen, uiPvCnt, uiClientIndex, &uiOpenTelLen);

                  do
                  {
                    /* --- nächstes Variablenpacket aus Empfangsbuffer holen ----------- */
                    bGetData = GetDataFromRecvBufOpen(pRecvBufOpen, uiPvCnt, &sPVOpenRequestX, &uiPvEntry);
                    if (bGetData == TRUE)
                    {
                      /* --- Variable existiert -------------------------------------- */
                      uiState = PV_xgetadr((char*) &sPVOpenRequestX.Variablenname, &pPvAdrServer, &PvLaengeServer);
                      if (Diagnose.LogPointerInfo == TRUE)
                      {
                        ValidatePointerServer((UDINT) pClientListeWork, 15, (MemManagerDiag_type*) &sMemDiag, 1);	/* Erweiterung, zur Überprüfung von Adressen */
                      }
                            
                      if (uiState == FUB_OK)
                      {                         
                        Diagnose.Base.RegisterPvX.OpenRequestOk++;
                            
                        CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "Variable Ident ok", (char*) &sPVOpenRequestX.Variablenname, 0, StepConnect.Step.StepNr, udTimeTick);
                        CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
                  
                        /* --- Platz in Liste suchen ------------------------------- */
                        uiPvExists = ExistsPvInList(pPvAdrServer, sPVOpenRequestX.ClientIndex, &uiPvOffset, &uiClntOffset);
                        switch (uiPvExists)
                        {
                          case 1: /* --- Variable eintragen */
                            {
                              CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "Variable Added to list", (char*) &sPVOpenRequestX.Variablenname, 1, StepConnect.Step.StepNr, udTimeTick);
                              CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
                            
                              /* --- Variable in Verwaltung einhängen ---------------------------------------- */
                              InsertNewPvInList(sPVOpenRequestX, pPvAdrServer, PvLaengeServer, uiPvOffset, uiClntOffset, &udVal, TRUE);
                            
                              CmdBurLogBook.FubState = WriteRegDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", sPVOpenRequestX.PVIndex, sPVOpenRequestX.ClientIndex, StepConnect.Step.StepNr, udTimeTick);
                              CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
                            
                              /* --- Antwort in Buffer eintragen --------------------------------------------- */
                              CreateNewEntryInRespBuf(pSendBufOpen, sPVOpenRequestX, (uiPvEntry - 1), pPvAdrServer, PvLaengeServer, udVal, STATUS_SERV_OK, &uiOpenTelLen);
                              break;
                            }
                                
                          case 2: /* --- Client für vorhandene Variable eintragen */
                            {
                              CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "Variable Exists in list", (char*) &sPVOpenRequestX.Variablenname, 2, StepConnect.Step.StepNr, udTimeTick);
                              CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
                            
                              /* --- Variable in Verwaltung einhängen ---------------------------------------- */
                              InsertNewPvInList(sPVOpenRequestX, pPvAdrServer, PvLaengeServer, uiPvOffset, uiClntOffset, &udVal, FALSE);

                              CmdBurLogBook.FubState = WriteRegDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", sPVOpenRequestX.PVIndex, sPVOpenRequestX.ClientIndex, StepConnect.Step.StepNr, udTimeTick);
                              CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
                            
                              /* --- Antwort in Buffer eintragen --------------------------------------------- */
                              CreateNewEntryInRespBuf(pSendBufOpen, sPVOpenRequestX, (uiPvEntry - 1), pPvAdrServer, PvLaengeServer, udVal, STATUS_SERV_OK, &uiOpenTelLen);
                              break;
                            }
                                
                          case 3: /* --- kein Platz in Variablenliste */
                            {
                            
                              CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "Variable: no place", (char*) &sPVOpenRequestX.Variablenname, 3, StepConnect.Step.StepNr, udTimeTick);
                              CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
                            
                              /* --- Antwort in Buffer eintragen --------------------------------------------- */
                              CreateNewEntryInRespBuf(pSendBufOpen, sPVOpenRequestX, (uiPvEntry - 1), pPvAdrServer, PvLaengeServer, udVal, STATUS_SERV_NO_PV_SPACE, &uiOpenTelLen);
                              break;
                            }
                                
                          case 4: /* --- kein Platz in Clientliste der Variable */
                            {
                              CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "ClientList: no place", (char*) &sPVOpenRequestX.Variablenname, 4, StepConnect.Step.StepNr, udTimeTick);
                              CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
                            
                              /* --- Antwort in Buffer eintragen --------------------------------------------- */
                              CreateNewEntryInRespBuf(pSendBufOpen, sPVOpenRequestX, (uiPvEntry - 1), pPvAdrServer, PvLaengeServer, udVal, STATUS_SERV_NO_CLIENT_SPACE, &uiOpenTelLen);                             
                              break;
                            }
                        } 
                      }
                      else  /* Variable auf RPS nicht vorhanden: */
                      {                
                        CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "Variable ident error", (char*) &sPVOpenRequestX.Variablenname, uiState, StepConnect.Step.StepNr, udTimeTick);
                        CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
                        
                        /* --- Antwort in Buffer eintragen --------------------------------------------- */
                        CreateNewEntryInRespBuf(pSendBufOpen, sPVOpenRequestX, (uiPvEntry - 1), pPvAdrServer, PvLaengeServer, udVal, STATUS_SERV_NO_VAR, &uiOpenTelLen);
                      }
                    }
                  } while (bGetData == TRUE);
                  
                  /* --- Abschluss, CRC für Telegramm erstellen ----------------------------------------------- */                  
                  CreateCrcInRespBuf(pSendBufOpen, &uiOpenTelLen);
                  
                  /* --- Senden des Telegrammes */
                  UDPsendFub.enable  = TRUE;
                  UDPsendFub.ident   = UDPopenFub[1].ident;
                  UDPsendFub.pData   = (UDINT) pSendBufOpen;
                  UDPsendFub.datalen = uiOpenTelLen;
                  UDPsendFub.port    = PortTarget[1];
                  UDPsendFub.pHost   = UDPreceiveFub.pIpAddr;
                  UDPsendFub.flags   = udpSendFlags;
                  UdpSend(&UDPsendFub);
                  
                  Diagnose.Base.RegisterPvX.OpenResponse++;
                  CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "OPEX send back", (char*) UDPreceiveFub.pIpAddr, PortTarget[1], 999, udTimeTick);
                  CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
                  
                  /* --- Schrittkette: Telegramm wird gesendet ------------------------------------------------ */
                  StepConnect.Step.StepNr = UDP_SCHRITT_SEND_OPEN;
                }
              }
              else
              {
                CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "OPEX/OPEN - wrong station", (char*) UDPreceiveFub.pIpAddr, sdCmpRes, 999, udTimeTick);
                CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
                /* --- die aktuell zu bearbeitende Station entspricht nicht der zu bearbeitenden Station */
                /* --- Daten werden nicht bearbeitet*/
                Diagnose.Base.RegisterPvX.OpenRequestIpFailure++;
              }
            }          
          }
            /*########################################################################################################*/
          else if ((CheckTelegramHeader((UDINT*) &arKennung[0], (STRING*) TELEGRAMM_IDENT_LINX) == TRUE) || /* Kennung "LINX" -> neuer Modus */
                    (CheckTelegramHeader((UDINT*) &arKennung[0], (STRING*) TELEGRAMM_IDENT_LINK)))           /* Kennung "LINK" -> alter Modus */
          {
            CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "LINX received", (char*) UDPreceiveFub.pIpAddr, UDPreceiveFub.recvlen, 999, udTimeTick);
            CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
            
            /* --- Ip Adressen vergleichen ... */
            sdCmpRes = brsstrcmp((UDINT) &pClientListeWork->IpAdr, (UDINT) UDPreceiveFub.pIpAddr);
            if (sdCmpRes == 0)
            {
              brsmemcpy((UDINT) &sClientLinkSave, (UDINT) &sClientLink, sizeof(ClientLink_typ));
              CopyReceiveBufferToClientLink((UDINT*) pRecvBufOpen, &sClientLink);

              Diagnose.Base.RegisterPvX.Link++;
              
              /*  ---------------------- Eintrag in Client - Liste -------------------------------------- */  
              pClientListeWork = pClientListeStart + sClientLink.ClientIndex - 1;

              if (Diagnose.LogPointerInfo == TRUE)
              {
                ValidatePointerServer((UDINT) pClientListeWork, 17, (MemManagerDiag_type*) &sMemDiag, 1);	/* Erweiterung, zur Überprüfung von Adressen */
              }
            
              /* ------------ Linkstatus auf Event - Auswertung: Ab jetzt werden die PVs abgecheckt ----- */
              pClientListeWork->LinkFlag = STATUS_EVENT;
  
              /* ----------- Initialisierung des Zeitstempels ------------------------------------------- */
              pClientListeWork->LastTimeStamp = udTimeTick;
               
              /* Generierung des Antworttelegrammes an den Client */
              brsmemcpy((UDINT) &sNAK.Kennung, (UDINT) TELEGRAMM_IDENT_ACKN, 4);

              sNAK.Status =  0;
  
              /* UDP Struktur belegen*/
              brsmemcpy((UDINT) &sNAKSave, (UDINT) &sNAK, sizeof(AckResponse_typ));
               
              CopyNAKToSendBuffer((UDINT*) pSendBufOpen, &sNAK);
               
              UDPsendFub.enable 	= TRUE;
              UDPsendFub.ident  	= UDPopenFub[1].ident;
              UDPsendFub.pData    = (UDINT) pSendBufOpen;
              UDPsendFub.datalen  = sizeof(sNAK);
              UDPsendFub.port   	= PortTarget[1];
              UDPsendFub.pHost  	= UDPreceiveFub.pIpAddr;  /* zurück zum Absender*/
              UDPsendFub.flags  	= udpSendFlags;
            
              UdpSend(&UDPsendFub);
              StepConnect.Step.StepNr = UDP_SCHRITT_SEND_LINK_ACK;
            }
            else
            {
              /* Telegramm von der falschen Station -> hier wird nichts gemacht */							
              CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "wrong station", (char*) UDPreceiveFub.pIpAddr, 0, StepConnect.Step.StepNr, udTimeTick);
              CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;

            }
          }
        }
      }     
      break;
       
        
    /*########################################################################################################*/
    /*  Senden des ClientConnectResponse Telegrammes                                */
    /*  bei scheitern nach n Aufrufen wird mit nächster Variable weitergemacht                  */
    case UDP_SCHRITT_SEND_CONN:
      brsstrcpy((UDINT) &StepConnect.Step.StepText, (UDINT) "UDP_SCHRITT_SEND_CONN");	// stephandling: copy actual step
      WriteStepDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration",  "UDP_SCHRITT_SEND_CONN", StepConnect.Step.StepNr, udTimeTick);
    
      if (UDPsendFub.status != ERR_FUB_BUSY)
      { 
        if (UDPsendFub.status == FUB_OK)
        {     
          Diagnose.Base.RegisterPv.ConnSend++;
          Diagnose.Base.RegisterPvX.ConnSend++;
        }
        else
        {
          CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "err send conn", (char*) UDPsendFub.pHost, UDPsendFub.status, StepConnect.Step.StepNr, udTimeTick);
          CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
          
          Diagnose.Base.RegisterPv.ConnSendFailure++;
          Diagnose.Base.RegisterPvX.ConnSendFailure++;
        }

        StepConnect.Step.StepNr = UDP_SCHRITT_RECEIVE_SET;
      } 
      else
      {
        UdpSend(&UDPsendFub);
      }
      break;
    
    /*########################################################################################################*/
    /* Antworttelegramm für PV Anmeldung*/
    case UDP_SCHRITT_SEND_OPEN:
      brsstrcpy((UDINT) &StepConnect.Step.StepText, (UDINT) "UDP_SCHRITT_SEND_OPEN");	// stephandling: copy actual step
      WriteStepDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration",  "UDP_SCHRITT_SEND_OPEN", StepConnect.Step.StepNr, udTimeTick);
    
      if (UDPsendFub.status != ERR_FUB_BUSY)
      {
	      if (UDPsendFub.status == FUB_OK)
	      {
          Diagnose.Base.RegisterPv.OpenSend++;
          Diagnose.Base.RegisterPvX.OpenSend++;
	      }
	      else
	      {
	        /* Fehler: Telegramm konnte nicht an Client gesendet werden */
          CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "failed to send data", (char*) UDPsendFub.pHost, 0, StepConnect.Step.StepNr, udTimeTick);
          CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
          
          Diagnose.Base.RegisterPv.OpenSendFailure++;
          Diagnose.Base.RegisterPvX.OpenSendFailure++;
	      }
	      StepConnect.Step.StepNr = UDP_SCHRITT_RECEIVE_SET;
      }
      else
      {
        UdpSend(&UDPsendFub);
      }
      break;

    /*########################################################################################################*/
    /* Bestätigung des Link - Telegrammes                                     */
    case UDP_SCHRITT_SEND_LINK_ACK:
      brsstrcpy((UDINT) &StepConnect.Step.StepText, (UDINT) "UDP_SCHRITT_SEND_LINK_ACK");	// stephandling: copy actual step
      WriteStepDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration",  "UDP_SCHRITT_SEND_LINK_ACK", StepConnect.Step.StepNr, udTimeTick);
    
      if (UDPsendFub.status != ERR_FUB_BUSY)
      {
        if (UDPsendFub.status == FUB_OK)
        {
          CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "Response link data", (char*) UDPsendFub.pHost, UDPsendFub.sentlen, StepConnect.Step.StepNr, udTimeTick);
          CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
        }
        else
        {
          if ((UDPsendFub.status != ERR_FUB_BUSY) && (UDPsendFub.status != udpERR_NO_DATA))
          {
            CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "failed to send link data", (char*) UDPsendFub.pHost, UDPsendFub.status, StepConnect.Step.StepNr, udTimeTick);
            CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
          }
        }
        StepConnect.Step.StepNr = UDP_SCHRITT_RECEIVE_SET;
      }
      else 
      {
        UdpSend(&UDPsendFub);
      }
      break;
    
  } /* --- Ende - Schrittschaltwerk f. Anmeldung     */
  
}


/***********************************************************************************************************
* --- Ermittlung der Belegung von Pipes
********************************************************************************************************** */

void CyclicGetPipeInfo()
{
	if (Diagnose.Base.bCmdGetPipeOccupied == TRUE)
	{
	  /* --- Pipes durchsuchen ------------------------------------------------------------------------------- */
      for (iGetPipeCnt = 0; iGetPipeCnt < MAX_ANZAHL_CLIENTS; iGetPipeCnt++ ) 
      {
      	pClientListeEventSend = pClientListeStart + iGetPipeCnt;
      	if (pClientListeEventSend->LinkFlag == STATUS_EVENT)
        {
        	Diagnose.Base.PipeOccupied[iGetPipeCnt] = GetPipeInfo(iGetPipeCnt);
        }
        else
        {
	        Diagnose.Base.PipeOccupied[iGetPipeCnt] = 0;
        }
      }
	}
}


/***********************************************************************************************************
* --- Telegramme senden
* -> aus Pipes auslesen 
* -> Telegramme generieren 
* -> Antworten von clients abwarten 
********************************************************************************************************** */

void CyclicSLSEventsSend()
{

  // Stephandler: overhead /initialisierung
  if (StepEventSend.StepHandling.Current.bTimeoutElapsed == 1)
  {
    StepEventSend.StepHandling.Current.bTimeoutElapsed = 0;
    StepEventSend.Step.StepNr = StepEventSend.StepHandling.Current.nTimeoutContinueStep;
  }
  StepEventSend.StepHandling.Current.nStepNr = (DINT) StepEventSend.Step.StepNr;
  brsstrcpy((UDINT) &StepEventSend.StepHandling.Current.sStepText, (UDINT) &StepEventSend.Step.StepText);
  BrbStepHandler(&StepEventSend.StepHandling);
  
  switch (StepEventSend.Step.StepNr)
  {    
    case UDP_EVENT_SEND_INIT:
      brsstrcpy((UDINT) &StepEventSend.Step.StepText, (UDINT) "UDP_EVENT_SEND_INIT");	// stephandling: copy actual step
      WriteStepDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.EventSend, &CmdBurLogBook, "EventSend", "UDP_EVENT_SEND_INIT", StepEventSend.Step.StepNr, udTimeTick);
			
      StepEventSend.Step.InitDone = TRUE;
      StepEventSend.Step.StepNr = UDP_EVENT_CHECK_PIPE;
      break;
    
    /***********************************************************************************/
    /* Schritt 1:  Pipe wird überprüft  */
    case UDP_EVENT_CHECK_PIPE:
    {  
      brsstrcpy((UDINT) &StepEventSend.Step.StepText, (UDINT) "UDP_EVENT_CHECK_PIPE");	// stephandling: copy actual step
      WriteStepDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.EventSend, &CmdBurLogBook, "EventSend", "UDP_EVENT_CHECK_PIPE", StepEventSend.Step.StepNr, udTimeTick);
  	
      /* --- Pipes durchsuchen ------------------------------------------------------------------------------- */
      for (EventSend.PipeCnt = EventSend.ActPipe; EventSend.PipeCnt < MAX_ANZAHL_CLIENTS; EventSend.PipeCnt++ ) 
      {
        /* --- Client überprüfen: gelinkt? ----------------------------------------------------------------- */
        pClientListeEventSend = pClientListeStart + EventSend.PipeCnt;
            
        if (Diagnose.LogPointerInfo == TRUE)
        {
          ValidatePointerServer((UDINT) pClientListeEventSend, 21, (MemManagerDiag_type*) &sMemDiag, 1);	/* Erweiterung, zur Überprüfung von Adressen */
        }
            
        if (pClientListeEventSend->LinkFlag == STATUS_EVENT)
        {              
          if (pClientListeEventSend->Modus == MODE_NEW)
          {
            /* --- aktuelle Pipe untersuchen ------------------------------------------------- */
            /* --- -> Telegramm generieren --------------------------------------------------- */
            uiEvntTelLen = 0;
            if (Diagnose.LogPointerInfo == TRUE)
            {
              ValidatePointerServer((UDINT) pSendBufEvent, 22, (MemManagerDiag_type*) &sMemDiag, 1);	/* Erweiterung, zur Überprüfung von Adressen */
            }
            
            bGetPipeDataForEvent = GetEventPipeDataNewMode(&EventSend, pSendBufEvent, &uiEvntTelLen, &uiClientEventNr);
            if (bGetPipeDataForEvent)
            {
              EventSend.Telegramm = TRUE;
              EventSend.ActPipe   = EventSend.PipeCnt + 1;  /* nächste Clientposition abspeichern */
              Diagnose.Base.PipeRead[EventSend.PipeCnt]++;
              break;
            }
          }
          else if (pClientListeEventSend->Modus == MODE_OLD)
          {
              /* --- aktuelle Pipe untersuchen ------------------------------------------------- */
              
              uiEvntTelLen = 0;
              if (Diagnose.LogPointerInfo == TRUE)
              {
                ValidatePointerServer((UDINT) pSendBufEvent, 23, (MemManagerDiag_type*) &sMemDiag, 1);	/* Erweiterung, zur Überprüfung von Adressen */
              }
              /* --- Telegramm generieren --------------------------------------------------- */
              bGetPipeDataForEvent = GetEventPipeDataOldMode(&EventSend, &uiClientEventNr, &sPVEvent);
              if (bGetPipeDataForEvent)
              {
                /* Daten befinden sich jetzt auf sPVEvent */
                CopyPVEventToSendBufferEvent(pSendBufEvent, &sPVEvent);
                uiEvntTelLen = sizeof(sPVEvent);
                EventSend.Telegramm = TRUE;
                EventSend.ActPipe   = EventSend.PipeCnt + 1;  /* nächste Clientposition abspeichern */
                Diagnose.Base.PipeRead[EventSend.PipeCnt]++;
                break;
              }
          }
        }            
      }

      /* ------------------- Pipeanzahl überschritten: --------------------------------------------- */
      if ((EventSend.PipeCnt + 1) >= MAX_ANZAHL_CLIENTS)
      {
        EventSend.ActPipe = 0;
      }      
        
      /* ------------------------------- neues Telegramm wird gesendet --------------------- */
      if (EventSend.Telegramm == TRUE)
      {
        EventSend.Telegramm = FALSE;
                  
        /* --- der Telegrammheader wurde bereits generiert: Fkt. "GetEventPipeDataNewMode"  */          
        pClientListeEventSend =  pClientListeStart + uiClientEventNr;
          
        /* --- UDP Send Funktion parametrieren ------------------------------------------ */
        UDPsendEventFub.enable  = TRUE;
        UDPsendEventFub.ident   = UDPopenFub[2].ident;
        UDPsendEventFub.pData   = (UDINT) pSendBufEvent;
        UDPsendEventFub.datalen = uiEvntTelLen;         
        UDPsendEventFub.pHost   = (UDINT) &pClientListeEventSend->IpAdr;
        UDPsendEventFub.port    = PortTarget[3];
        UDPsendEventFub.flags   = udpSendFlags;
          
        Diagnose.Base.Events.SendTelegrammCmd++;

        UdpSend(&UDPsendEventFub);
        StepEventSend.Step.StepNr = UDP_EVENT_SEND;
          
      } /* if (EventSend.Telegramm == TRUE) */
    }
    break;
    
    /***********************************************************************************/
    case UDP_EVENT_SEND:
    {
      brsstrcpy((UDINT) &StepEventSend.Step.StepText, (UDINT) "UDP_EVENT_SEND");	// stephandling: copy actual step
      WriteStepDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.EventSend, &CmdBurLogBook, "EventSend", "UDP_EVENT_SEND", StepEventSend.Step.StepNr, udTimeTick);
	
      if (UDPsendEventFub.status != FUB_BUSY)
      {
        if (UDPsendEventFub.status == FUB_OK)
        {
          StepEventSend.Step.StepNr = UDP_EVENT_CHECK_PIPE;
          Diagnose.Base.SendDataToClient[EventSend.PipeCnt]++;
          Diagnose.Base.Events.SendTelegrammOk++;
        }
        else
        {
          StepEventSend.Step.StepNr = UDP_EVENT_CHECK_PIPE;
            
          CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.EventSend, &CmdBurLogBook, "EventsSend", "failed to send data to client (EVNX)", (char*) UDPsendEventFub.pHost, UDPsendEventFub.status, StepEventSend.Step.StepNr, udTimeTick);
          CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
        }
      }
      else
      {       
        UdpSend(&UDPsendEventFub);
      }
    }
    break;
    
  } /* switch (StepEventSend.Step.StepNr) - Ende:  */

}


/***********************************************************************************************************
* --- Empfangsbestätigung von clients auswerten
********************************************************************************************************** */

void CyclicSLSEventsRecv()
{
  
  // Stephandler: overhead /initialisierung
  if (StepEventRecv.StepHandling.Current.bTimeoutElapsed == 1)
  {
    StepEventRecv.StepHandling.Current.bTimeoutElapsed = 0;
    StepEventRecv.Step.StepNr = StepEventRecv.StepHandling.Current.nTimeoutContinueStep;
  }
  StepEventRecv.StepHandling.Current.nStepNr = (DINT) StepEventRecv.Step.StepNr;
  brsstrcpy((UDINT) &StepEventRecv.StepHandling.Current.sStepText, (UDINT) &StepEventRecv.Step.StepText);
  BrbStepHandler(&StepEventRecv.StepHandling);
  
  /********************************************************************************************************/
  switch (StepEventRecv.Step.StepNr)
  {
    case UDP_EVENT_RECEIVE_INIT:
      brsstrcpy((UDINT) &StepEventRecv.Step.StepText, (UDINT) "UDP_EVENT_RECEIVE_INIT");	// stephandling: copy actual step
      WriteStepDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.EventRecv, &CmdBurLogBook, "EventRecv",  "UDP_EVENT_RECEIVE_INIT", StepEventRecv.Step.StepNr, udTimeTick);
  			
      StepEventRecv.Step.InitDone = TRUE;
      StepEventRecv.Step.StepNr = UDP_EVENT_RECEIVE_SET;
      break;
    
    /*########################################################################################################*/
    /* Bestätigung des Eventtelegrammes parametrieren                             */
    case UDP_EVENT_RECEIVE_SET:
      brsstrcpy((UDINT) &StepEventRecv.Step.StepText, (UDINT) "UDP_EVENT_RECEIVE_SET");	// stephandling: copy actual step
      WriteStepDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.EventRecv, &CmdBurLogBook, "EventRecv",  "UDP_EVENT_RECEIVE_SET", StepEventRecv.Step.StepNr, udTimeTick);
      
      UDPreceiveEventFub.enable = TRUE;
      UDPreceiveEventFub.ident  = UDPopenFub[3].ident;
      UDPreceiveEventFub.pData  = (UDINT) pRecvBufEvent;
      UDPreceiveEventFub.datamax  = MAX_UDP_FRAME_LEN;
      UDPreceiveEventFub.pIpAddr  = (UDINT) &arRecvEvntIpAdr;

      StepEventRecv.Step.StepNr = UDP_EVENT_RECEIVE;
      /* -> kein break notwendig */
    
    /*########################################################################################################*/
    /* Empfang des Eventtelegrammes auswerten                               */
    case UDP_EVENT_RECEIVE:
      
      brsstrcpy((UDINT) &StepEventRecv.Step.StepText, (UDINT) "UDP_EVENT_RECEIVE");	// stephandling: copy actual step
      WriteStepDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.EventRecv, &CmdBurLogBook, "EventRecv",  "UDP_EVENT_RECEIVE", StepEventRecv.Step.StepNr, udTimeTick);
      
	    UdpRecv(&UDPreceiveEventFub);
      if (UDPreceiveEventFub.status == FUB_OK)
      {     
        Diagnose.Base.Events.ReceiveAckFromClient++;
        
        /* --- Kennung auf Char - Feld umkopieren ---------------------------------------------------- */
        brsmemcpy((UDINT) &arKennung,(UDINT) pRecvBufEvent, 4);
          
          if ((arKennung[0] = 'A') &&
              (arKennung[1] = 'C') &&
              (arKennung[2] = 'K') &&
              (arKennung[3] = 'N'))
        {
          CopyReceiveBufferEventToPVEventAcknowledge((UDINT*) pRecvBufEvent); 

          CRCCheckTemp = CRCcheck((UDINT*) pRecvBufEvent, 12);  
          
          if (sPVEventAcknowledge.BCC == CRCCheckTemp)
          {                        
            /* --------------- Zeitstempel aktualisieren ------------------------------ */
            /* Sicherheitsabfrage: wenn Zeit überschritten ist, so wird Client entfernt */
            for (iClient = 0; iClient < MAX_ANZAHL_CLIENTS; iClient++)
            {
              pClientListeEventSend = pClientListeStart + iClient;
              if (Diagnose.LogPointerInfo == TRUE)
              {
                ValidatePointerServer((UDINT) pClientListeEventSend, 30, (MemManagerDiag_type*) &sMemDiag, 1);	/* Erweiterung, zur Überprüfung von Adressen */
              }
              
              if (pClientListeEventSend->IpAdr[0] != 0)
              {
                /* --- IP Adresse vom Absender mit IP Adresse des Clients vergleichen */
                sdCmpRes = brsstrcmp((UDINT) &pClientListeEventSend->IpAdr, (UDINT) UDPreceiveEventFub.pIpAddr);
                if (sdCmpRes == 0)
                {
                  /* --- Zeitstempel für Clienttelegramm eintragen -------------- */
                  pClientListeEventSend->LastTimeStamp = udTimeTick;
                  break;
                }
              }
            }
            Diagnose.Base.Events.ReceiveTelegramm++;
          }
        }
      }
      break;
  }

}


/*************************************************************************************************************************
* --- Lifecheck für Server 
* -> Server wartet lediglich auf LifeCheck Telegramm von Client
* -> falls Telegramm eintrifft, so wird nur einmalig eine Antwort gebildet und versendet
**************************************************************************************************************************/

void CyclicSLSLifeCheck()
{
  
  switch (UDPLifeCheck)
  {
    case LIFE_CHECK_RESPONSE: /*  Abfrage der Empfangsfkt. und bilden der Antwort           */
      
      UdpRecv(&UDPreceiveLiveFub);
      if (UDPreceiveLiveFub.status == FUB_OK)
      {     
        if ((arRecvBufLife[0] == 'L') &&
            (arRecvBufLife[1] == 'I') &&
            (arRecvBufLife[2] == 'F') &&
            (arRecvBufLife[3] == 'E')) 
        {
          CopyReceiveBufferLifeToLifeCheckRequest();

          CRCCheckTemp = CRCcheck((UDINT*) &arRecvBufLife[0], 6);
          
          Diagnose.Base.LifeCheck.TelegrammCnt++;
          if (CRCCheckTemp == sLifeCheckRequest.BCC)
          {
            /* ----------------- Antworttelegramm wird generiert : ---------------------------- */
            brsmemcpy((UDINT) &sLifeCheckResponse.Kennung, (UDINT) "LIFE", 4);

            /* ---------------- Status des Clients ermitteln ---------------------------------- */
            LifeClientFound = 0;
            for (iClient = 0; iClient < MAX_ANZAHL_CLIENTS; iClient++ )
            {
              pClientListeLiveCheck = pClientListeStart + iClient;
              if (Diagnose.LogPointerInfo == TRUE)
              {            
		        ValidatePointerServer((UDINT) pClientListeLiveCheck, 40, (MemManagerDiag_type*) &sMemDiag, 1);	/* Erweiterung, zur Überprüfung von Adressen */
              }
              
              sdCmpRes = brsstrcmp((UDINT) &pClientListeLiveCheck->IpAdr, (UDINT) UDPreceiveLiveFub.pIpAddr);
              if (sdCmpRes == 0)
              {
                /* --- Zeitstempel für Clienttelegramm eintragen */
                pClientListeLiveCheck->LastTimeStamp = udTimeTick;
                Diagnose.Base.LifeCheck.TelegrOk++;
                
                /* --- Eventbetrieb ist ein  -------------------------- */
                if (pClientListeLiveCheck->LinkFlag == STATUS_EVENT)
                {
                  sLifeCheckResponse.ClientStatus = STATE_LIFE_OK;
                  LifeClientFound = 1;
                }
                /* --- Client wird zur Zeit angemeldet ---------------- */
                else if (pClientListeLiveCheck->LinkFlag == STATUS_OPEN)
                {
                  sLifeCheckResponse.ClientStatus = STATE_LIFE_CONN;
                  LifeClientFound = 1;
                }
                /* --- Client geht komplett ab ------------------------ */
                else
                {
                  /* --- Client fehlt -------------------------------- */
                  sLifeCheckResponse.ClientStatus = STATE_LIFE_CLIENT_MISSING;
                  LifeClientFound = 0;
                }
                break;
              }
            }
            
            if  (LifeClientFound == 0)
            {
              sLifeCheckResponse.ClientStatus = STATE_LIFE_CLIENT_MISSING;
            }
            
            /* --- Sicherheitskopie des Telegramms --------------------------------------------------------- */
            brsmemcpy((UDINT) &sLifeCheckResponseSafe,(UDINT) &sLifeCheckResponse, sizeof(LifeCheckResponse_typ));
            
            /* --- Telegramm auf Sendebuffer kopieren ------------------------------------------------------ */
            CopyLifeCheckResponseToSendBuffer();
            
            /* --- das Telegramm wird einmal losgesendet, damit ist die Arbeit für den Server beendet -------*/
            UDPsendLiveFub.enable = TRUE;
            UDPsendLiveFub.ident  = UDPopenFub[4].ident;
            UDPsendLiveFub.pData  = (UDINT) &arSendBufLife;
            UDPsendLiveFub.datalen  = sizeof(arSendBufLife);
            UDPsendLiveFub.pHost  = UDPreceiveLiveFub.pIpAddr;
            UDPsendLiveFub.port   = PortTarget[5];
            UDPsendLiveFub.flags  = udpSendFlags;
            UdpSend(&UDPsendLiveFub);
            
            UDPLifeCheck = LIFE_CHECK_RESPONSE_WAIT;
          }
        }
      }      
      break;
    
    case LIFE_CHECK_RESPONSE_WAIT:
      
      if ( UDPsendLiveFub.status != ERR_FUB_BUSY)
      {
        if (UDPsendLiveFub.status == ERR_OK)
        {
          /* --- Diagnose für Lifecheck ----------------------------- */
          Diagnose.Base.LifeCheck.TelegrammSend++;
        }
       
        UDPLifeCheck  = LIFE_CHECK_RESPONSE;
      }
      else
      {
        UdpSend(&UDPsendLiveFub);
      }
      break; 
  } 
  
}


/*************************************************************************************************************************
* --- Status des Clients ermitteln 
**************************************************************************************************************************/

void CyclicGetLifeStateOfClients()
{	
  UINT i;
  
  for (i = 0; i < MAX_ANZAHL_CLIENTS; i++ )
  {
    pClientListeLiveCheck = pClientListeStart + i;
    if (Diagnose.LogPointerInfo == TRUE)
    {
      ValidatePointerServer((UDINT) pClientListeLiveCheck, 50, (MemManagerDiag_type*) &sMemDiag, 1);	/* Erweiterung, zur Überprüfung von Adressen */
    }
    
    /* ------------------------------------- Client ist gelinkt ------------------------------------------- */
    if (pClientListeLiveCheck->LinkFlag == STATUS_EVENT)
    {
      if (pClientListeLiveCheck->IpAdr[0] != 0)
      {
        /* --------------------------------------- Client noch am Netz ? ------------------------------- */
        /* Änderung MEN: Achtung -> Intervall einige Minuten ! -> bei sehr großen Anlagen ~ 100      */
        /* UST kann es zu riesigen Timeouts kommen, wenn viele USTs bereits parametriert sind, aber noch */
        /* nicht existieren */        
        //if (udTimeTick - pClientListeLiveCheck->LastTimeStamp > TIMEOUT_DELETE_CLIENT / 10)
        if (udTimeTick - pClientListeLiveCheck->LastTimeStamp > (pClientListeLiveCheck->Timeout /10) * TIMEOUT_LIFE_RETRY)
        {
          
          Diagnose.Base.Events.ClientFailed++;
          DeleteExistingClient(i + 1);
    
          /* ----- Client komplett austragen, ansonsten wird bei Anmeldung wieder alles durchsucht ------ */
          brsmemset ((UDINT) pClientListeLiveCheck , 0, sizeof(ClientListeEntry_typ));
          
          CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Lifecheck, &CmdBurLogBook, "CyclicGetLifeStateOfClients", "removed idle client",(char*) &pClientListeLiveCheck->IpAdr, i + 1, 999, udTimeTick);
          CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;          
        }
      }
    }
  }
}


/*****************************************************************************************************************/
/*                        CRC Check                              */
/*****************************************************************************************************************/

UINT CRCcheck (
UDINT* TelegrammStartAdresse, 
UINT Telegrammlaenge)
{
  
  USINT data;
  USINT *sData;
  UINT  bcs;  
  UINT  cnt;
  UINT j;
  
  bcs = 0;
  
  sData = (USINT*) TelegrammStartAdresse;
  
  for (cnt = 0; cnt < Telegrammlaenge; cnt++)
  {
    data = sData[cnt];
    for (j = 0; j < 8; j++) 
    {
      if (((data^bcs)&0x01) != 0) 
      {
        bcs>>=1;
        bcs^=0x8408;
      }
      else 
      {
        bcs>>=1;
      }
      data>>=1; 
    }
  }
  
  return bcs;
}


/*****************************************************************************************************************
* Schliessen der UDP - Ports bei Löschen des Tasks
*****************************************************************************************************************/

_EXIT void taskExit(void)
{
UINT i;
  
  for (i = 0; i < PORT_CNT; i++)
  {
    UDPcloseFub[i].ident = UDPopenFub[i].ident;
    UDPcloseFub[i].enable = TRUE;
    UdpClose(&UDPcloseFub[i]);
  }
  
  
  WriteLoggerMessage((Logger_typ*) &sLogger, 0xF000, (STRING*) "server:exit", 0x7FFFFFFF, PTOP_LOG_SEVERITY_INFO);
}


/*****************************************************************************************************************/
/*              Swap-Funktionen für Intel                              */
/*****************************************************************************************************************/
UDINT SwapUdintForIntel(UDINT value)
{
#if INTEL
  return swapUDINT(value);
#else
  return value;
#endif
}

UINT SwapUintForIntel(UINT value)
{
#if INTEL
  return swapUINT(value);
#else
  return value;
#endif
}

/*****************************************************************************************************************/
/*              Ticks auslesen                                 */
/*****************************************************************************************************************/
UDINT TimerFunction (UDINT *AdrTickCount)
{

#if INTEL
  SysInfo_typ   SysInfoFub;
  
  SysInfoFub.enable = 1;
  SysInfo(&SysInfoFub);
  *AdrTickCount = SysInfoFub.tick_count;
  return 0;
#else
  UDINT   InitCount;
  UDINT InitDescription;   
  UDINT   ObjectVersion;
  UDINT   Version;
  UDINT   TickCount;
  UDINT StatusInfo;
  
  StatusInfo = SYS_info(&InitCount, &InitDescription, &TickCount, &Version, &ObjectVersion);
  *AdrTickCount = TickCount;
  return StatusInfo;
#endif

}

/*****************************************************************************************************************/
/*              Ip-Adresse auslesen                                  */
/*****************************************************************************************************************/
UDINT GetOwnIpAddress()
{
#if INTEL
  CfgGetIPAddr_typ  CfgGetIpAddrFub;
  USINT   IpAddressString[16];
  UDINT   IpAddress;
  UDINT   retIpAdr = 0;
  
  IpAddressString[0] = 0;
  CfgGetIpAddrFub.enable = 1;
  CfgGetIpAddrFub.pDevice = (UDINT) "IF2";
  CfgGetIpAddrFub.pIPAddr = (UDINT) &IpAddressString;
  CfgGetIpAddrFub.i_state = 0;
  CfgGetIpAddrFub.i_result = 0;
  CfgGetIpAddrFub.i_tmp = 0;
  CfgGetIpAddrFub.Len = sizeof(IpAddressString);
  do
  {
    CfgGetIPAddr(&CfgGetIpAddrFub);
  }while (CfgGetIpAddrFub.status == 0xFFFF);

  if (CfgGetIpAddrFub.status == FUB_OK)
  {
    ethInetAton((UDINT) &IpAddressString, (UDINT) &retIpAdr);
    IpAddress = SwapUdintForIntel(retIpAdr);
    //IpAddress = SwapUdintForIntel(inet_addr((UDINT)IpAddressString));
  }
  else
  {
    IpAddress = 0;  /* keine gültige Adresse gefunden -> 0) */
  }
  return IpAddress;
#else
  ETHnode_typ         ETHNodeFub;
  
  ETHNodeFub.enable = TRUE;
  ETHnode(&ETHNodeFub);
  return ETHNodeFub.ipaddr;
#endif
}

/****************************************************************************************************************/
/*              Übertragen eines ClientConnectRequest-Telegramms                  */
/*              aus dem Empfangsbuffer                                */
/*              in Struktur "ClientConnectRequest"                          */
/****************************************************************************************************************/
void CopyReceiveBufferToClientConnectRequest(
UDINT* pRecvBuf,
ClientConnectRequest_typ* pClientConnectRequest)
{

UINT  ClientIndex;              /* Byte 4/5 */
UINT  Timeout;                /* Byte 6/7 */
UINT  BCC;                  /* Byte 8/9 */
USINT arTempBuf[OPEN_BUFFER_LEN];

  brsmemcpy((UDINT) &arTempBuf, (UDINT) pRecvBuf, sizeof(arTempBuf));
  
  /* Kennung */
  *(UDINT*)&pClientConnectRequest->Kennung = *(UDINT*)&arTempBuf[0];
  /* ClientIndex */
  *(UINT*)&ClientIndex = *(UINT*)&arTempBuf[4];
  pClientConnectRequest->ClientIndex = SwapUintForIntel(ClientIndex);
  /* Timeout */
  *(UINT*)&Timeout = *(UINT*)&arTempBuf[6];
  pClientConnectRequest->Timeout = SwapUintForIntel(Timeout);
  /* BCC */
  *(UINT*)&BCC = *(UINT*)&arTempBuf[8];
  pClientConnectRequest->BCC = SwapUintForIntel(BCC);
}


/****************************************************************************************************************/
/*              Übertragen eines ClientConnectResponse-Telegramms                   */
/*              aus der Struktur "ClientConnectResponse" in den Sendebuffer             */
/*              mit Eintragung der Checksum                             */
/****************************************************************************************************************/

void CopyClientConnectResponseToSendBuffer(
UDINT* pSendBuf,
ClientConnectResponse_typ* pClientConnectResponse)
{
UINT  ClientIndex;              /* Byte 4/5 */
UINT  Status;                 /* Byte 6/7 */
UINT  BCC;                  /* Byte 8/9 */
UINT  BCCraw;
USINT arTempBuf[OPEN_BUFFER_LEN];

  /* Sendebuffer löschen */ 
  brsmemset((UDINT) &arTempBuf[0], 0, sizeof(arTempBuf));
  
  /* Kennung */
  *(UDINT*)&arTempBuf[0] = *(UDINT*)&pClientConnectResponse->Kennung;
  /* ClientIndex */
  ClientIndex = SwapUintForIntel(pClientConnectResponse->ClientIndex);
  *(UINT*)&arTempBuf[4] = *(UINT*)&ClientIndex;
  /* Status */
  Status = SwapUintForIntel(pClientConnectResponse->Status);
  *(UINT*)&arTempBuf[6] = *(UINT*)&Status;
  /* BCC */
  BCCraw = CRCcheck((UDINT*)&arTempBuf, 8);
  pClientConnectResponse->BCC = BCCraw;  
  BCC = SwapUintForIntel(BCCraw);
  *(UINT*)&arTempBuf[8] = *(UINT*)&BCC;
  
  brsmemcpy((UDINT) pSendBuf, (UDINT) &arTempBuf, OPEN_BUFFER_LEN);
}


/****************************************************************************************************************/
/*              Übertragen eines PVOpenRequest-Telegramms                     */
/*              aus dem Empfangsbuffer                                */
/*              in Struktur "PVOpenRequest"                             */
/****************************************************************************************************************/
void CopyReceiveBufferToPVOpenRequest(
UDINT* pRecvBuf,
PVOpenRequest_typ* pOpenRequest)
{

UINT  ClientIndex;              /* Byte 4/5 */
UINT  PVIndex;                /* Byte 56/57 */
UINT  Laenge;                 /* Byte 58/59 */
UDINT Hysterese;                /* Byte 60-63 */
UINT  SyncTime;               /* Byte 64/65 */
UDINT Reserve[2];               /* Byte 66/73 */
UINT  BCC;                  /* Byte 74/75 */
UINT    IndexBuffer = 0;
USINT arTempBuf[OPEN_BUFFER_LEN];

  brsmemcpy((UDINT) &arTempBuf, (UDINT) pRecvBuf, sizeof(arTempBuf));

  /* --- Kennung */
  *(UDINT*)&pOpenRequest->Kennung = *(UDINT*)&arTempBuf[IndexBuffer];
  IndexBuffer += 4; /* 4 */
  
  /* --- ClientIndex */ 
  *(UINT*)&ClientIndex = *(UINT*)&arTempBuf[IndexBuffer]; 
  pOpenRequest->ClientIndex = SwapUintForIntel(ClientIndex);
  IndexBuffer += 2; /* 6 */
  
  /* --- Variablenname */
  brsmemcpy ((UDINT) &pOpenRequest->Variablenname, (UDINT) &arTempBuf[IndexBuffer], sizeof(pOpenRequest->Variablenname));
  IndexBuffer += sizeof(pOpenRequest->Variablenname);
  
  /* --- PVIndex */
  *(UINT*)&PVIndex = *(UINT*)&arTempBuf[IndexBuffer];
  pOpenRequest->PVIndex = SwapUintForIntel(PVIndex);
  IndexBuffer += 2;
  
  /*  --- Laenge */
  *(UINT*)&Laenge = *(UINT*)&arTempBuf[IndexBuffer];
  pOpenRequest->Laenge = SwapUintForIntel(Laenge);
  IndexBuffer += 2;
    
  /* --- Hysterese */
  *(UDINT*)&Hysterese = *(UDINT*)&arTempBuf[IndexBuffer];
  pOpenRequest->Hysterese = SwapUdintForIntel(Hysterese);
  IndexBuffer += 4;
  
  /* --- SyncTime */
  *(UINT*)&SyncTime = *(UINT*)&arTempBuf[IndexBuffer];
  pOpenRequest->SyncTime = SwapUintForIntel(SyncTime);
  IndexBuffer += 2;
  
  /*  --- Reserve[0] */
  *(UDINT*)&Reserve[0] = *(UDINT*)&arTempBuf[IndexBuffer];
  pOpenRequest->Reserve[0] = SwapUdintForIntel(Reserve[0]);
  IndexBuffer += 4;
  
  /* --- Reserve[1] */
  *(UDINT*)&Reserve[1] = *(UDINT*)&arTempBuf[IndexBuffer];
  pOpenRequest->Reserve[1] = SwapUdintForIntel(Reserve[1]);
  IndexBuffer += 4;
  
  /* --- BCC */
  *(UINT*)&BCC= *(UINT*)&arTempBuf[IndexBuffer];
  pOpenRequest->BCC = SwapUintForIntel(BCC);

}


/****************************************************************************************************************/
/*              Übertragen eines PVOpenResponse-Telegramms                      */
/*              aus der Struktur "PVOpenResponse" in den Sendebuffer                */
/*              mit Eintragung der Checksum                             */
/****************************************************************************************************************/

void CopyPVOpenResponseToSendBuffer(
UDINT *pSendBuf,
PVOpenResponse_typ* pPVOpenResponse)
{
UINT  Status;                 /* Byte 4/5 */
UINT  ClientIndex;              /* Byte 6/7 */
UINT  PVIndex;                /* Byte 8/9 */
UDINT PVIdent;                /* Byte 10-13 */
UINT  Laenge;                 /* Byte 14/15 */
UDINT Wert;                 /* Byte 16-19 */
UINT  BCC;                  /* Byte 20/21 */
UINT  BCCraw;
USINT arTempBuf[OPEN_BUFFER_LEN];
  
  /* Sendebuffer löschen */ 
  brsmemset((UDINT) &arTempBuf[0], 0, sizeof(arTempBuf));  

  /* Kennung */
  brsmemcpy ((UDINT) &arTempBuf[0], (UDINT) &pPVOpenResponse->Kennung,sizeof(pPVOpenResponse->Kennung));
  *(UDINT*)&arTempBuf[0] = *(UDINT*)&pPVOpenResponse->Kennung;
  /* Status */
  Status = SwapUintForIntel(pPVOpenResponse->Status);
  *(UINT*)&arTempBuf[4] = *(UINT*)&Status;
  /* ClientIndex */
  ClientIndex = SwapUintForIntel(pPVOpenResponse->ClientIndex);
  *(UINT*)&arTempBuf[6] = *(UINT*)&ClientIndex;
  /* PVIndex */
  PVIndex = SwapUintForIntel(pPVOpenResponse->PVIndex);
  *(UINT*)&arTempBuf[8]= *(UINT*)&PVIndex;
  /* PVIdent */
  PVIdent = SwapUdintForIntel(pPVOpenResponse->PVIdent);
  *(UDINT*)&arTempBuf[10]= *(UDINT*)&PVIdent;
  /* Laenge */
  Laenge = SwapUintForIntel(pPVOpenResponse->Laenge);
  *(UINT*)&arTempBuf[14] = *(UINT*)&Laenge;
  /* Wert */
  Wert = SwapUdintForIntel(pPVOpenResponse->Wert);
  *(UDINT*)&arTempBuf[16] = *(UDINT*)&Wert;
  /* BCC */
  BCCraw = CRCcheck((UDINT*)&arTempBuf[0], 20);
  pPVOpenResponse->BCC = BCCraw; 
  BCC = SwapUintForIntel(BCCraw);
  *(UINT*)&arTempBuf[20] = *(UINT*)&BCC;

  brsmemcpy((UDINT) pSendBuf, (UDINT) &arTempBuf, sizeof(arTempBuf));
}


/****************************************************************************************************************/
/*              Übertragen eines ClientLink-Telegramms                        */
/*              aus dem Empfangsbuffer                                */
/*              in Struktur "ClientLink"                              */
/****************************************************************************************************************/

void CopyReceiveBufferToClientLink(
UDINT* pRecvBuf,
ClientLink_typ* pClientLink)
{
UINT  ClientIndex;          /* Byte 4/5 */
UINT  BCC;                  /* Byte 6/7 */
USINT arTempBuf[OPEN_BUFFER_LEN];
  
  brsmemcpy((UDINT) &arTempBuf, (UDINT) pRecvBuf, sizeof(arTempBuf));

  /* Kennung */
  *(UDINT*)&pClientLink->Kennung = *(UDINT*)&arTempBuf[0];
  /* ClientIndex */
  *(UINT*)&ClientIndex = *(UINT*)&arTempBuf[4];
  pClientLink->ClientIndex = SwapUintForIntel(ClientIndex);
  /* BCC */
  *(UINT*)&BCC = *(UINT*)&arTempBuf[6];
  pClientLink->BCC = SwapUintForIntel(BCC);  

}


/****************************************************************************************************************
* --- Übertragen eines LinkResponse-Telegramms 
* --- aus der Struktur "NAK" in den Sendebuffer
* --- mit Eintragung der Checksum
*****************************************************************************************************************/

void CopyNAKToSendBuffer(
UDINT* pSendBuf,
AckResponse_typ* pNAK)
{

UINT  Status;               /* Byte 4/5 */
UINT  Reserve2;             /* Byte 6/7 */
UINT  BCC;                  /* Byte 8/9 */
UINT  BCCraw;
USINT arTempBuf[OPEN_BUFFER_LEN];

  /* Sendebuffer löschen */
  brsmemset((UDINT) &arTempBuf[0], 0, sizeof(arTempBuf));
  /* Kennung */
  *(UDINT*)&arTempBuf[0] = *(UDINT*)&pNAK->Kennung;
  /* Status */
  Status = SwapUintForIntel(pNAK->Status);
  *(UINT*)&arTempBuf[4] = *(UINT*)&Status;
  /* Reserve2 */
  Reserve2 = SwapUintForIntel(pNAK->Reserve2);
  *(UINT*)&arTempBuf[6] = *(UINT*)&Reserve2;
  /* BCC */
  BCCraw = CRCcheck((UDINT*)&arTempBuf, 8);
  pNAK->BCC = BCCraw;  
  BCC = SwapUintForIntel(BCCraw);
  *(UINT*)&arTempBuf[8] = *(UINT*)&BCC;
  
  brsmemcpy((UDINT) pSendBuf, (UDINT) &arTempBuf, sizeof(arTempBuf));
}


/***************************************************************************************************************
* --- Übertragen eines PVEvent-Telegramms 
* --- aus der Struktur "PVEvent" in den Sendebuffer
* --- mit Eintragung der Checksum
****************************************************************************************************************/

void CopyPVEventToSendBufferEvent(
UDINT* pSendBuf,
PVEvent_typ* pPVEvent)
{
UINT  PVIndex;                /* Byte 4/5 */
UINT  Laenge;                 /* Byte 6/7 */
UDINT Wert;                 /* Byte 8-11 */
UINT  EventCnt;               /* Byte 12/13 */
UINT  BCC;                  /* Byte 14/15 */
UINT  BCCraw;
USINT arTempBuf[20];

  /* Sendebuffer löschen */
  brsmemset((UDINT) &arTempBuf[0], 0, sizeof(arTempBuf));
  /* Kennung */
  *(UDINT*)&arTempBuf[0] = *(UDINT*)&pPVEvent->Kennung;
  /* PVIndex */
  PVIndex = SwapUintForIntel(pPVEvent->PVIndex);
  *(UINT*)&arTempBuf[4] = *(UINT*)&PVIndex;
  /* Laenge */
  Laenge = SwapUintForIntel(pPVEvent->Laenge);
  *(UINT*)&arTempBuf[6] = *(UINT*)&Laenge;
  /* Wert */
  Wert = SwapUdintForIntel(pPVEvent->Wert);
  *(UDINT*)&arTempBuf[8] = *(UDINT*)&Wert;
  /* EventCnt */
  EventCnt = SwapUintForIntel(pPVEvent->EventCnt);
  *(UINT*)&arTempBuf[12] = *(UINT*)&EventCnt;
  /* BCC */
  BCCraw = CRCcheck((UDINT*) &arTempBuf, 14); 
  pPVEvent->BCC = BCCraw;  
  BCC = SwapUintForIntel(BCCraw);
  *(UINT*)&arTempBuf[14] = *(UINT*)&BCC;

  brsmemcpy((UDINT) pSendBuf, (UDINT) &arTempBuf, sizeof(arTempBuf));
}


/****************************************************************************************************************/
/*              Übertragen eines PVEventAcknowledge-Telegramms                    */
/*              aus dem Empfangsbuffer                                */
/*              in Struktur "PVEventAcknowledge"                          */
/****************************************************************************************************************/

void CopyReceiveBufferEventToPVEventAcknowledge(
UDINT* pSendBuf)
{
UINT  Status;                 /* Byte 4/5 */
UINT  PVIndex;                /* Byte 6/7 */
UINT  ClientIndex;              /* Byte 8/9 */
UINT  EventCnt;               /* Byte 10/11 */
UINT  BCC;                  /* Byte 12/13 */
USINT arTempBuf[20];

  brsmemcpy((UDINT) &arTempBuf, (UDINT) pSendBuf, sizeof(arTempBuf));
  
  /* Kennung */
  *(UDINT*)&sPVEventAcknowledge.Kennung = *(UDINT*)&arTempBuf[0];
  /* Status */
  *(UINT*)&Status = *(UINT*)&arTempBuf[4];
  sPVEventAcknowledge.Status = SwapUintForIntel(Status);
  /* PVIndex */
  *(UINT*)&PVIndex = *(UINT*)&arTempBuf[6];
  sPVEventAcknowledge.PVIndex = SwapUintForIntel(PVIndex);
  /* ClientIndex */
  *(UINT*)&ClientIndex = *(UINT*)&arTempBuf[8];
  sPVEventAcknowledge.ClientIndex = SwapUintForIntel(ClientIndex);
  /* EventCnt */
  *(UINT*)&EventCnt = *(UINT*)&arTempBuf[10];
  sPVEventAcknowledge.EventCnt = SwapUintForIntel(EventCnt);
  /* BCC */
  *(UINT*)&BCC = *(UINT*)&arTempBuf[12];
  sPVEventAcknowledge.BCC = SwapUintForIntel(BCC);  

}


/****************************************************************************************************************/
/*              Übertragen eines LifeCheckRequest-Telegramms                    */
/*              aus dem Empfangsbuffer                                */
/*              in Struktur "LifeCheckRequest"                            */
/****************************************************************************************************************/
void CopyReceiveBufferLifeToLifeCheckRequest()
{

  UINT  Reserve1;               /* Byte 4/5 */
  UINT  BCC;                  /* Byte 6/7 */

  /* Kennung */
  *(UDINT*)&sLifeCheckRequest.Kennung = *(UDINT*)&arRecvBufLife[0];
  /* Reserve1 */
  *(UINT*)&Reserve1 = *(UINT*)&arRecvBufLife[4];
  sLifeCheckRequest.Reserve1 = SwapUintForIntel(Reserve1);  
  /* BCC */
  *(UINT*)&BCC = *(UINT*)&arRecvBufLife[6];
  sLifeCheckRequest.BCC = SwapUintForIntel(BCC);  
}


/****************************************************************************************************************
* --- Übertragen eines LifeCheckResponse-Telegramms                   
* --- aus der Struktur "LifeCheckResponse" in den Sendebuffer               
* --- mit Eintragung der Checksum                             
*****************************************************************************************************************/

void CopyLifeCheckResponseToSendBuffer()
{

  UINT  ClientStatus;             /* Byte 4/5 */
  UINT  BCC;                  /* Byte 6/7 */
  UINT  BCCraw;

  /* Sendebuffer löschen */
  brsmemset((UDINT) &arSendBufLife[0], 0, sizeof(arSendBufLife));
  /* Kennung */
  *(UDINT*)&arSendBufLife[0] = *(UDINT*)&sLifeCheckResponse.Kennung;
  /* ClientStatus */
  ClientStatus = SwapUintForIntel(sLifeCheckResponse.ClientStatus);
  *(UINT*)&arSendBufLife[4] = *(UINT*)&ClientStatus;
  /* BCC */
  BCCraw = CRCcheck((UDINT*) &arSendBufLife, 6);
  sLifeCheckResponse.BCC = BCCraw;
  BCC = SwapUintForIntel(BCCraw);
  *(UINT*)&arSendBufLife[6] = *(UINT*)&BCC;

}


/****************************************************************************************************************
*   Löschen eines vorhandenen Clients
*****************************************************************************************************************/

UINT DeleteExistingClient(
  INT iClientNr)    /* Nr. innerhalb der Clientverwaltungsliste */
{
  UINT  usPVCnt = 0;
  UINT  usClientCnt = 0;
  PVListEntry_typ *pPVListTmp = NULL;
  BOOL  bClientFound = FALSE;
  UINT  usClientCfgCnt = 0;
  UINT  RetStatus = 0;
	
  if (iClientNr == 0)
  {
    return 3;	// index nicht gültig
  }
	
  if (iClientNr > MAX_ANZAHL_CLIENTS)
  {
    return 3;	// index nicht gültig
  }
	
  /* ---------------- vorhandene Variablen durchsuchen ------------------------ */
  /* Schleife über alle Variablen: jede Variable wird durchsucht, ob der Client  eingetragen worden ist */
  for (usPVCnt = 0; usPVCnt < MAX_CLIENT_VARIABLEN; usPVCnt++ )
  { 
    pPVListTmp = pPVListeStart + usPVCnt;
    usClientCfgCnt = 0;
    bClientFound = FALSE;
    
    /* ------------------------- Variable vorhanden --------------------------- */
    if ((pPVListTmp->pPv != NULL) || (pPVListTmp->pPv != NULL))
    {
      /* ------------------ Client - Liste komplett durchsuchen ------------- */
      for (usClientCnt = 0; usClientCnt < MAX_ANZAHL_CLIENTS; usClientCnt++)
      {
        /* ------------ gültige PV vorhanden, ansonsten zur nächsten PV --- */
        if (pPVListTmp->Client[usClientCnt].iClntIndex == iClientNr)
        {
          pPVListTmp->Client[usClientCnt].iClntIndex = 0;
          pPVListTmp->Client[usClientCnt].uiClntPvIndex = 0;
          bClientFound = TRUE;
          // Diagnose
          Diagnose.Base.RegisterPv.DelClient++;
          Diagnose.Base.RegisterPvX.DelClient++;
          RetStatus = 1;
        }
        /* trotzdem weitersuchen: ziel ist rauszufinden wieviele weitere clients bedient werden müssen */
        else if (pPVListTmp->Client[usClientCnt].iClntIndex > 0)
        {
          /* -- wieviele Clients wurden gefunden: wenn Client einmal gefunden wurde UND --- */
          /* -- ansonsten keine Clients vorhanden sind, so kann die PV gelöscht werden ---  */
          usClientCfgCnt++;
        } 
      }
    }
    
    /* ------ keine Clients gefunden -> PV kann komplett gelöscht werden -------------- */
    if ((usClientCfgCnt == 0) && (bClientFound == TRUE))
    {
      /* --------------------- PV entfernen ----------------------------------------- */
      brsmemset((UDINT) pPVListTmp, 0, sizeof(PVListEntry_typ)); 
      RetStatus = 2;
    }
  }
	
  /* --------------------- Client entfernen -----------------------------------------  */
  brsmemset((UDINT) (pClientListeStart + iClientNr - 1), 0, sizeof(ClientListeEntry_typ));
	
  /* -------------------------------- Pipe leeren ------------------------------------ */
  ResetPipe(iClientNr);
	
  return RetStatus;
}


/****************************************************************************************************************
* --- Zurücksetzen einer Pipe
*****************************************************************************************************************/

void ResetPipe(
INT iClientNr)    /* Clientnr. */
{
  /* --- Lese / Schreibzeiger neu initialisieren ---------- */
  pReadPipe[iClientNr - 1] = (PipeManagement_typ*) PipeStartAdress[iClientNr - 1];
  pWritePipe[iClientNr - 1] = (PipeManagement_typ*) PipeStartAdress[iClientNr - 1];
  
  /* --- Pipespeicher rücksetzen -------------------------- */
  brsmemset((UDINT) PipeStartAdress[iClientNr - 1], 0, sizeof(PipeManagement_typ) * sPtopConf.uiPipeLen);
  
  /* --- Pipeauswertung zurücksetzen*/
  PipeLock[iClientNr - 1].Lock = 0;
  PipeLock[iClientNr - 1].PVIndex = 0;
  
  return;
}


/****************************************************************************************************************
* --- Generierung einer Telegramm - Kennung für Conn
*****************************************************************************************************************/

void GenerateConnHeaderTelegramm(
UDINT *pKennung,
USINT iModus)  /* Kennung  */
{
  if (iModus == MODE_NEW)
  {
    brsmemcpy((UDINT) pKennung, (UDINT) TELEGRAMM_IDENT_CONX, 4);  
  }
  else if (iModus == MODE_OLD)
  {
    brsmemcpy((UDINT) pKennung, (UDINT) TELEGRAMM_IDENT_CONN, 4);
  }
  
}


/****************************************************************************************************************
* --- Generierung einer Telegramm - Kennung für Open
*****************************************************************************************************************/

void GenerateOpenHeaderTelegramm(
UDINT *pKennung, 
USINT iModus)  /* Kennung  */
{
  if (iModus == MODE_NEW)
  {
    brsmemcpy((UDINT) pKennung, (UDINT) TELEGRAMM_IDENT_OPEX, 4);
  }
  else if (iModus == MODE_OLD)
  {
    brsmemcpy((UDINT) pKennung, (UDINT) TELEGRAMM_IDENT_OPEN, 4);
  }
}


/****************************************************************************************************************
* --- Überprüfung des Bufferinhalts
*****************************************************************************************************************/

BOOL ValidateDataFromRecvBuf(
UDINT *pRecvBufOpen,
UINT uiTelLen)
{
UINT uiCrcCheck = 0;
UINT uiCrcFromTel = 0;
  
  uiCrcCheck = CRCcheck((UDINT*) pRecvBufOpen, uiTelLen - 2);
  brsmemcpy((UDINT) &uiCrcFromTel, (UDINT) pRecvBufOpen + uiTelLen - 2, 2);
  
  if (uiCrcCheck == uiCrcFromTel)
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}


/****************************************************************************************************************
* --- Ermittlung der Basisdaten aus Empfangsbuffer - OPEX
*****************************************************************************************************************/

void GetMainDataFromRecvBufOpen(
UDINT* pRecvBufOpen, 
UINT*  puiPvCnt,
UINT*  puiClientIndex)
{
  /* --- Anzahl der Variablen im Telegramm ermitteln ------------------------- */
  brsmemcpy((UDINT) puiPvCnt, (UDINT) pRecvBufOpen + 4, 2);
  
  /* --- Anzahl der Variablen im Telegramm ermitteln ------------------------- */
  brsmemcpy((UDINT) puiClientIndex, (UDINT) pRecvBufOpen + 6, 2);  
}


/****************************************************************************************************************
* --- Ermittlung des nächsten PV Eintrags aus Empfangsbuffer - OPEX
*****************************************************************************************************************/

BOOL GetDataFromRecvBufOpen(
UDINT* pRecvBufOpen,      /* Empfangsbuffer*/
UINT uiGetMaxPv,        /* Anzahl der Variablen im Telegramm */
PVOpenRequestX_typ* pPVOpenRequestX, /* ein Variableneintrag */
UINT* puiPvEntry)
{
USINT* pusStartData = NULL;
PVOpenRequestX_typ* pGetData = NULL;

  /* --- auf Start der eigentlichen Daten verweisen: +4(Kennung); +2(Variable) */
  pusStartData = (USINT*) pRecvBufOpen + 4 + 2;
  
  if (uiGetMaxPv > *puiPvEntry)
  {
    pGetData = (PVOpenRequestX_typ*) pusStartData + (*puiPvEntry);
    if (Diagnose.LogPointerInfo == TRUE)
    {
      ValidatePointerServer((UDINT) pGetData, 18, (MemManagerDiag_type*) &sMemDiag, 1);
    }
    
    brsmemcpy((UDINT) pPVOpenRequestX, (UDINT) pGetData, sizeof(PVOpenRequestX_typ));
    (*puiPvEntry)++;
    return TRUE;
  } 
  else
  {
    return FALSE;
  }
}


/****************************************************************************************************************
*   Existiert PV in Liste (nächsten Eintrag ermitteln) - neue Funktion 30.03.2021
*   Identifizierung wird jetzt über Variablenname gemacht
*****************************************************************************************************************/

UINT ExistsPvInListNew(
  UDINT udNamePv,         /* Name der neuen Variablen */
  UINT  uiClientIndex,    /* ClientIndex von Client*/
  UINT* puiPvOffset,      /* Offset für Pv in Liste */
  UINT* puiClntOffset)    /* Offset für Client in Liste */
{
  UINT uiLoopPv = 0;
  UINT uiLoopClient = 0;
  PVListEntry_typ* pSearchPvList = NULL;
  UINT uiRetVal = 0;
  BOOL bListPvEntryNew = FALSE;   /* neue Variable in Liste eintragen */
  BOOL bPvEntryClient = FALSE;    /* bei vorhandener Variable neuen Client eintragen */
  BOOL bTmpNoPlaceForNewClient = FALSE;
  
  /* --- 
    Retval: 
    0 ... Variable wird neu in Liste eingetragen
    1 ... neuer Client für bestehende Variable
    2 ... kein Platz mehr für Variableneintrag
  */

  /* --- Vergleich der vorhandenen PVs mit der gerade anzumeldenden PV --- */
  for (uiLoopPv = 0; uiLoopPv < MAX_CLIENT_VARIABLEN; uiLoopPv++)
  {
    pSearchPvList = pPVListeStart + uiLoopPv;
    if (Diagnose.LogPointerInfo == TRUE)
    {
      ValidatePointerServer((UDINT) pSearchPvList, 26, (MemManagerDiag_type*) &sMemDiag, 1);
    }
  
    if (pSearchPvList->Name[0] != 0)  // Variable vorhanden ?
    {
      if (brsstrcmp((UDINT) &pSearchPvList->Name, udNamePv) == 0)  // Variable identisch
      {
        for (uiLoopClient = 0; uiLoopClient < MAX_ANZAHL_CLIENTS; uiLoopClient++ )
        {
          /* --- Client befindet sich bereits in Verwaltung -> Ende --------------  */
          if (pSearchPvList->Client[uiLoopClient].iClntIndex == uiClientIndex)
          {
            *puiPvOffset   = uiLoopPv;
            *puiClntOffset = uiLoopClient;
            bPvEntryClient = TRUE;
            break;
          }
            /* --- erster leerer Eintrag wird abgespeichert (für Eintrag, falls PV noch nicht vorhanden) ------------ */
          else if ((pSearchPvList->Client[uiLoopClient].iClntIndex == 0) && (bPvEntryClient == FALSE))
          {
            *puiPvOffset   = uiLoopPv;
            *puiClntOffset = uiLoopClient;
            bPvEntryClient = TRUE;
          }
        }
        
        /* --- Ende Schleife: kein Platz mehr für neuen Client ? ------------------- */
        if (bPvEntryClient == FALSE) 
        {
          bTmpNoPlaceForNewClient = TRUE;    
        }
      }      
    }
    else if (pSearchPvList->Name[0] == 0)   // erster freier Platz in Variablenliste
    {
      /* --- erste gefundene leere Stelle abspeichern -------------------------- */
      if (bListPvEntryNew == FALSE)
      {
        *puiPvOffset  = uiLoopPv;
        *puiClntOffset  = 0;
        bListPvEntryNew = TRUE;
      }
    }
    
    if ((bPvEntryClient == TRUE) || (bTmpNoPlaceForNewClient == TRUE))
    {
      break;
    }  
  }
  
  /* --- Zuweisung der Funktionsblock - Stati -----------------------------  */ 
  /* --- Eintrag ist möglich --------------------------------------------------------- */
  if ((bPvEntryClient == TRUE) || (bListPvEntryNew == TRUE))
  {
    /* --- komplett neuer Eintrag -------------------------------------------------- */
    if (bListPvEntryNew == TRUE)
    {
      uiRetVal = 1;
    }

    /* --- neuen Client in Liste eintragen ----------------------------------------- */
    if (bPvEntryClient == TRUE)
    {
      uiRetVal = 2;
    }
  }
    
  /* --- Ermittlung, ob in der PV - Liste noch Platz vorhanden ist -------------------- */
  if ((uiLoopPv >= MAX_CLIENT_VARIABLEN) && (bListPvEntryNew == FALSE) && (bPvEntryClient == FALSE))
  {
    /* --- kein Platz für neue Variable --------------------------------------------- */
    if (bListPvEntryNew == FALSE)
    {
      uiRetVal = 3;
    }
      /* --- kein Platz für neuen Client f. vorhandene Variable ----------------------- */
    else if (bPvEntryClient == FALSE)
    {
      uiRetVal = 4;     
    }     
  }
  
  return uiRetVal;
}


/****************************************************************************************************************
* --- Existiert PV in Liste (nächsten Eintrag ermitteln)
*****************************************************************************************************************/

UINT ExistsPvInList(
UDINT udAdrPv,        /* Adresse der Variable */
UINT  uiClientIndex,    /* ClientIndex von Client*/
UINT* puiPvOffset,      /* Offset für Pv in Liste */
UINT* puiClntOffset)    /* Offset für Client in Liste */
{
UINT uiLoopPv = 0;
UINT uiLoopClient = 0;
PVListEntry_typ* pSearchPvList = NULL;
UINT uiRetVal = 0;
BOOL bListPvEntryNew = FALSE;   /* neue Variable in Liste eintragen */
BOOL bPvEntryClient = FALSE;    /* bei vorhandener Variable neuen Client eintragen */
BOOL bTmpNoPlaceForNewClient = FALSE;

  /* --- 
    Retval: 
    0 ... Variable wird neu in Liste eingetragen
    1 ... neuer Client für bestehende Variable
    2 ... kein Platz mehr für Variableneintrag
  */

  /* --- Vergleich der vorhandenen PVs mit der gerade ermittelten PV --- */
  for (uiLoopPv = 0; uiLoopPv < MAX_CLIENT_VARIABLEN; uiLoopPv++)
    {
      pSearchPvList = pPVListeStart + uiLoopPv;
      if (Diagnose.LogPointerInfo == TRUE)
      {
        ValidatePointerServer((UDINT) pSearchPvList, 26, (MemManagerDiag_type*) &sMemDiag, 1);
      }
      
      /* --- gleiche PV gefunden -> Eintrag in Client Liste der PV ---------- */
      if (pSearchPvList->pPv == udAdrPv)
      {
        for (uiLoopClient = 0; uiLoopClient < MAX_ANZAHL_CLIENTS; uiLoopClient++ )
        {
          /* --- Client befindet sich bereits in Verwaltung -> Ende --------------  */
          if (pSearchPvList->Client[uiLoopClient].iClntIndex == uiClientIndex)
          {
            *puiPvOffset   = uiLoopPv;
            *puiClntOffset = uiLoopClient;
            bPvEntryClient = TRUE;
            break;
          }
          /* --- erster leerer Eintrag wird abgespeichert (für Eintrag, falls PV noch nicht vorhanden) ------------ */
          else if ((pSearchPvList->Client[uiLoopClient].iClntIndex == 0) && (bPvEntryClient == FALSE))
          {
            *puiPvOffset   = uiLoopPv;
            *puiClntOffset = uiLoopClient;
            bPvEntryClient = TRUE;
          }
        }

        /* --- Ende Schleife: kein Platz mehr für neuen Client ? ------------------- */
        if (bPvEntryClient == FALSE) 
        {
          bTmpNoPlaceForNewClient = TRUE;
        }
      }
      else if (pSearchPvList->pPv == NULL_ADR )
      {
        /* --- erste gefundene leere Stelle abspeichern -------------------------- */
        if (bListPvEntryNew == FALSE)
        {
          *puiPvOffset  = uiLoopPv;
          *puiClntOffset  = 0;
          bListPvEntryNew = TRUE;
        }
      }
            
      if ((bPvEntryClient == TRUE) || (bTmpNoPlaceForNewClient == TRUE))
      {
        break;
      }
    }
    
    /* --- Zuweisung der Funktionsblock - Stati -----------------------------  */ 
    /* --- Eintrag ist möglich --------------------------------------------------------- */
    if ((bPvEntryClient == TRUE) || (bListPvEntryNew == TRUE))
    {
      /* --- komplett neuer Eintrag -------------------------------------------------- */
      if (bListPvEntryNew == TRUE)
      {
        uiRetVal = 1;
      }

      /* --- neuen Client in Liste eintragen ----------------------------------------- */
      if (bPvEntryClient == TRUE)
      {
        uiRetVal = 2;
      }
    }
    
    /* --- Ermittlung, ob in der PV - Liste noch Platz vorhanden ist -------------------- */
    if ((uiLoopPv >= MAX_CLIENT_VARIABLEN) && (bListPvEntryNew == FALSE) && (bPvEntryClient == FALSE))
    {
      /* --- kein Platz für neue Variable --------------------------------------------- */
      if (bListPvEntryNew == FALSE)
      {
        uiRetVal = 3;
      }
      /* --- kein Platz für neuen Client f. vorhandene Variable ----------------------- */
      else if (bPvEntryClient == FALSE)
      {
        uiRetVal = 4;     
      }     
    }
    
    return uiRetVal;
    
}


/****************************************************************************************************************
* --- Eintrag einer PV in Liste
*****************************************************************************************************************/

void InsertNewPvInList(
PVOpenRequestX_typ sPVOpenRequestX,   /* Anforderungstelegramm */
UDINT udPvAdrServer,          /* Adresse der PV*/
UDINT udPvLen,              /* Länge der PV */
UINT uiPvOffset,            /* Platz für neue Variable */
UINT uiClntOffset,            /* Platz für neuen Client*/
UDINT* pudVal,              /* Wert der Variable */
BOOL bInsertNew)            /* komplett neu Einfügen (TRUE), oder nur neuen Client anhängen (FALSE) */
{
PVListEntry_typ* pPvListEntry = NULL;
USINT*  pusPvVal = NULL;
UINT*   puiPvVal = NULL;
UDINT*  pudPvVal = NULL;

  pPvListEntry = pPVListeStart + uiPvOffset;

  if (Diagnose.LogPointerInfo == TRUE)
  {
    ValidatePointerServer((UDINT) pPvListEntry, 19, (MemManagerDiag_type*) &sMemDiag, 1);
  }

  /* --- PV kann gleichen Namen haben, aber zur Sicherheit Pointer + Länge neu schreiben ----------- */
  pPvListEntry->pPv   = udPvAdrServer;
  pPvListEntry->PvLaenge  = udPvLen;
  pPvListEntry->Tick.Old  = udTimeTick;
    
  /* --- Neu eintragen ----------------------------------------------------------------- */
  if (bInsertNew == TRUE)
  { 
    pPvListEntry->Tick.Intervall = sPVOpenRequestX.SyncTime;
    pPvListEntry->Hysterese   = sPVOpenRequestX.Hysterese;
    brsstrcpy((UDINT) &pPvListEntry->Name, (UDINT) &sPVOpenRequestX.Variablenname);
    //brsmemcpy((UDINT) &pPvListEntry->Name, (UDINT) &sPVOpenRequestX.Variablenname, sizeof(sPVOpenRequestX.Variablenname));
  }
  /* --- bestehenden Eintrag überarbeiten ----------------------------------------------- */
  else
  {
    /* ---------------------------- Zeitsynchronisation ------------------------------- */    
    if (sPVOpenRequestX.SyncTime > 0)
        {
          if ((pPvListEntry->Tick.Intervall > sPVOpenRequestX.SyncTime) || (pPvListEntry->Tick.Intervall == 0))
          {
            /* kleinere Zeit gewinnt, ausser Zeit = 0, da hiermit Überprüfung ausgeschaltet wird */
            pPvListEntry->Tick.Intervall = sPVOpenRequestX.SyncTime;
          }
        }                     
                                        
    /* --- Hysterese ------------------------------------------------------------------ */
    if (sPVOpenRequestX.Hysterese < pPvListEntry->Hysterese)
    {
      // kleinere Hysterese gewinnt
      pPvListEntry->Hysterese  = sPVOpenRequestX.Hysterese;
    } 
  }    
      
  /* --- Clientnummer eintragen: zu jedem Server gibt es n - Clients -------------------- */
  pPvListEntry->Client[uiClntOffset].iClntIndex    = sPVOpenRequestX.ClientIndex;
  pPvListEntry->Client[uiClntOffset].uiClntPvIndex = sPVOpenRequestX.PVIndex; 

  /* --- PV Access */
  /* --- Aufgrund der Variablenlänge wird der entsprechende Zeiger auf die Adresse gelegt*/
  /* --- nur 1, 2 u. 4 Bytes                                 */
  switch (udPvLen) 
  {
    case 1:
    {
      pusPvVal = (USINT*)  udPvAdrServer;
      pPvListEntry->PvWert = *pusPvVal;
      break;
    }
          
    case 2:
    {
      puiPvVal = (UINT*)  udPvAdrServer;
      pPvListEntry->PvWert = *puiPvVal;
      break;
    }
        
    case 4:
    {
      pudPvVal = (UDINT*)  udPvAdrServer;
      pPvListEntry->PvWert = *pudPvVal;
      break;
    }
  }
    
  /* --- Rückgabe des PV Werts -------------------------------------------------------- */
  *pudVal = pPvListEntry->PvWert;
}


/*****************************************************************************************************************/
/*              Telegramm Kennung ermitteln                         */
/*****************************************************************************************************************/

BOOL CheckTelegramHeader(UDINT* pKennung, STRING* pTelegramHeader )
{
  DINT iCmp;
  
  /* Telegrammheader ist immer 4*/
  
  iCmp = brsmemcmp((UDINT) pKennung, (UDINT) pTelegramHeader, 4);  
  
  if (iCmp == 0)
  {
    return TRUE;  /* ja, Kennung richtig */
  }
  else
  {
    return FALSE;
  }
}


/****************************************************************************************************************
* --- Generierung der Kennung für Antworttelegramm OPEX
*****************************************************************************************************************/

void CreateHeaderInRespBuf(
USINT* pSendBuf,
UINT uiPvCnt,
UINT uiClientIndex,
UINT* puiTelLen)      /* Sendelänge*/
{
USINT* pusSendBuf = NULL;

  pusSendBuf = (USINT*) pSendBuf;
  
  /* --- neues Telegramm -> Buffer löschen -------------------------------------------- */
  brsmemset((UDINT) pusSendBuf, 0, sizeof(MAX_UDP_FRAME_LEN));

  brsmemcpy((UDINT) pusSendBuf, (UDINT) "OPEX", 4);       /* --- Kennung */
  pusSendBuf+=4;

  brsmemcpy((UDINT) pusSendBuf, (UDINT) &uiPvCnt, 2);  /* --- Anzahl der Variablen */
  pusSendBuf+=2;
  
  brsmemcpy((UDINT) pusSendBuf, (UDINT) &uiClientIndex, 2);  /* --- ClientIndex      */
  
  *puiTelLen+=8;
  
}


/****************************************************************************************************************
* --- Eintrag einer PV in das Antworttelegramm OPEX
*****************************************************************************************************************/

void CreateNewEntryInRespBuf(
UDINT* pSendBuf,            /* Zeiger auf Sendebuffer*/
PVOpenRequestX_typ sPVOpenRequestX,   /* Variablenanforderungs - Daten */
UINT uiPvEntry,             /* Eintrags - Nr.*/
UDINT udPvAdrServer,          /* Adresse */
UDINT udPvLen,              /* Länge */
UDINT udVal,              /* Wert */
UINT uiStatus,
UINT* puiSendLen)
{
USINT* pusSendBuf = NULL;
PVOpenResponseX_typ* pPvOpenResp = NULL;

  pusSendBuf = (USINT*) pSendBuf;
  
  /* --- um Kennung, Variablenanzahl, ClientIndex verschieben */
  pusSendBuf+=8;

  pPvOpenResp = (PVOpenResponseX_typ*) pusSendBuf + uiPvEntry;

  if (Diagnose.LogPointerInfo == TRUE)
  {
    ValidatePointerServer((UDINT) pPvOpenResp, 25, (MemManagerDiag_type*) &sMemDiag, 1);
  }
  
  pPvOpenResp->Status  = uiStatus;
  pPvOpenResp->PVIndex = sPVOpenRequestX.PVIndex;
  
  if (uiStatus == STATUS_SERV_OK)
  {
    pPvOpenResp->PVIdent = udPvAdrServer;
    pPvOpenResp->Laenge  = udPvLen;
    pPvOpenResp->Wert    = udVal;
  }
  else
  {
    pPvOpenResp->PVIdent = 0;
    pPvOpenResp->Laenge  = 0;
    pPvOpenResp->Wert    = 0; 
  }
  
  *puiSendLen+=sizeof(PVOpenResponseX_typ);
}


/****************************************************************************************************************
* --- Erzeugung der CRC + Eintragen in Buffer
*****************************************************************************************************************/

void CreateCrcInRespBuf(
UDINT* pSendBuf, 
UINT* puiSendTelLen)
{
USINT* pusSendBuf = NULL;
UINT uiBCCVal = 0;

  pusSendBuf = (USINT*) pSendBuf;
  uiBCCVal = CRCcheck(pSendBuf, *puiSendTelLen);
  pusSendBuf += *puiSendTelLen;
  
  brsmemcpy((UDINT) pusSendBuf, (UDINT) &uiBCCVal, 2);
  (*puiSendTelLen)+=2;
}


/****************************************************************************************************************
* Ermittlung der Daten innerhalb der Pipe
*****************************************************************************************************************/

UINT GetPipeInfo(
UINT uiPipeNr)
{
UINT uiCnt;
UDINT udFreeSpace;
uiCnt = 0;

  if (pReadPipe[uiPipeNr] != pWritePipe[uiPipeNr])
  {
    if (pWritePipe[uiPipeNr] > pReadPipe[uiPipeNr])
    {
	    udFreeSpace = pWritePipe[uiPipeNr] - pReadPipe[uiPipeNr];
	    uiCnt = udFreeSpace / sizeof(PipeManagement_typ);
    }
    else
    {
    
	    udFreeSpace = ((PipeStopAdress[uiPipeNr] - (UDINT) pReadPipe[uiPipeNr]) + ((UDINT) pWritePipe[uiPipeNr] - PipeStartAdress[uiPipeNr]));
  	    uiCnt = udFreeSpace / sizeof(PipeManagement_typ);
    }
  }
  return uiCnt;
}


/****************************************************************************************************************
* --- Erzeugung der CRC + Eintragen in Buffer
*****************************************************************************************************************/

BOOL GetEventPipeDataNewMode(
EventSend_typ* pEventSend,    /* Verwaltungsstruktur  */
UDINT* pSendBuf,        /* Adresse des Sendebuffers */
UINT* puiSendTelLen,      /* Telegrammlänge */
UINT* puiClientNr)        /* Clientnr. für Zeigeroffset*/
{
UINT uiGetValPipe = 0;
PipeManagement_typ sPipeValue;
USINT* pusSendBuf = NULL;
UINT* puiPvCnt = NULL;
UINT* puiEventCnt = NULL;
PVEventX_typ sPvEvent;
BOOL bEventFound = FALSE;
UINT uiPvCnt = 0;
BOOL bBuildOnce = FALSE;

  /* ... Eintrag aus Pipe lesen -> Ende ODER MAX_TELEGRAMM */
  /* ... Rückgabe des Sendebuffers */

  pusSendBuf = (USINT*) pSendBuf;
  
  /* --- Events aus Pipe lesen -------------------------------------------------------------------------- */
  do
  {
    Diagnose.Base.Events.TryToReadFromPipe++;
    Diagnose.Base.PvCheck[pEventSend->PipeCnt].TryToReadFromPipe++;
    
    uiGetValPipe = ReadDataFromPipe(PipeStopAdress[pEventSend->PipeCnt], PipeStartAdress[pEventSend->PipeCnt], 
             &pReadPipe[pEventSend->PipeCnt], pWritePipe[pEventSend->PipeCnt], &sPipeValue);
  
    if (uiGetValPipe == 0)
    {
      bEventFound = TRUE;
      uiPvCnt++;
      
      /* --- einmalig Telegrammheader bauen --------------------------------------------------------- */
      if (bBuildOnce == FALSE)
      {
        bBuildOnce = TRUE;
        brsmemset((UDINT) pusSendBuf, 0, MAX_UDP_FRAME_LEN);
        
        /* --- Kennung ----------------------------------------------------------- */
        brsmemcpy((UDINT) pusSendBuf, (UDINT) "EVNX", 4); 
        pusSendBuf+=4;
        *puiSendTelLen+=4;
        
        /* --- Pointer f. Anzahl der Variablen ----------------------------------- */
        puiPvCnt = (UINT*) pusSendBuf;
        pusSendBuf+=2;
        *puiSendTelLen+=2;
        
        /* --- Pointer f. EventCnt ----------------------------------------------- */
        puiEventCnt = (UINT*) pusSendBuf;
        pusSendBuf+=2;
        *puiSendTelLen+=2;        
      }

      Diagnose.Base.Events.ReadFromPipe++;
      Diagnose.Base.Events.WriteToEvent++;

      Diagnose.Base.PipeRead[pEventSend->PipeCnt]++;
      
      /* --- Daten in Telegrammstruktur kopieren ------------------------------------- */
      sPvEvent.PVIndex = sPipeValue.PVIndex;
      sPvEvent.Laenge  = sPipeValue.Laenge;
      sPvEvent.Wert    = sPipeValue.Wert;
      
      brsmemcpy((UDINT) pusSendBuf, (UDINT) &sPvEvent, sizeof(sPvEvent));
      pusSendBuf+=sizeof(sPvEvent);
      *puiSendTelLen+=sizeof(sPvEvent);
      
      /* --- Abbruchbedingung, falls maximale Framelänge überschritten wird ----------- */
      if ((*puiSendTelLen + sizeof(sPvEvent) + 2) >= MAX_UDP_FRAME_LEN)
      {
        uiGetValPipe = 1;
      }
    }

  } while (uiGetValPipe == 0);
  
  
  if (bEventFound)
  {    
    *puiPvCnt    =  uiPvCnt;
    *puiEventCnt = pEventSend->AbbildCnt;
    *puiClientNr = pEventSend->PipeCnt;

    /* --- BCC ------------------------------------------------------------------------- */
    CreateCrcInRespBuf((UDINT*) pSendBuf, puiSendTelLen);
    
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}


/****************************************************************************************************************
* --- Erzeugung der CRC + Eintragen in Buffer
*****************************************************************************************************************/

BOOL GetEventPipeDataOldMode(
  EventSend_typ* pEventSend,    /* Verwaltungsstruktur  */
  UINT* puiClientNr,             /* Clientnr. für Zeigeroffset*/
  PVEvent_typ* pEvent)
{
  UINT uiGetValPipe = 0;
  PipeManagement_typ sPipeValue;  
  BOOL bEventFound = FALSE;
  

  /* ... Eintrag aus Pipe lesen -> Ende ODER MAX_TELEGRAMM */
  /* ... Rückgabe des Sendebuffers */

  /* --- Events aus Pipe lesen -------------------------------------------------------------------------- */
  Diagnose.Base.Events.TryToReadFromPipe++;
  Diagnose.Base.PvCheck[pEventSend->PipeCnt].TryToReadFromPipe++;
  
  uiGetValPipe = ReadDataFromPipe(PipeStopAdress[pEventSend->PipeCnt], PipeStartAdress[pEventSend->PipeCnt], 
                    &pReadPipe[pEventSend->PipeCnt], pWritePipe[pEventSend->PipeCnt], &sPipeValue);

  if (uiGetValPipe == 0)
  {
    bEventFound = TRUE;

    Diagnose.Base.Events.ReadFromPipe++;
    Diagnose.Base.Events.WriteToEvent++;

    Diagnose.Base.PipeRead[pEventSend->PipeCnt]++;
    *puiClientNr = pEventSend->PipeCnt; // welcher Client ist betroffen...
    
    /* --- Daten in Telegrammstruktur kopieren ------------------------------------- */
    brsmemcpy((UDINT) &pEvent->Kennung, (UDINT) TELEGRAMM_IDENT_EVNT, 4);
    pEvent->PVIndex = sPipeValue.PVIndex;
    pEvent->Laenge  = sPipeValue.Laenge;
    pEvent->Wert    = sPipeValue.Wert;
    pEvent->EventCnt = pEventSend->AbbildCnt;
  }

  return bEventFound;  
}


/*****************************************************************************************************************
* SetPtrToNull: alle Pointer auf NULL setzen, da Datobj weg!
*****************************************************************************************************************/

void SetPtrToNull()
{
  pPVListeStart = NULL;
  pClientListeStart = NULL;
  pClientListeWork = NULL;
  pClientListeLiveCheck= NULL;

  pSendBufOpen = NULL;
  pRecvBufOpen = NULL;
  pSendBufEvent = NULL;
  pRecvBufEvent = NULL;
  
  pPipeMonitor = NULL;
}


/*****************************************************************************************************************
* CreateDataObjMem: Datenobjekt erzeugen (ausschließlich im INIT verwenden!)
*****************************************************************************************************************/

UINT CreateDataObjMem(
MemManagerServ_type* pMemManger,
DatObjCreate_typ  *pDataObjCreateFub,
DatObjInfo_typ    *pDatObjInfoFub,
DatObjDelete_typ  *pDatObjDeleteFub
)
{
BOOL bCreate = FALSE;
  
  /* --- Info über Modul anfordern --------------------------------- */
  pDatObjInfoFub->enable = TRUE;
  pDatObjInfoFub->pName  = (UDINT) DATA_OBJ_MEM_NAME_SERVER;
    
  do
  {
    DatObjInfo(pDatObjInfoFub);
  }while (pDatObjInfoFub->status == 0xFFFF);
  
  switch (pDatObjInfoFub->status)
  {
    case 0: /* ERR_OK: */
    {
      /* --- Datenobjekt gefunden  -> löschen  */
      pDatObjDeleteFub->enable = TRUE;
      pDatObjDeleteFub->ident = pDatObjInfoFub->ident;
      do
      {
        DatObjDelete(pDatObjDeleteFub);     
      }while (pDatObjDeleteFub->status == 0xFFFF);
      
      /* --- Löschen ok ----------------------------------------- */
      if (pDatObjDeleteFub->status == 0)
      {
        bCreate = TRUE;
        WriteLoggerMessage((Logger_typ*) &sLogger, 0xF000, (STRING*) "DataObj gelöscht", 0x7FFFFFFF, PTOP_LOG_SEVERITY_INFO);
      }
      else
      {
        WriteLoggerMessage((Logger_typ*) &sLogger, 0xF000, (STRING*) "Fehler bei DataObj löschen", pDatObjDeleteFub->status, PTOP_LOG_SEVERITY_INFO);
        return pDatObjDeleteFub->status;
      }
    }
    break;
    
    case 20605: /*  doERR_ILLOBJECT: */
    case 20606: /* doERR_ILLOBJTYPE: */
    case 20609: /* doERR_MODULNOTFOUND: */
      /* --- Datenobjekt nicht vorhanden -> erzeugen */   
      bCreate = TRUE;
      break;
  }
  
  if (bCreate)
  {
    do
    {
      pDataObjCreateFub->enable   = TRUE;
      pDataObjCreateFub->len    = pMemManger->udMemory;
      pDataObjCreateFub->pName  = (UDINT) DATA_OBJ_MEM_NAME_SERVER;
      pDataObjCreateFub->MemType  = 65; // Achtung: eventuell wieder auf doTemp rückändern!
      pDataObjCreateFub->Option   = 1;
      pDataObjCreateFub->pCpyData = 0;
    
      DatObjCreate(pDataObjCreateFub);
    } while (pDataObjCreateFub->status == 0xFFFF);
    
    if (pDataObjCreateFub->status == 0)
    {
      pMemManger->udDOPtr = pDataObjCreateFub->pDatObjMem;
      pMemManger->udIdent = pDataObjCreateFub->ident;
      WriteLoggerMessage((Logger_typ*) &sLogger, 0xF000, (STRING*) "DataObj created", 0x7FFFFFFF, PTOP_LOG_SEVERITY_INFO);
    }
    else
    {
      WriteLoggerMessage((Logger_typ*) &sLogger, 0xF000, (STRING*) "Error creating DataObj", pDataObjCreateFub->status, PTOP_LOG_SEVERITY_INFO);
    }
  }
  
  
  return pDataObjCreateFub->status;
}


/*****************************************************************************************************************
* ValidatePointerServer: Überprüfung - nur als Debugfunktion für V 3.07.x 
*****************************************************************************************************************/

UDINT ValidatePointerServer(
UDINT udAdr,        /* zu überprüfende Adresse   */
UINT  uiLocation,   /* Position der Überprüfung - muss eindeutig sein!  */
MemManagerDiag_type* pMemDiag,	/* Speicherbereiche */
UINT  uiTypeOfMem
)
{
 USINT  Messagestring1[64];
 USINT  Messagestring2[64];
 BOOL bNullPointer = FALSE;
 BOOL bSysPointer = FALSE;
 BOOL bErrPointer = FALSE;
 BOOL bErr = FALSE;
 
  if (udAdr == NULL_ADR)
  {
    bNullPointer = TRUE;
    bErr = TRUE;
  }
  else if ((udAdr > NULL_ADR) && (udAdr <= 0x3FFF))
  {  
    bSysPointer = TRUE;
    bErr = TRUE;    
  }
  else
  {
    /* --- Basisprüfung überstanden: weiter */
    
    if (uiTypeOfMem == 1)
    {
      /* Adresse ausserhalb des allokierten Bereichs */
      if ((udAdr < pMemDiag->udStartDM) || (udAdr > pMemDiag->udStopDM))
      {
        bErrPointer = TRUE;
        bErr = TRUE;
      }
    }
  }
  
  /*  --- Auswertung ---------------------- */  
  if (bErr == TRUE)
  {
  
  	/* Ergebnis: */
    /* "server#1 Pointer im falschen Bereich " */
    /* "server#2 Adresse - 938204402" */
    /* "server#3 Start: 120230: Stop: 34093402 */
    
  	if (bNullPointer == TRUE)
  	{
      WriteLoggerMessage((Logger_typ*) &sLogger, 0xF000, (STRING*) "server#1: Pointer auf NULL", uiLocation, PTOP_LOG_SEVERITY_INFO);
  	}
  	else if (bSysPointer == TRUE)
  	{
      WriteLoggerMessage((Logger_typ*) &sLogger, 0xF000, (STRING*) "server#1: Pointer im System Bereich", uiLocation, PTOP_LOG_SEVERITY_INFO);
  	}
  	else if (bErrPointer == TRUE)
  	{
      WriteLoggerMessage((Logger_typ*) &sLogger, 0xF000, (STRING*) "server#1: Pointer im falschen Bereich", uiLocation, PTOP_LOG_SEVERITY_INFO);
  	}
  	
  	Messagestring2[0] = 0;
  	brsitoa(udAdr, (UDINT) &Messagestring2);

  	Messagestring1[0] = 0;
  	brsstrcpy((UDINT) &Messagestring1, (UDINT) "server#2: Adresse - ");
  	brsstrcat((UDINT) &Messagestring1, (UDINT) &Messagestring2);
    WriteLoggerMessage((Logger_typ*) &sLogger, 0xF000, (STRING*) &Messagestring1, uiLocation, PTOP_LOG_SEVERITY_INFO);
    
    Messagestring1[0] = 0;
    Messagestring2[0] = 0;
    brsstrcpy((UDINT) &Messagestring1, (UDINT) "server#3: Start: ");
    brsitoa(pMemDiag->udStartDM, (UDINT) &Messagestring2);
    brsstrcat((UDINT) &Messagestring1, (UDINT) &Messagestring2);
    brsstrcat((UDINT) &Messagestring1, (UDINT) " ;Ende: ");
    Messagestring2[0] = 0;
    brsitoa(pMemDiag->udStopDM, (UDINT) &Messagestring2);
    brsstrcat((UDINT) &Messagestring1, (UDINT) &Messagestring2);
    WriteLoggerMessage((Logger_typ*) &sLogger, 0xF000, (STRING*) &Messagestring1, uiLocation, PTOP_LOG_SEVERITY_INFO);
    
    return NULL_ADR;
  
  }
  
  return udAdr;
  
}


/* Ermittlung des ptop Modus */
USINT SetMode(
UDINT* pTelegrammKennung
)
{
  USINT iMode;
  iMode = MODE_NEW;
  
  if (CheckTelegramHeader((UDINT*) pTelegrammKennung, (STRING*) TELEGRAMM_IDENT_CONN) == TRUE)
  {
    iMode = MODE_OLD;
  }
  else if (CheckTelegramHeader((UDINT*) pTelegrammKennung, (STRING*) TELEGRAMM_IDENT_CONX) == TRUE)
  {
    iMode = MODE_NEW;
  }
  
  return iMode;
}


/* Eintrag in das ptop Logbuch mit 4 einzelnen Texten welche mit '||' zusammengesetzt werden */
DINT WriteDiagDataToBuRLoggerNormal(
  BOOL EnableWrite,   /* enable Function Call*/
  char* pTask,        /* name of task - Entry#1 */
  char* pFubName,     /* name of calling function - Entry#2*/
  char* pStepName,    /* name of procedure step - Entry#3*/
  char* pFreeText,    /* free text - Entry#3*/
  UDINT iState)       /* State */
{
  USINT HelpString1[255];
  USINT HelpString2[6];
  USINT HelpString3[5];
  DINT iRetVal;
  
  if (EnableWrite == FALSE)
  {    
    return 0;
  }
  
  if ((pTask == NULL) || (pFubName == NULL) || (pStepName == NULL) || (pFreeText == NULL))
  {    
    return 1000;  /* nullpointer */
  }
  
  
  HelpString1[0] = 0;
  HelpString2[0] = 0;
  HelpString3[0] = 0;
  brsstrcpy((UDINT) &HelpString3, (UDINT) " || ");
  
  brsstrcpy((UDINT) &HelpString1, (UDINT) pTask);
  brsstrcat((UDINT) &HelpString1, (UDINT) &HelpString3);
  brsstrcat((UDINT) &HelpString1, (UDINT) pFubName);
  brsstrcat((UDINT) &HelpString1, (UDINT) &HelpString3);
  brsstrcat((UDINT) &HelpString1, (UDINT) pStepName); 
  
  if (brsstrlen((UDINT) pFreeText) != 0)
  {
    brsstrcat((UDINT) &HelpString1, (UDINT) &HelpString3);
    brsstrcat((UDINT) &HelpString1, (UDINT) pFreeText);
  }
  
  brsstrcat((UDINT) &HelpString1, (UDINT) &HelpString3);
  
  brsitoa((DINT) iState, (UDINT) &HelpString2);
  brsstrcat((UDINT) &HelpString1, (UDINT) &HelpString2); 
  iRetVal = WriteLoggerMessage((Logger_typ*) &sLogger, 0xF000, (STRING*) &HelpString1, 0x7FFFFFFF, PTOP_LOG_SEVERITY_INFO);
  
  return iRetVal;
}


// get the correct difference from 2 UDINT Values: needed for variable checking
UDINT GetAbsoluteValue(UDINT NewValue, UDINT OldValue)
{
  UDINT ReturnValue = 0;
  
  if (NewValue == OldValue)
  {
    ReturnValue = 0;
  }
  else if (NewValue > OldValue)
  {
    ReturnValue = NewValue - OldValue;
  }
  else if (OldValue > NewValue)
  {
    ReturnValue = OldValue - NewValue;
  }
  
  return ReturnValue;
}


/* Funktion zur Protokollierung von Schritten:
1. Schritte vollständig protokollieren		Func: CyclicEvents || EVENT_SCHRITT_INIT
2. Datenverteilung (Eventbetrieb)
*/
UINT WriteStepDataToBuRLogger(
  Logger_typ* pLogger,	/* Main Part*/
  ServBurLogCtrlStepsInt_typ* pDetailLogger,
  ServBurLog_typ* 	pLogManager,
  char* pFubName,     /* name of calling function*/
  char* pStepName,    /* name of procedure step*/
  DINT Step,
  UDINT ActTime	/* Act time */
  )
{
  USINT HelpString1[255];
  USINT HelpString3[5];
  UINT iRetVal = 0;
  UINT iStrLen;
	
  HelpString1[0] = 0;
  HelpString3[0] = 0;
  brsstrcpy((UDINT) &HelpString3, (UDINT) " || ");
	
  // main and detail must be true for logging	
  if (pLogManager->StartLogging != pLogManager->Internal.StartLogging)
  {
    if (pLogManager->StartLogging == TRUE)
    {
      // save starting time
      pLogManager->Internal.ActTime = ActTime;

      // Logger is started; just enter it in the logger once
      brsstrcpy((UDINT) HelpString1, (UDINT) START_LOG_TXT);
      brsstrcat((UDINT) HelpString1, (UDINT) &pLogManager->Title);
      pLogger->Fubs.ArEventLogWrite_Fub.AddData = (UDINT) &HelpString1;
      iStrLen = brsstrlen((UDINT) &HelpString1);
   
      pLogger->Fubs.ArEventLogWrite_Fub.AddDataSize = iStrLen + 1;
      pLogger->Fubs.ArEventLogWrite_Fub.Execute = TRUE;
      pLogger->Fubs.ArEventLogWrite_Fub.Ident = pLogger->Ident;
      pLogger->Fubs.ArEventLogWrite_Fub.TimeStamp = 0;
      pLogger->Fubs.ArEventLogWrite_Fub.AddDataFormat = 1; //arEVENTLOG_ADDFORMAT_TEXT;  
      pLogger->Fubs.ArEventLogWrite_Fub.EventID = ArEventLogMakeEventID(PTOP_LOG_SEVERITY_ERROR, 2, 0);
  
      // set message
      ArEventLogWrite(&pLogger->Fubs.ArEventLogWrite_Fub);
      iRetVal = pLogger->Fubs.ArEventLogWrite_Fub.StatusID;
  
      // and reset FUB
      pLogger->Fubs.ArEventLogWrite_Fub.Execute = FALSE;
      ArEventLogWrite(&pLogger->Fubs.ArEventLogWrite_Fub);
    }
  }
  pLogManager->Internal.StartLogging = pLogManager->StartLogging;
  HelpString1[0] = 0;
	
  if (pLogManager->StartLogging == FALSE)
  {    
    return 0;
  }
	
  if (pDetailLogger->Checked == FALSE)
  {
    return 1;
  }
	
  // Main Part
  // Main Part
  if (pLogManager->Settings.MeasurementType != LOG_INFINITE)	// check logging type (Time, entries or infinite)
  {
    if ((pLogManager->Settings.NrOfLogEntries == 0) && (pLogManager->Settings.MeasurementTime == 0))
    {
      return 2;
    }
  }
	
  if (pLogManager->Settings.MeasurementType == LOG_ENTRIES)
  {
    if (pLogManager->Internal.LogEntries >= pLogManager->Settings.NrOfLogEntries)
    {
      pLogManager->Internal.LogEntries = 0;
      pLogManager->StartLogging = 0;
    }
  }
  else if (pLogManager->Settings.MeasurementType == LOG_TIME)
  {
    if (ActTime >= pLogManager->Internal.ActTime + (pLogManager->Settings.MeasurementTime / 10))
    {
      pLogManager->Internal.LogEntries = 0;
      pLogManager->StartLogging = 0;
    }
  }
	

  // just for logging step, if the step stays the same nothing happens: otherwise all wait steps would be entered cyclic
  if (Step == pDetailLogger->LastStepInternal)
  {
    return 3;
  }
  pLogManager->Internal.LogEntries++;
	
  pDetailLogger->LastStepInternal = Step;
	
  brsstrcpy((UDINT) &HelpString1, (UDINT) pFubName);
  brsstrcat((UDINT) &HelpString1, (UDINT) &HelpString3);
  brsstrcat((UDINT) &HelpString1, (UDINT) pStepName); 
  //
	
  pLogger->Fubs.ArEventLogWrite_Fub.AddData = (UDINT) &HelpString1;
  iStrLen = brsstrlen((UDINT) HelpString1);
   
  pLogger->Fubs.ArEventLogWrite_Fub.AddDataSize = iStrLen + 1;
  pLogger->Fubs.ArEventLogWrite_Fub.Execute = TRUE;
  pLogger->Fubs.ArEventLogWrite_Fub.Ident = pLogger->Ident;
  pLogger->Fubs.ArEventLogWrite_Fub.TimeStamp = 0;
  pLogger->Fubs.ArEventLogWrite_Fub.AddDataFormat = 1;//arEVENTLOG_ADDFORMAT_TEXT;  
  pLogger->Fubs.ArEventLogWrite_Fub.EventID = ArEventLogMakeEventID(PTOP_LOG_SEVERITY_INFO, 2, 0);
  
  // set message
  ArEventLogWrite(&pLogger->Fubs.ArEventLogWrite_Fub);
  iRetVal = pLogger->Fubs.ArEventLogWrite_Fub.StatusID;
  
  // and reset FUB
  pLogger->Fubs.ArEventLogWrite_Fub.Execute = FALSE;
  ArEventLogWrite(&pLogger->Fubs.ArEventLogWrite_Fub);
  
  // debug
  pLogger->Diag.CreateEntry++;
	
  return iRetVal;
}

//Registration || STEP || PV || StatusID
UINT WriteDataToBuRLogger(
  Logger_typ* pLogger,	/* Main Part*/
  ServBurLogCtrlStepsInt_typ* pDetailLogger,
  ServBurLog_typ* pLogManager,
  char* pFubName,     /* name of calling function*/
  char* pStepName,    /* name of procedure step*/
  char* pAddName,    /* Additional text */
  DINT AdditionalStatus,	/* Additional info */	
  DINT Step,
  UDINT ActTime	/* Act time */
  )
{
  USINT HelpString1[255];
  USINT HelpString2[20];
  USINT HelpString3[5];
  UINT iRetVal = 0;
  UINT iStrLen;
	
  HelpString1[0] = 0;
  HelpString3[0] = 0;
  brsstrcpy((UDINT) &HelpString3, (UDINT) " || ");
	
  // main and detail must be true for logging
  if (pLogManager->StartLogging != pLogManager->Internal.StartLogging)
  {
    if (pLogManager->StartLogging == TRUE)
    {
      // save starting time
      pLogManager->Internal.ActTime = ActTime;
			
      // Logger is started; just enter it in the logger once
      brsstrcpy((UDINT) &HelpString1, (UDINT) START_LOG_TXT);
      brsstrcat((UDINT) &HelpString1, (UDINT) &pLogManager->Title);
      pLogger->Fubs.ArEventLogWrite_Fub.AddData = (UDINT) &HelpString1;
      iStrLen = brsstrlen((UDINT) &HelpString1);
   
      pLogger->Fubs.ArEventLogWrite_Fub.AddDataSize = iStrLen + 1;
      pLogger->Fubs.ArEventLogWrite_Fub.Execute = TRUE;
      pLogger->Fubs.ArEventLogWrite_Fub.Ident = pLogger->Ident;
      pLogger->Fubs.ArEventLogWrite_Fub.TimeStamp = 0;
      pLogger->Fubs.ArEventLogWrite_Fub.AddDataFormat = 1; //arEVENTLOG_ADDFORMAT_TEXT;  
      pLogger->Fubs.ArEventLogWrite_Fub.EventID = ArEventLogMakeEventID(PTOP_LOG_SEVERITY_ERROR, 2, 0);
  
      // set message
      ArEventLogWrite(&pLogger->Fubs.ArEventLogWrite_Fub);
      iRetVal = pLogger->Fubs.ArEventLogWrite_Fub.StatusID;
  
      // and reset FUB
      pLogger->Fubs.ArEventLogWrite_Fub.Execute = FALSE;
      ArEventLogWrite(&pLogger->Fubs.ArEventLogWrite_Fub);
    }
    else
    {
      pLogManager->Internal.LogEntries = 0;
    }
  }
  pLogManager->Internal.StartLogging = pLogManager->StartLogging;
  HelpString1[0] = 0;
	
  if (pLogManager->StartLogging == FALSE)
  {
    return 0;
  }
	
  if (pDetailLogger->Checked == FALSE)
  {
    return 1;
  }
	
  // Main Part
  if (pLogManager->Settings.MeasurementType != LOG_INFINITE)	// check logging type (Time, entries or infinite)
  {
    if ((pLogManager->Settings.NrOfLogEntries == 0) && (pLogManager->Settings.MeasurementTime == 0))
    {
      return 2;
    }
  }
		
  if (pLogManager->Settings.MeasurementType == LOG_ENTRIES)
  {
    if (pLogManager->Internal.LogEntries >= pLogManager->Settings.NrOfLogEntries)
    {
      pLogManager->Internal.LogEntries = 0;
      pLogManager->StartLogging = 0;
    }
  }
  else if (pLogManager->Settings.MeasurementType == LOG_TIME)
  {
    if (ActTime >= pLogManager->Internal.ActTime + (pLogManager->Settings.MeasurementTime/10))
    {
      pLogManager->Internal.LogEntries = 0;
      pLogManager->StartLogging = 0;
    }
  }
	

  // just for logging step, if the step stays the same nothing happens: otherwise all wait steps would be entered cyclic
  //	if (Step == pDetailLogger->LastStepInternal)
  //	{
  //		return 0; // set no value 
  //	}
  pLogManager->Internal.LogEntries++;
	
  pDetailLogger->LastStepInternal = Step;
	
  brsstrcpy((UDINT) &HelpString1, (UDINT) pFubName);
  brsstrcat((UDINT) &HelpString1, (UDINT) &HelpString3);
  brsstrcat((UDINT) &HelpString1, (UDINT) pStepName); 
  brsstrcat((UDINT) &HelpString1, (UDINT) &HelpString3);
  brsstrcat((UDINT) &HelpString1, (UDINT) pAddName); 
	
  //
  if (AdditionalStatus != 0x7FFFFFFF)
  {
    brsitoa(AdditionalStatus, (UDINT) &HelpString2);    
    brsstrcat((UDINT) &HelpString1,(UDINT)  " (");
    brsstrcat((UDINT) &HelpString1,(UDINT) &HelpString2);
    brsstrcat((UDINT) &HelpString1,(UDINT) ")");	
  }
	
  pLogger->Fubs.ArEventLogWrite_Fub.AddData = (UDINT) &HelpString1;
  iStrLen = brsstrlen((UDINT) &HelpString1);
   
  pLogger->Fubs.ArEventLogWrite_Fub.AddDataSize = iStrLen + 1;
  pLogger->Fubs.ArEventLogWrite_Fub.Execute = TRUE;
  pLogger->Fubs.ArEventLogWrite_Fub.Ident = pLogger->Ident;
  pLogger->Fubs.ArEventLogWrite_Fub.TimeStamp = 0;
  pLogger->Fubs.ArEventLogWrite_Fub.AddDataFormat = 1; //arEVENTLOG_ADDFORMAT_TEXT;  
  pLogger->Fubs.ArEventLogWrite_Fub.EventID = ArEventLogMakeEventID(PTOP_LOG_SEVERITY_WARNING, 2, 0);
  
  // set message
  ArEventLogWrite(&pLogger->Fubs.ArEventLogWrite_Fub);
  iRetVal = pLogger->Fubs.ArEventLogWrite_Fub.StatusID;
  
  // and reset FUB
  pLogger->Fubs.ArEventLogWrite_Fub.Execute = FALSE;
  ArEventLogWrite(&pLogger->Fubs.ArEventLogWrite_Fub);
  
  // debug
  pLogger->Diag.CreateEntry++;
	
  return iRetVal;
}


//Registration || STEP || PV || StatusID
UINT WriteRegDataToBuRLogger(
  Logger_typ* pLogger,	/* Main Part*/
  ServBurLogCtrlStepsInt_typ* pDetailLogger,
  ServBurLog_typ* pLogManager,
  char* pFubName,    /* name of calling function*/
  UINT  PvIndex,     /* name of procedure step*/
  UINT ClientIndex,  /* Additional text */	
  DINT Step,
  UDINT ActTime	/* Act time */
  )
{
  USINT HelpString1[255];
  USINT HelpString2[20];
  USINT HelpString3[5];
  UINT iRetVal = 0;
  UINT iStrLen;
	
  HelpString1[0] = 0;
  HelpString3[0] = 0;
  brsstrcpy((UDINT) &HelpString3, (UDINT) " || ");
	
  // main and detail must be true for logging
  if (pLogManager->StartLogging != pLogManager->Internal.StartLogging)
  {
    if (pLogManager->StartLogging == TRUE)
    {
      // save starting time
      pLogManager->Internal.ActTime = ActTime;
			
      // Logger is started; just enter it in the logger once
      brsstrcpy((UDINT) HelpString1, (UDINT) START_LOG_TXT);
      brsstrcat((UDINT) HelpString1, (UDINT) &pLogManager->Title);
      pLogger->Fubs.ArEventLogWrite_Fub.AddData = (UDINT) &HelpString1;
      iStrLen = brsstrlen((UDINT) &HelpString1);
   
      pLogger->Fubs.ArEventLogWrite_Fub.AddDataSize = iStrLen + 1;
      pLogger->Fubs.ArEventLogWrite_Fub.Execute = TRUE;
      pLogger->Fubs.ArEventLogWrite_Fub.Ident = pLogger->Ident;
      pLogger->Fubs.ArEventLogWrite_Fub.TimeStamp = 0;
      pLogger->Fubs.ArEventLogWrite_Fub.AddDataFormat = 1; //arEVENTLOG_ADDFORMAT_TEXT;  
      pLogger->Fubs.ArEventLogWrite_Fub.EventID = ArEventLogMakeEventID(PTOP_LOG_SEVERITY_ERROR, 2, 0);
  
      // set message
      ArEventLogWrite(&pLogger->Fubs.ArEventLogWrite_Fub);
      iRetVal = pLogger->Fubs.ArEventLogWrite_Fub.StatusID;
  
      // and reset FUB
      pLogger->Fubs.ArEventLogWrite_Fub.Execute = FALSE;
      ArEventLogWrite(&pLogger->Fubs.ArEventLogWrite_Fub);
    }
    else
    {
      pLogManager->Internal.LogEntries = 0;
    }
  }
  pLogManager->Internal.StartLogging = pLogManager->StartLogging;
  HelpString1[0] = 0;
	
  if (pLogManager->StartLogging == FALSE)
  {
    return 0;
  }
	
  if (pDetailLogger->Checked == FALSE)
  {
    return 1;
  }
	
  // Main Part
  if (pLogManager->Settings.MeasurementType != LOG_INFINITE)	// check logging type (Time, entries or infinite)
  {
    if ((pLogManager->Settings.NrOfLogEntries == 0) && (pLogManager->Settings.MeasurementTime == 0))
    {
      return 2;
    }
  }
		
  if (pLogManager->Settings.MeasurementType == LOG_ENTRIES)
  {
    if (pLogManager->Internal.LogEntries >= pLogManager->Settings.NrOfLogEntries)
    {
      pLogManager->Internal.LogEntries = 0;
      pLogManager->StartLogging = 0;
    }
  }
  else if (pLogManager->Settings.MeasurementType == LOG_TIME)
  {
    if (ActTime >= pLogManager->Internal.ActTime + (pLogManager->Settings.MeasurementTime/10))
    {
      pLogManager->Internal.LogEntries = 0;
      pLogManager->StartLogging = 0;
    }
  }
	

  // just for logging step, if the step stays the same nothing happens: otherwise all wait steps would be entered cyclic
  //	if (Step == pDetailLogger->LastStepInternal)
  //	{
  //		return 0; // set no value 
  //	}
  pLogManager->Internal.LogEntries++;
	
  pDetailLogger->LastStepInternal = Step;
	
  brsstrcpy((UDINT) &HelpString1, (UDINT) pFubName);
  brsstrcat((UDINT) &HelpString1, (UDINT) &HelpString3);
  brsstrcat((UDINT) &HelpString1, (UDINT) "PvIndex: ");
  brsitoa(PvIndex, (UDINT) &HelpString2);
  brsstrcat((UDINT) &HelpString1, (UDINT) &HelpString2);
  brsstrcat((UDINT) &HelpString1, (UDINT) &HelpString3);
  brsstrcat((UDINT) &HelpString1, (UDINT) "ClientIndex: ");
  brsitoa(ClientIndex, (UDINT) &HelpString2);
  brsstrcat((UDINT) &HelpString1, (UDINT) &HelpString2);
	
  pLogger->Fubs.ArEventLogWrite_Fub.AddData = (UDINT) &HelpString1;
  iStrLen = brsstrlen((UDINT) &HelpString1);
   
  pLogger->Fubs.ArEventLogWrite_Fub.AddDataSize = iStrLen + 1;
  pLogger->Fubs.ArEventLogWrite_Fub.Execute = TRUE;
  pLogger->Fubs.ArEventLogWrite_Fub.Ident = pLogger->Ident;
  pLogger->Fubs.ArEventLogWrite_Fub.TimeStamp = 0;
  pLogger->Fubs.ArEventLogWrite_Fub.AddDataFormat = 1; //arEVENTLOG_ADDFORMAT_TEXT;  
  pLogger->Fubs.ArEventLogWrite_Fub.EventID = ArEventLogMakeEventID(PTOP_LOG_SEVERITY_WARNING, 2, 0);
  
  // set message
  ArEventLogWrite(&pLogger->Fubs.ArEventLogWrite_Fub);
  iRetVal = pLogger->Fubs.ArEventLogWrite_Fub.StatusID;
  
  // and reset FUB
  pLogger->Fubs.ArEventLogWrite_Fub.Execute = FALSE;
  ArEventLogWrite(&pLogger->Fubs.ArEventLogWrite_Fub);
  
  // debug
  pLogger->Diag.CreateEntry++;
	
  return iRetVal;
}


/* search through the variable list and get data for up to 10 variables with some problems */
BOOL GetServerPvListProblems(
  UDINT* pPvStartAdr,		/* Start of Varible list */
  UDINT* pClientStartAdr, /* Start of Client list */
  PvServListMain_typ* pPvListMain /* Variable Result List*/
  )
{
  PVListEntry_typ* pPv;
  //ClientListeEntry_typ* pClient;
	
  UINT i;
  UINT iPvResult = 0;
	
  //	memcpy((void**)&PvMonitor, (void**)(pPVListeStart + Monitor.PVIndex), sizeof(PVListEntry_typ));
	
  if ((pPvStartAdr == NULL) || (pClientStartAdr == NULL) || (pPvListMain == NULL))
  {
    return FALSE;
  }
	
  if (pPvListMain->Start == FALSE)
  {
    return FALSE;
  }
	
  pPvListMain->Start = FALSE;	// set to false, check one time
	
  // clear last list
  brsmemset((UDINT) &pPvListMain->Result, 0, sizeof(pPvListMain->Result));
	
  // get infos:
  for (i = 0, pPv = (PVListEntry_typ*)pPvStartAdr; i < MAX_CLIENT_VARIABLEN; i++, pPv++)
  {

    if (pPv->Name[0] != 0)
    {
      if ((pPv->pPv == NULL) || (pPv->PvLaenge == 0)) 
      {
        pPvListMain->Result[iPvResult].PvIndex = i;
			
        if (pPv->pPv != NULL) 
        {
          pPvListMain->Result[iPvResult].Info.pPv = PV_STATUS_OK;
        }
        else
        {
          pPvListMain->Result[iPvResult].Info.pPv = PV_STATUS_ERR;
        }	
			
        if (pPv->PvLaenge != 0)
        {
          pPvListMain->Result[iPvResult].Info.PvLaenge = PV_STATUS_OK;
        }
        else
        {
          pPvListMain->Result[iPvResult].Info.PvLaenge = PV_STATUS_ERR;
        }
			
        iPvResult++;
			
        if (iPvResult >= DIAG_PV_RESULT_LIST)
        {
          break;
        }
      }
    }
  }
	
	
  return 0;
}


UDINT RemoveCompilerWarning(UDINT Param)
{
  DINT iFor=0;
  
  iFor+= Param;
  
  return iFor;
}