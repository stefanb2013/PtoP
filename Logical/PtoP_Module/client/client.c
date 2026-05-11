/*****************************************************************************************************************
*            UDP - Client
*---------------------------------------------------------------------------------------------------------------
*            Taskname: client
*            Dateiname: client.c
*            Autor:  B&R         
*            Erstelldatum: Mai 2002
*            Classtime: 10 - 30000 ms
*---------------------------------------------------------------------------------------------------------------
*  Funktion:
*          - Clientfunktionalität für UDP Treiber BMW
*          - Speicherverwaltung PV + Client Liste
*          - Anmeldevorgang über ClientConnect, PVOpenRequest u. ClientLink
*          - Eventauswertung über EventRespones, Event von Server 
*
*--------------------------------------------------------------------------------------------------------------
*  Änderungen:
*          - Änderungen siehe Revisionstext
*****************************************************************************************************************/

#ifndef _REPLACE_CONST
#define _REPLACE_CONST    // ab AS 3.0 werden Konstanten default als globale Variablen angelegt
                          // mit dem define _REPLACE_CONST wird dieses Verhalten abgeschaltet!
#endif

#ifdef _DEFAULT_INCLUDES
#include <AsDefault.h>
#endif

#include "../processor.h"
#include <bur/plc.h>
#include <bur/plctypes.h>
#include <dataobj.h>
#include <asudp.h>
#include <standard.h>
#include <sys_lib.h>
#include <asieccon.h>
#include <astime.h>
#include <asbrstr.h>

#if INTEL
#include <asarcfg.h>
#include <ashost.h>
#include <brsystem.h>
#endif

#include "allgclnt.h" 
#include "PtPCl.h"
#include "ptoppars.h"
#include "errclnt.h"
#include "../diagnostic.h"
#include "funcclient.h"
#include "../techguard.h"
#include "../stephandler.h"

/*****************************************************************************************************************/
/*                      Internal defines                                                                         */
/*****************************************************************************************************************/

#define CLIENT_VERSION_TXT  "client: Version V6.50.0"


/*****************************************************************************************************************/
/*                      Variablendeklaration                         */
/*****************************************************************************************************************/

/* ---- statische Variablen -------------------------------------------------------------------------------------*/
#if INTEL
static USINT        usIpAdrMonitor[4];
#endif


/* ****************************************************************************************************************
* --- Prototyping 
**************************************************************************************************************** */

void ResetLifeCheck(Client_List_Typ* pClientStart);
void ResetIpAdrForServer(Client_List_Typ* pClientStart, UINT uiServerNr);
void CyclicSLSConnectToUst();
void CyclicSLSEventhandleToUst();
void CyclicSLSLifeCheckForUst();
void CyclicClientMonitor();
void CyclicSLSGetIpAdr();
void CyclicTriggerReconnect();
void CyclicGetOwnIpAdr(UDINT *pIpAdr);
void GenerateConnHeaderTelegramm(UDINT *pKennung, UINT iMode);
void GenerateLinkHeaderTelegramm(UDINT *pKennung, UINT iMode);
void GenerateOpenTelegrammX(UINT* puiPvOffset, UINT uiActServ, PvList_Typ* pPvStartAdr, Client_List_Typ* pClientStartAdr, UDINT* pStartAdrBuf, UINT* puiTelLen, PvConnect_Typ* pPvConnect, UINT* puiPvCnt);
BOOL GenerateOpenTelegramm(UINT* puiPvOffset, UINT uiActServ, PvList_Typ* pPvStartAdr, Client_List_Typ* pClientStartAdr, UDINT* pStartAdrBuf, UINT* puiTelLen, PvConnect_Typ* pPvConnect, PVOpenRequest_typ *pPvOpenRequest);
BOOL ServIsValidForOpen (UDINT *pServEntry, BOOL Conn);
void GetMainDataFromRecvBufOpen(UDINT* pRecvBufOpen, UINT*  puiPvCnt, UINT*  puiClientIndex);
BOOL GetDataFromRecvBufOpen(UDINT* pRecvBufOpen, UINT uiGetMaxPv, PVOpenResponseX_typ* pPVOpenResponseX, UINT* puiPvEntry);
UINT AssignPvDataToListOpenX(PVOpenResponseX_typ sPv);
BOOL ValidateDataFromRecvBuf(UDINT *pRecvBufOpen, UINT uiTelLen);
void SetPvStateReg(UDINT pPvAdr, UINT uiState);
void GetMainDataFromRecvBufEvent(UDINT* pRecvBufOpen, UINT*  puiPvCnt, UINT*  puiEventCnt);
BOOL GetDataFromRecvBufEvent(UDINT* pRecvBufOpen, UINT  uiGetMaxPv, PVEventX_typ* pPVEvent, UINT* puiPvEntry);
UINT AssignDataForEvntPv(PVEventX_typ sPvEvent, UINT* puiState);
BOOL CheckIpAdrForClient(PVEventX_typ sPvEvent, UDINT pRecvIpAdr);
void ResetConnPv(PvConnect_Typ* sPvConnect);
void WriteConnPvInLogbook(PvConnect_Typ sPvConnect, Client_List_Typ* pActClient, UINT uiActServ, BOOL bEnable);
void SetPtrToNull();
UINT CreateDataObjMem(MemManager_type* pMemManger, DatObjCreate_typ* pDataObjCreateFub, DatObjInfo_typ* pDatObjInfoFub, DatObjDelete_typ* pDatObjDeleteFub);
void ResetStatPv(ServerConn_typ* pServerConn);
UDINT ValidatePointerClient(UDINT udAdr, UINT  uiLocation, MemManagerDiag_type* pMemDiag, UINT  uiTypeOfMem);
void AddErrInfoEventX(UINT uiPVIndex, UDINT* pStrMEssage);
void AssignMonitorDataInit(Client_List_Typ* pClientStart, ServerConn_typ* pServerConn, PvList_Typ* pVariableStart);
void BuildConnTelegram(ClientConnectRequest_typ* pRequest, UINT iTimeout, UINT iClientIndex, UINT iMode);
BOOL CheckTelegramHeader(UDINT* pKennung, STRING* pTelegramHeader);
DINT WriteDiagDataToBuRLoggerOneTime(BOOL* pEntered, char* pTask, char* pFubName, char* pStepName, char* pFreeText, UDINT iState);
UDINT RemoveCompilerWarning(UDINT Param);
UINT WriteStepDataToBuRLogger(Logger_typ* pLogger, ClntBurLogCtrlStepsInt_typ* pDetailLogger, ClntBurLog_typ* pLogManager, char* pFubName, char* pStepName, DINT Step, UDINT ActTime);
UINT WriteDataToBuRLogger(Logger_typ* pLogger, ClntBurLogCtrlStepsInt_typ* pDetailLogger, ClntBurLog_typ* pLogManager, char* pFubName,  char* pStepName,char* pAddName, DINT AddInfo, DINT Step, UDINT ActTime);
BOOL GetClientPvListProblems(UDINT* pPvStartAdr, UDINT* pClientStartAdr, PvClntListMain_typ* pPvListMain);

/*****************************************************************************************************************
* Init ptop - Client
******************************************************************************************************************/

_INIT void Initialisierung (void)
{
  UINT uiCreateMemState = 0;
  
  // for the compiler; 
  RemoveCompilerWarning(LIFE_BUFFER_LEN);
  RemoveCompilerWarning(LOGGER_START_ENABLE);
  RemoveCompilerWarning(Exist_LoggerEnable[0]);
  
  // - - - Breakpoint im Init
  iInitDebug = 0;
  bInitDebug = FALSE;
//  bInitDebug = TRUE; // Breakpoint? (TRUE = kein Breakpoint)
//  do
//  {
//    iInitDebug++;
//  }while (bInitDebug == FALSE);
  
  // - - - Erzeugen eines ptop Logbuchs oder identifizierung des bestehenden Logbuchs
  do
  {
    sLogger.iStatus = CyclicBurLoggerCreate(&sLogger, LOGGER_NAME_CLNT);
  } while (sLogger.iStatus == ERR_FUB_BUSY);
  
  WriteLoggerMessage((Logger_typ*) &sLogger, 0xF000, (STRING*) "client:Init - Start", 0x7FFFFFFF, PTOP_LOG_SEVERITY_INFO);
  WriteLoggerMessage((Logger_typ*) &sLogger, 0xF000, (STRING*) CLIENT_VERSION_TXT, 0x7FFFFFFF, PTOP_LOG_SEVERITY_INFO);
  
  SwVersion = PTOP_VERSION;
  
  /* --- Set Inits Measurements ----------------------------- */
  CmdBurLogBook.Settings.NrOfLogEntries = 100;  /* nr. of entries */
  CmdBurLogBook.Settings.MeasurementTime = 5000; /* [ms]			*/
  CmdBurLogBook.Settings.MeasurementType = LOG_INFINITE;
  CmdBurLogBook.Steps.Connect.Checked = 1;  /* loggen während boot up*/
  brsstrcpy((UDINT) &CmdBurLogBook.Title, (UDINT) "Default");
  CmdBurLogBook.StartLogging = LOGGER_START_ENABLE;
  
  /* --- Zuweisung der benötigten Portadressen -------------- */
  PortTarget[0] = 20001;  /* Senden Server Anmeldung    */
  PortTarget[1] = 20002;  /* Empfangen Server Anmeldung */
  
  PortSource[0] = 20003;  /* Senden Client Anmeldung    */
  PortSource[1] = 20004;  /* Empfangen Client Anmeldung */
  
  PortTarget[2] = 20005;  /* Senden Client Events     */
  PortTarget[3] = 20006;  /* Empfangen Client  Events   */
  
  PortSource[2] =   20007;  /* Senden Server Event      */
  PortSource[3] =   20008;  /* Empfangen Server Event   */
  
  PortTarget[4] = 20009;  /* Senden Client Lifecheck    */
  PortTarget[5] = 20010;  /* Empfangen Client Lifecheck   */
  
  PortSource[4] =   20011;  /* Senden Server Lifecheck    */
  PortSource[5] =   20012;  /* Empfangen Server Lifecheck */
  
  /* --- UDP Ports öffnen ----------------------------------------------------------------------------------------------- */
  for (uiLoopOpenPorts = 0; uiLoopOpenPorts < PORT_CNT; uiLoopOpenPorts++) 
  { 
    UDPopenFub[uiLoopOpenPorts].enable = TRUE;
    UDPopenFub[uiLoopOpenPorts].port = PortSource[uiLoopOpenPorts];
    UdpOpen(&UDPopenFub[uiLoopOpenPorts]);
  }
  
  /************************************************************************************************************************/
  /*                      Speicherverwaltung                                  */
  /* Allokierung des Speichers mit TmpAlloc -> nach Warmstart wird Speicher freigegeben + Speicher ist zusammenhängender  */
  /* Bereich                                                        */
  
  sMemManager.sPVSpeicherPlatz.udLen  = sizeof(PvList_Typ) * MAX_VARIABLEN;
  sMemManager.sClientSpeicherPlatz.udLen =  sizeof(Client_List_Typ) * MAX_SERVER;
  
  sMemManager.udMemory = sMemManager.sPVSpeicherPlatz.udLen 
    + sMemManager.sClientSpeicherPlatz.udLen 
    + MAX_UDP_FRAME_LEN 
    + MAX_UDP_FRAME_LEN 
    + MAX_UDP_FRAME_LEN 
    + MAX_UDP_FRAME_LEN;

  /* --- Datenobjekt anlegen  --------------------------- */
  uiCreateMemState = CreateDataObjMem(&sMemManager, &DataObjCreateFub, &DatObjInfoFub, &DatObjDeleteFub);

  /* --- Fehler bei Datenmodulerschaffung, keine weitere Bearbeitung mehr möglich ! ------------------------------------ */
  if (uiCreateMemState != 0)
  {
    brsitoa(uiCreateMemState, (UDINT) &HelpString2);
    brsstrcpy((UDINT) &HelpString1, (UDINT) "client:Fehler bei DM - ");
    brsstrcat((UDINT) &HelpString1, (UDINT) &HelpString2);
    
    WriteLoggerMessage((Logger_typ*) &sLogger, 0xF000, (STRING*) &HelpString1, uiCreateMemState, PTOP_LOG_SEVERITY_ERROR);    
    return;
  }
    
  /* --- Zuweisung der Pointer ----------------------------------------------------------------------------------------- */
  
  /* --- sMemManager.udDOPtr ... Startadresse des DM */
  /* --- Adressen + Offset werden in Übersichtsstruktur eingetragen und die eigentlichen Pointer werden initialisiert */
    
  pVariablenVerwaltungStart = (PvList_Typ*) (sMemManager.udDOPtr + udAdrOffset);
  sMemManager.sPVSpeicherPlatz.udAdr = sMemManager.udDOPtr + udAdrOffset;
  udAdrOffset = sMemManager.sPVSpeicherPlatz.udLen;
  
  pClientVerwaltungStart = (Client_List_Typ*) (sMemManager.udDOPtr + udAdrOffset);
  sMemManager.sClientSpeicherPlatz.udAdr = sMemManager.udDOPtr + udAdrOffset;
  udAdrOffset += sMemManager.sClientSpeicherPlatz.udLen;

  /* --- Buffer für neuen Modus allokieren ------------------------------------------------------------- */
  pSendBufOpen = (UDINT*) (sMemManager.udDOPtr + udAdrOffset);
  sMemManager.sSendBufOpen.udAdr = sMemManager.udDOPtr + udAdrOffset;
  sMemManager.sSendBufOpen.udLen = MAX_UDP_FRAME_LEN;
  udAdrOffset += MAX_UDP_FRAME_LEN;
  
  pSendBufEvnt = (UDINT*) (sMemManager.udDOPtr + udAdrOffset);
  sMemManager.sRecvBufOpen.udAdr = sMemManager.udDOPtr + udAdrOffset;
  sMemManager.sRecvBufOpen.udLen = MAX_UDP_FRAME_LEN; 
  udAdrOffset += MAX_UDP_FRAME_LEN;
    
  pRecvBufOpen = (UDINT*) (sMemManager.udDOPtr + udAdrOffset);
  sMemManager.sSendBufEvnt.udAdr = sMemManager.udDOPtr + udAdrOffset;
  sMemManager.sSendBufEvnt.udLen = MAX_UDP_FRAME_LEN;
  udAdrOffset += MAX_UDP_FRAME_LEN;
  
  pRecvBufEvnt = (UDINT*) (sMemManager.udDOPtr + udAdrOffset);
  sMemManager.sRecvBufEvnt.udAdr = sMemManager.udDOPtr + udAdrOffset;
  sMemManager.sRecvBufEvnt.udLen = MAX_UDP_FRAME_LEN; 
  udAdrOffset += MAX_UDP_FRAME_LEN;

  /**********************************************************************************************************************/
  /*              Empfangsfunktion vorbelegen                                 */
  UDPreceiveFub.enable = TRUE;
  UDPreceiveFub.pData = (UDINT) pRecvBufOpen;
  UDPreceiveFub.datamax = MAX_UDP_FRAME_LEN;
  UDPreceiveFub.pIpAddr = (UDINT) &arRecvOpenIpAdr;
  
  /* --- Ermittlung der eigenen IP - Adresse*/
  IPAdresseSource = GetOwnIpAddress();

  if (IPAdresseSource != 0)
  {
    /* --- Umwandlung der Adresse in einen String und ablegen in interner Verwaltung */
    ConvertIpAdrToAsciiX(IPAdresseSource, (UDINT*) &arIpAdrSource);
  }
  else
  {
    /* --- Anforderung zu Laufzeit ---------------------------------- */
    sIpAdrSource.bEnable = TRUE;
  }

  /*****************************************************************************************************************/
  /* Suchen des Datenmoduls und Ermittlung der Adresse                               */
  /* 1. DatObjInfo um Startadresse zu bekommen                                   */
  DatObjInfoFub.enable  =   TRUE;
  DatObjInfoFub.pName =   (UDINT) "ptop";
  DatObjInfo(&DatObjInfoFub);
  
  if (DatObjInfoFub.status == FUB_OK)
  {
    FlagInitOk[0] = TRUE;
  }
  else
  {
    FlagInitOk[0] = FALSE;
  }
  
  pStartParsData = (USINT*)DatObjInfoFub.pDatObjMem;
  ParsLen = DatObjInfoFub.len;
    
  /***************************************************************************/
  /*                Eintrag der Fehler ins Logbuch                           */
  /*                Datenmodul                                               */
  if (FlagInitOk[0] == FALSE)
  {
    WriteLoggerMessage((Logger_typ*) &sLogger, DatObjInfoFub.status, (STRING*) "client:Fehler - Datenmodul 'ptop'", 0x7FFFFFFF, PTOP_LOG_SEVERITY_ERROR);
  }
 	
  /*****************************************************************************************************************/
  /* 2. Parsen des Datenmoduls und Eintrag in Speicher                               */
  bParsMaxReached = ParsenDataModul ((UDINT*)pVariablenVerwaltungStart , (UDINT*)pClientVerwaltungStart, pStartParsData, ParsLen, &AktVar);

  /* --- Umwandlung der ermittelten Ip - Adresse von integer in ASCII, bei konfiguriertem Hostnamen wird erst bei  */
  /* --- Auflösung gewandelt ---- */
  ConvertIpAdrListToAscii((UDINT*) pClientVerwaltungStart);

  /* --- Eintrag ins Logbuch, falls die maximale Anzahl der PVs überschritten wurde */
  if (bParsMaxReached)
  {
    WriteLoggerMessage((Logger_typ*) &sLogger, 0xF000, (STRING*) "client:max. PV's überschritten", 0x7FFFFFFF, PTOP_LOG_SEVERITY_ERROR);    
  }
    
  
  /*****************************************************************************************************************/
  /* Eintrag ins Logbuch, falls Teil der Initialisierung                               */
  /* Start bei Nr.: 57344 lt. AS Doku                                          */
  /*****************************************************************************************************************/
  /*                Eintragen der Statusmeldungen ins Logbuch                      */
  FlagInitOk[1] = TRUE; // setzen für Logbuch-Eintrag
  if ((FlagInitOk[0] == TRUE) && (FlagInitOk[1] == TRUE))
  {   
    brsitoa(sMemManager.udMemory, (UDINT) &HelpString2);
    brsstrcpy((UDINT) &HelpString1, (UDINT) "client:Speicher=");
    brsstrcat((UDINT) &HelpString1, (UDINT) &HelpString2);
    brsstrcat((UDINT) &HelpString1, (UDINT) " bytes");

    /* ----------------------------- benötiger Speicher --------------------------------------- */
    WriteLoggerMessage((Logger_typ*) &sLogger, 0xF000, (STRING*) &HelpString1, 0x7FFFFFFF, arEVENTLOG_SEVERITY_INFO);      
  }


  /*****************************************************************************************************************/
  /*     PV_xgetadr   */
  GetPointerOfPV((UDINT*) pVariablenVerwaltungStart, MAX_VARIABLEN);

  /* --- Eintrag der vorhandenen Clients und Server in das Logbuch */
  ParserLogbuch ((UDINT*) pVariablenVerwaltungStart);
    
  /* --- Initialisierung zum Anmelden der Variablen an Server --------------------------------------------------- */
  uiNextRegServer   = 0;
  uiActRegServer  = 0;
  TimeoutCnt    = 0;
  uiActRegPv    = 0;
  uiLoopStartPv   = 0;
  NeuAnmeldung  = FALSE;
  OpenErrorCnt    = 0;
  PVOpenCnt   = 0;
  LogbuchCnt    = 0;    
  
  /*****************************************************************************************************************/
  /*    Remanenz der Variablen bearbeiten                                    */
  /* bei Remanenz = TRUE --> Werte bleiben nach Warmstart unverändert                        */
  /* bei Remanenz = FALSE --> Variablen werden nach Warmstart mit Initialisierungswert (Init) überschrieben    */
  for (uiLoopRemPv = 0; uiLoopRemPv < MAX_VARIABLEN; uiLoopRemPv++)
  {
    pPVInit = pVariablenVerwaltungStart + uiLoopRemPv;
    
    /* --- Adresse ermitteln und Remanent == False:*/
    if ((pPVInit->pPv > 0) && (pPVInit->Remanent == FALSE))
    { 
      switch (pPVInit->PvLaenge)
      {
        case 1:
          {
            pPVWert08 = (USINT*)pPVInit->pPv;
            *pPVWert08 = pPVInit->Init;         
            break;
          }
        
        case 2:
          {
            pPVWert16 = (UINT*)pPVInit->pPv;
            *pPVWert16 = pPVInit->Init;
            break;
          }
        
        case 4:
          {
            pPVWert32 = (UDINT*)pPVInit->pPv;
            *pPVWert32 = pPVInit->Init;
            break;
          }
      }
      pPVInit->LetzterWert = pPVInit->Init;
      pPVInit->LetzterWertCnt = 0;
      pPVInit->LetzterWertServ = 0;
      brsstrcpy((UDINT) &pPVInit->LetzterWertServIp, (UDINT) "");
    }
  }
  
  /*****************************************************************************************************************/ 
  NeuAnmeldungLaeuft = FALSE;
  
  /* Schrittketten initialisieren*/
  StepConnect.Step.StepNr   = UDP_SCHRITT_INIT;
  StepGetIpAdr.Step.StepNr = GET_HOST_INIT;
  

  /* Schrittvariable wird auf nicht benutzten Schritt gesetzt -> Schritt 1 darf erst angewählt werden, wenn UdpOpen in Ordnung ist !*/
  StepLifeCheck.Step.StepNr = LIFE_CHECK_INACTIVE;
  StepEvent.Step.StepNr     = EVENT_SCHRITT_INACTIVE; /* Dummyschritt */
  
  Monitor.Lock    =   TRUE;
  Monitor.ClientIndex =   0;
  Monitor.PVIndex   =   0;
  
  /* --- Zeigerreferenzierung */
  pVariablenManagement  = pVariablenVerwaltungStart + 0;
  pClientManagement = pClientVerwaltungStart + 0; 
  pVariablenEvent   = pVariablenVerwaltungStart + 0;
  pClientEvent      = pClientVerwaltungStart + 0;   
  pClientLife       = pClientVerwaltungStart + 0;
  pClientGetIpAdr   = pClientVerwaltungStart + 0; 
  
  /* --- Lebensüberwachung vorbelegen */
  brsmemset((UDINT) &Lebensueberwachung[0], 1, sizeof(Lebensueberwachung[0]) * MAX_SERVER);
  
  
  /* --- Neuanmeldung an Server vorbelegen */
  brsmemset((UDINT) &NeuAnmeldungHand[0], FALSE, sizeof(NeuAnmeldungHand[0]) * MAX_SERVER);
  brsmemset((UDINT) &NeuAnmeldungFehler[0], FALSE, sizeof(NeuAnmeldungHand[0]) * MAX_SERVER);
  brsmemset((UDINT) &bRequestConn, FALSE, sizeof(NeuAnmeldungHand[0]) * MAX_SERVER);
  
  uiSaveServerReconn = 0;
  
  /*  udpSendFlags = udpMSG_DONTROUTE;*/
  udpSendFlags = 0;
  
  // Größe der Udp Buffer setzen
  udIoCtlParamSendBuf = UDP_BUF_SEND_SIZE;
  udIoCtlParamRecvBuf = UDP_BUF_RECV_SIZE;

  brsmemset((UDINT) &HelpString1, 0, sizeof(HelpString1));
  brsmemset((UDINT) &HelpString2, 0, sizeof(HelpString2));
  
  /* --- Löschen des Speichers f. Anmeldung an Server ------------------------------------------------------- */
  brsmemset((UDINT) &sServerConn, 0, sizeof(sServerConn));

  udMinTimeTick  = 10000;
  
  DIAG_Max_Variables = MAX_VARIABLEN;
  DIAG_Max_Clients = MAX_SERVER;
  
  /* Speicherbereiche hinterlegen */
  sMemDiag.udStartDM = (UDINT) sMemManager.udDOPtr;
  sMemDiag.udStopDM = (UDINT) sMemManager.udDOPtr + sMemManager.udMemory;

  AssignMonitorDataInit(pClientVerwaltungStart, (ServerConn_typ* ) &sServerConn, (PvList_Typ*) pVariablenVerwaltungStart);
  
 
  FlagInitOk[1] = TRUE;
  FlagInitOk[2] = TRUE;    /* Laufzeitflag für Datenobjekt existiert: wenn es nicht mehr existiert, dann client stillegen */

  DTGetTime_Fub.enable = TRUE;
  uiGetIpServer = 0;
  
  WriteLoggerMessage((Logger_typ*) &sLogger, 0xF000, (STRING*) "client:Init - Ende", 0x7FFFFFFF, arEVENTLOG_SEVERITY_INFO);
  
  PVOpenRequest.Variablenname[0] = 0; // for the compiler
  
}   /* void Initialisierung (void) */


/*****************************************************************************************************************
* zyklische Funktion - Client
*****************************************************************************************************************/

_CYCLIC void CyclicFunction(void)
{ 
  if (bTest)
  {
    
    //    AssignMonitorDataInit(pClientVerwaltungStart, (ServerConn_typ* ) &sServerConn, (PvList_Typ*) pVariablenVerwaltungStart);
    bParsMaxReached = ParsenDataModul ((UDINT*)pVariablenVerwaltungStart , (UDINT*)pClientVerwaltungStart, pStartParsData, ParsLen, &AktVar);
    /*
    AddErrInfoEventX(12300, (UDINT*) &arRecvEvntIpAdr);
    pClientVerwaltungStart = 0; */
    //    CmdBurLogBook.FubState = WriteDataToBuRLogger(TRUE, &CmdBurLogBook, "ZyklischeFunktion", "START", "Freier Text", 27);
    //    CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
    
    //    if (CyclicBurLoggerCreate(&sLogger) == ERR_OK)
    //    {
    //      bTest = FALSE;
    //    }
    bTest = FALSE;
    WriteLoggerMessage((Logger_typ*) &sLogger, 0xF000, "client:Speicher = 9191919 bytes", 0x7FFFFFFF, arEVENTLOG_SEVERITY_INFO);
  }
  if (bTest1)
  {
//    sLogger.iStatus = CyclicBurLoggerCreate(&sLogger);
    if (sLogger.iStatus != ERR_FUB_BUSY)
    {
      bTest1 = FALSE;
    }
  }
  
  
  /* --- Abbruchbedingungen ------------------------------------------------ */
  if ((FlagInitOk[0] == FALSE) || (FlagInitOk[1] == FALSE) || (FlagInitOk[2] == FALSE))
  {
    return;
  }

  /* - - -  Überprüfung, ob Datenobjekt, welches als Speicher verwendet wird, noch vorhanden ist - - - - - */
  DatObjInfoFub.enable = TRUE;
  DatObjInfoFub.pName  = (UDINT) DATA_OBJ_MEM_NAME_CLIENT;
  DatObjInfo(&DatObjInfoFub);
  if (DatObjInfoFub.status != 0)
  {
    FlagInitOk[2] = FALSE;
    /* alle Ptr auf NULL setzen */
    SetPtrToNull();

    WriteLoggerMessage((Logger_typ*) &sLogger, 0xF000, (STRING*) "client: Speicherverwaltungs - Datenobjekt nicht vorhanden.", DatObjInfoFub.status, PTOP_LOG_SEVERITY_ERROR);
    return;
  }

  if (Diagnose.LogPointerInfo == TRUE)
  {
    ValidatePointerClient((UDINT) pVariablenVerwaltungStart, 0, (MemManagerDiag_type* ) &sMemDiag, 1);
    ValidatePointerClient((UDINT) pClientVerwaltungStart, 1, (MemManagerDiag_type* ) &sMemDiag, 1);
  }
  
  /* --- Systemtick ermitteln ------------------------------ */
  GetClock(&udStartTimeTick);
  
  /* --- Systemzeit ermitteln ------------------------------ */
  DTGetTime(&DTGetTime_Fub);
  if (DTGetTime_Fub.status == FUB_OK)
  {
    dtActTime = DTGetTime_Fub.DT1;
  }
    
  /* --- Timer aktualisieren  ------------------------------------------------------------------------------ */
  StatusTimer = TimerFunction(&udTimeTick);
  
  /* === händische Neuanmeldung des kompletten Systems ===================================================== */
  if (NeuAnmeldung == TRUE)
  {
    NeuAnmeldung = FALSE;

    /* Neue Initialisierung zum Anmelden der Variablen an Server*/
    uiNextRegServer = 0;
    uiActRegServer  = 0;
    TimeoutCnt    = 0;
    uiActRegPv    = 0;
    uiLoopStartPv = 0;    
    StepConnect.Step.StepNr    = UDP_SCHRITT_NEUANMELDUNG_SET; /* --- Udp Open bereits erledigt*/
    ResetLifeCheck(pClientVerwaltungStart);
  } 
   
  /* --- Client Monitor ------------------------------------------------------------------------------------ */
  CyclicClientMonitor();
  
  /* --- Ermittlung der eigenen Ip-Adresse (falls statt fixer Adresse über Hostname adressiert wird -------- */
  CyclicGetOwnIpAdr(&IPAdresseSource);
  
  /* --- Schrittschaltwerk für Auflösung von Hostnamen------------------------------------------------------ */ 
  CyclicSLSGetIpAdr();
  
  /* --- Überprüfung von Neuanmeldungen auf Servern durch den Benutzer ------------------------------------- */
  CyclicTriggerReconnect();

  /* --- Schrittschaltwerk für Anmeldung ------------------------------------------------------------------- */
  CyclicSLSConnectToUst();

  /* --- Schrittschaltwerk für Eventauswertung ------------------------------------------------------------- */
  CyclicSLSEventhandleToUst();

  /* --- Schrittschaltwerk für Lifecheck ------------------------------------------------------------------- */ 
  CyclicSLSLifeCheckForUst();
    
 
  /* ---  Zeit ermitteln ----------------------------------------------------------------------------------- */
  GetClock(&udEndTimeTick);
  udActTimeTick = udEndTimeTick - udStartTimeTick;
  if (udActTimeTick > udMaxTimeTick)
  {
    udMaxTimeTick = udActTimeTick;
  }
  
  if (udActTimeTick < udMinTimeTick)
  {
    udMinTimeTick = udActTimeTick;
  }


  // check the pv list and get the first 10 problems
  GetClientPvListProblems((UDINT*) pVariablenVerwaltungStart, (UDINT*) pClientVerwaltungStart, (PvClntListMain_typ*) &Diagnose.PvList);
  
  
} /*   _CYCLIC void ZyklischeFunktion(void) */


/*********************************************************************************************
* --- Schrittschaltwerk für Anmeldung 
******************************************************************************************** */

void CyclicSLSConnectToUst()
{

  // Stephandler: overhead /initialisierung
  if (StepConnect.StepHandling.Current.bTimeoutElapsed == TRUE)
  {
    StepConnect.StepHandling.Current.bTimeoutElapsed = 0;
    StepConnect.Step.StepNr = StepConnect.StepHandling.Current.nTimeoutContinueStep;
  }
  StepConnect.StepHandling.Current.nStepNr = (DINT) StepConnect.Step.StepNr;
  brsstrcpy((UDINT) &StepConnect.StepHandling.Current.sStepText, (UDINT) &StepConnect.Step.StepText);
  BrbStepHandler(&StepConnect.StepHandling);
  
  switch (StepConnect.Step.StepNr)
  {
    
    case UDP_SCHRITT_INIT:
      brsstrcpy((UDINT) &StepConnect.Step.StepText, (UDINT) "UDP_SCHRITT_INIT");	// stephandling: copy actual step
      WriteStepDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "UDP_SCHRITT_INIT", StepConnect.Step.StepNr, udTimeTick);
			
      StepConnect.Step.InitDone = TRUE;
      StepConnect.Step.StepNr = UDP_SCHRITT_OPEN;
      break;
    
    /* ---  Sicherstellung, daß Open korrekt ist  ------------------------------- */
    case UDP_SCHRITT_OPEN:
      brsstrcpy((UDINT) &StepConnect.Step.StepText, (UDINT) "UDP_SCHRITT_OPEN");	// stephandling: copy actual step
      WriteStepDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "UDP_SCHRITT_OPEN", StepConnect.Step.StepNr, udTimeTick);
			
      /* --- öffnen der UDP Schnittstellen ------------------------------------ */
      for (uiLoopOpenPorts = 0; uiLoopOpenPorts < PORT_CNT; uiLoopOpenPorts++)
      {
        if (UDPopenFub[uiLoopOpenPorts].status != FUB_OK)
        {
          UdpOpen(&UDPopenFub[uiLoopOpenPorts]);
          OpenUDPCnt++;
        }
      }
      
      /* --- wenn alle Schnittstellen offen sind, weiter ... */
      if (OpenUDPCnt == 0)
      {
        StepConnect.Step.StepNr  = UDP_SCHRITT_OPTIONS_1_SET;
      }

      OpenUDPCnt = 0;
      
      break; /* UDP_SCHRITT_OPEN */
        
    case UDP_SCHRITT_OPTIONS_1_SET:
      brsstrcpy((UDINT) &StepConnect.Step.StepText, (UDINT) "UDP_SCHRITT_OPTIONS_1_SET");	// stephandling: copy actual step
      WriteStepDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "UDP_SCHRITT_OPTIONS_1_SET", StepConnect.Step.StepNr, udTimeTick);
			
      for (uiSetOptions = 0; uiSetOptions < PORT_CNT; uiSetOptions++)
      {
        UdpIoctlFub[uiSetOptions].enable  = TRUE;
        UdpIoctlFub[uiSetOptions].ident   = UDPopenFub[uiSetOptions].ident;
        UdpIoctlFub[uiSetOptions].ioctl   = udpSO_SNDBUF_SET;
        UdpIoctlFub[uiSetOptions].pData   = (UDINT) &udIoCtlParamSendBuf;
        UdpIoctlFub[uiSetOptions].datalen   = sizeof(udIoCtlParamSendBuf);
        UdpIoctl(&UdpIoctlFub[uiSetOptions]);
      }
      StepConnect.Step.StepNr = UDP_SCHRITT_OPTIONS_1;
    /* -> kein Break notwendig !*/

    case UDP_SCHRITT_OPTIONS_1:
      {
        brsstrcpy((UDINT) &StepConnect.Step.StepText, (UDINT) "UDP_SCHRITT_OPTIONS_1");	// stephandling: copy actual step
        WriteStepDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "UDP_SCHRITT_OPTIONS_1", StepConnect.Step.StepNr, udTimeTick);
			
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
          brsstrcat((UDINT) &HelpString1, (UDINT) "client: Udp-Send Buf (");
          brsitoa(PORT_CNT * udIoCtlParamSendBuf , (UDINT) &HelpString2);
          brsstrcat((UDINT) &HelpString1, (UDINT) &HelpString2);
          brsstrcat((UDINT) &HelpString1, (UDINT) "Byte)");
        
          WriteLoggerMessage((Logger_typ*) &sLogger, 0xF000, (STRING*) &HelpString1, 0x7FFFFFFF, arEVENTLOG_SEVERITY_INFO);
          
          StepConnect.Step.StepNr = UDP_SCHRITT_OPTIONS_2_SET;
        }
        uiSetOptionsCnt = 0;
      }
      break;      
    
    case UDP_SCHRITT_OPTIONS_2_SET:
      brsstrcpy((UDINT) &StepConnect.Step.StepText, (UDINT) "UDP_SCHRITT_OPTIONS_2_SET");	// stephandling: copy actual step
      WriteStepDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "UDP_SCHRITT_OPTIONS_2_SET", StepConnect.Step.StepNr, udTimeTick);
			
      for (uiSetOptions = 0; uiSetOptions < PORT_CNT; uiSetOptions++)
      {
        UdpIoctlFub[uiSetOptions].enable  = TRUE;
        UdpIoctlFub[uiSetOptions].ident   = UDPopenFub[uiSetOptions].ident;
        UdpIoctlFub[uiSetOptions].ioctl   = udpSO_RCVBUF_SET;
        UdpIoctlFub[uiSetOptions].pData   = (UDINT) &udIoCtlParamRecvBuf;
        UdpIoctlFub[uiSetOptions].datalen   = sizeof(udIoCtlParamRecvBuf);
        UdpIoctl(&UdpIoctlFub[uiSetOptions]);
      }
      StepConnect.Step.StepNr = UDP_SCHRITT_OPTIONS_2;
    /* -> kein Break notwendig !*/

    case UDP_SCHRITT_OPTIONS_2:
      {
        brsstrcpy((UDINT) &StepConnect.Step.StepText, (UDINT) "UDP_SCHRITT_OPTIONS_2");	// stephandling: copy actual step
        WriteStepDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "UDP_SCHRITT_OPTIONS_2", StepConnect.Step.StepNr, udTimeTick);
			
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
          WriteLoggerMessage((Logger_typ*) &sLogger, 0xF000, (STRING*) "Udp-Recv Buffer gesetzt", 0x7FFFFFFF, arEVENTLOG_SEVERITY_INFO);

          StepConnect.Step.StepNr    = UDP_SCHRITT_NEUANMELDUNG_SET;
          StepLifeCheck.Step.StepNr    = LIFE_CHECK_INIT;
          StepEvent.Step.StepNr      = EVENT_SCHRITT_INIT;
        
          /* --- Vorbelegung für Lifecheck-FUB */
          UDPreceiveLifeFub.enable = TRUE;
          UDPreceiveLifeFub.ident = UDPopenFub[5].ident;
          UDPreceiveLifeFub.pData = (UDINT)&arRecvBufLife;
          UDPreceiveLifeFub.datamax = sizeof(arRecvBufLife);
          UDPreceiveLifeFub.pIpAddr = (UDINT) &arRecvLifeIpAdr;     
        }
        uiSetOptionsCnt = 0;
      }
      break;
          
    
    /* ########################  Neuanmeldung an Server initialisieren ########################################## */
    case UDP_SCHRITT_NEUANMELDUNG_SET:
      brsstrcpy((UDINT) &StepConnect.Step.StepText, (UDINT) "UDP_SCHRITT_NEUANMELDUNG_SET");	// stephandling: copy actual step
      WriteStepDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "UDP_SCHRITT_NEUANMELDUNG_SET", StepConnect.Step.StepNr, udTimeTick);
			
      uiNextRegServer = 0;
      uiActRegServer  = 0;
      TimeoutCnt      = 0;
      StepConnect.Step.StepNr = UDP_SCHRITT_NEUANMELDUNG;
      
    /* KEIN break notwendig !*/
      
    /* ########################  Neuanmeldung an Server DO ############################################## */
    case UDP_SCHRITT_NEUANMELDUNG:
      {
        brsstrcpy((UDINT) &StepConnect.Step.StepText, (UDINT) "UDP_SCHRITT_NEUANMELDUNG");	// stephandling: copy actual step
        WriteStepDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "UDP_SCHRITT_NEUANMELDUNG", StepConnect.Step.StepNr, udTimeTick);
			
        NeuAnmeldungLaeuft = TRUE;
        uiActRegPv      = 0;
        uiLoopStartPv   = 0;
            
        for (uiLoopRegServ = uiNextRegServer; uiLoopRegServ < MAX_SERVER; uiLoopRegServ++)
        {
          pClientManagement = pClientVerwaltungStart + uiLoopRegServ;
        
          if (Diagnose.LogPointerInfo == TRUE)
          {
            ValidatePointerClient((UDINT) pClientManagement, 10, (MemManagerDiag_type* ) &sMemDiag, 1);
          }
        
          /* --- Ermittlung, ob auf Server angemeldet werden kann*/
          /* --- alle Anforderungen zur Neuanmeldung werden jetzt in "CyclicTriggerReconnect" bearbeitet */
          bServIsValid = ServIsValidForOpen((UDINT*) pClientManagement, bRequestConn[uiLoopRegServ]);
        
          if (bServIsValid == TRUE)
          {
            bRequestConn[uiLoopRegServ] = FALSE;
          
            /* --- Server zur Neuanmeldung gefunden ------------------- */
            uiActRegServer = uiLoopRegServ;
            bRegServFound = TRUE; 
           
            CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "UDP_SCHRITT_NEUANMELDUNG", "bServIsValid", uiActRegServer, 999, udTimeTick);
            CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
            
            break;
          }
        }
      
        uiNextRegServer = uiLoopRegServ + 1;
        if (uiNextRegServer >= MAX_SERVER)
        {
          uiNextRegServer = 0;
        }
      
        if (bRegServFound == TRUE)
        {
          Diagnose.EventCnt.Open++;
        
          /* --- Löschen der Status/Verwaltungs-PV für Anmeldung an einen Server ---------- */
          ResetStatPv(&sServerConn[uiActRegServer]);
        
          sServerConn[uiActRegServer].bUsed = TRUE;
        
          /* --- Alarm bei Neuanmeldung zurücksetzen */
          sServerConn[uiActRegServer].Alarm.AlarmList.PvNotConnected = FALSE;
		 
          /* --- Löschen der Statistik - PV ----------------------------------------------- */
          ResetConnPv(&sPvConnect);        

          /* --- Zusammensetzen des Telegrammes ------------------------------------------- */
          pClientManagement = pClientVerwaltungStart + uiActRegServer;
        
          if (Diagnose.LogPointerInfo == TRUE)
          {
            ValidatePointerClient((UDINT) pClientManagement, 11, (MemManagerDiag_type* ) &sMemDiag, 1);
          }
        
          /* --- Verbindung auf Fehler ----------------------------------------------------- */
          pClientManagement->LifeCheck = STATE_LIFE_ERR;

          /* --- Bau des Verbindungstelegrams (Telegrammkennung,...) */
          BuildConnTelegram(&ClientConnectRequest, pClientManagement->Timeout, uiActRegServer, pClientManagement->Modus);        
      
          /* Backup - Kopie erzeugen */
          brsmemcpy((UDINT) &ClientConnectRequestSave, (UDINT) &ClientConnectRequest, sizeof(ClientConnectRequest));
        
          CopyClientConnectRequestToSendBuffer((UDINT*) pSendBufOpen, &ClientConnectRequest);
        
          CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", (char*) "UDP_SCHRITT_NEUANMELDUNG", (char*) pClientManagement->IpAdr, uiActRegServer, 999, udTimeTick);
          CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
          
          UDPsendFub.enable = TRUE;
          UDPsendFub.ident  = UDPopenFub[0].ident;
          UDPsendFub.pHost  = (UDINT) &pClientManagement->IpAdr;
          UDPsendFub.pData  = (UDINT) pSendBufOpen;
          UDPsendFub.datalen  = sizeof(ClientConnectRequest);
          UDPsendFub.port   = PortTarget[0];
          UDPsendFub.flags  = udpSendFlags;
          UdpSend(&UDPsendFub);
          StepConnect.Step.StepNr = UDP_SCHRITT_SEND_CONN;
      
        } /* (bRegServFound == TRUE) */    
      }
      break;
    
    
    /* ---  Abfrage Status Conn - Telegramm  ---------------------------------------------------------- */
    case UDP_SCHRITT_SEND_CONN:
      {
        brsstrcpy((UDINT) &StepConnect.Step.StepText, (UDINT) "UDP_SCHRITT_SEND_CONN");	// stephandling: copy actual step
        WriteStepDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "UDP_SCHRITT_SEND_CONN", StepConnect.Step.StepNr, udTimeTick);
			
        if (UDPsendFub.status != FUB_BUSY)
        {
          CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "UDP_SCHRITT_SEND_CONN", "UDPsendFub.status", UDPsendFub.status, 999, udTimeTick);
          CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
          
          if (UDPsendFub.status == FUB_OK)
          {            
            /* --- mitloggen für Diagnosezwecke ------------------------------------------------------- */
            CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "Send Conn to: ", (char*)&pClientManagement->IpAdr, PortTarget[0], 999, udTimeTick);
            CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
            
            Diagnose.EventCnt.SendConn++;
            StepConnect.Step.StepNr = UDP_SCHRITT_RECEIVE_CONN_SET;
          }
          else
          {
            NeuAnmeldungFehler[uiActRegServer] = TRUE;
            if (UDPsendFub.status != udpERR_NO_DATA) 
            {
              if (pClientManagement != NULL)
              {
                CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "Failed Conn", (char*)&pClientManagement->IpAdr, UDPsendFub.status, StepConnect.Step.StepNr, udTimeTick);
                CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
              }
              else
              {
                CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook,"Registration", "Failed Conn", "udpERR_NO_DATA", UDPsendFub.status, StepConnect.Step.StepNr, udTimeTick);
                CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;      
              }
            }
            bRegServFound = FALSE;
            StepConnect.Step.StepNr = UDP_SCHRITT_NEUANMELDUNG;
          }
        }
        else 
        {
          UdpSend(&UDPsendFub);
          Diagnose.EventCnt.RetryConn++;
        }    
      }
      break;
    
    /* ---  Connection - Empfang der Anmeldebestätigung initialisieren ----------------------------- */
    case UDP_SCHRITT_RECEIVE_CONN_SET:
      {
        brsstrcpy((UDINT) &StepConnect.Step.StepText, (UDINT) "UDP_SCHRITT_RECEIVE_CONN_SET");	// stephandling: copy actual step
        WriteStepDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "UDP_SCHRITT_RECEIVE_CONN_SET", StepConnect.Step.StepNr, udTimeTick);
			
        UDPreceiveFub.ident = UDPopenFub[1].ident;      
        StepConnect.Step.StepNr = UDP_SCHRITT_RECEIVE_CONN;
      
        /* Zeitverzögerung starten*/
        TONFub.IN = TRUE;
        TONFub.PT = pClientManagement->Timeout;
        TON(&TONFub);
        /* hier kein break! -> Auswertung des Aufrufes kann sofort erfolgen */      
      }
    
    /* ---  Abfrage Status Conn - Telegramm  -------------------------------------------------------- */
    case UDP_SCHRITT_RECEIVE_CONN:
      {
        brsstrcpy((UDINT) &StepConnect.Step.StepText, (UDINT) "UDP_SCHRITT_RECEIVE_CONN");	// stephandling: copy actual step
        WriteStepDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "UDP_SCHRITT_RECEIVE_CONN", StepConnect.Step.StepNr, udTimeTick);
			
        UdpRecv(&UDPreceiveFub);
        if (UDPreceiveFub.status == FUB_OK)
        {
          CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "UDP_SCHRITT_RECEIVE_CONN", "UDPreceiveFub.status", UDPreceiveFub.status, 999, udTimeTick);
          CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
          
          CopyReceiveBufferToClientConnectResponse((UDINT*) pRecvBufOpen);

          /* 2. Fälle, alt&neu werden hier durch den Standard Code abgedeckt */
          if ((CheckTelegramHeader((UDINT*) &ClientConnectResponse.Kennung, (STRING*) TELEGRAMM_IDENT_CONN) == TRUE) ||
            (CheckTelegramHeader((UDINT*) &ClientConnectResponse.Kennung, (STRING*) TELEGRAMM_IDENT_CONX) == TRUE))
          {
            Diagnose.EventCnt.ConnResp++;
          
            CRCCheckTemp = CRCcheck((UDINT*) pRecvBufOpen, 8);
            if (CRCCheckTemp == ClientConnectResponse.BCC)
            {
              if (ClientConnectResponse.Status == SERVER_RDY)
              {
                StepConnect.Step.StepNr = UDP_SCHRITT_SEND_OPEN_SET;
                pClientManagement->ClientIndex = ClientConnectResponse.ClientIndex;
                Lebensueberwachung[uiActRegServer] = FALSE;
                lifecnt++;
                bVarReadInit = TRUE;
               					 
                /* --- Rücksetzen des Status - Anmeldung bei allen dem Server zugehörenden PVs ---- */
                for (uiLoopResetServ = 0; uiLoopResetServ < MAX_VARIABLEN; uiLoopResetServ++)
                {
                  pVariablenAnmeldung = pVariablenVerwaltungStart + uiLoopResetServ;
                  if (Diagnose.LogPointerInfo == TRUE)
                  {
                    ValidatePointerClient((UDINT) pVariablenAnmeldung, 12, (MemManagerDiag_type* ) &sMemDiag, 1);
                  }
                
                  if (pVariablenAnmeldung->Client == uiActRegServer)
                  {
                    pVariablenAnmeldung->Anmeldung = 0;
                  }
                }
                CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook,"Registration", "UDP_SCHRITT_RECEIVE_CONN", "conn ready", 0, 999, udTimeTick);
                CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
              }
              else if (ClientConnectResponse.Status == SERVER_CONN_FAILURE)
              { 
                /* --- akt. Server ist nicht bereit zur Anmeldung */
                StepConnect.Step.StepNr = UDP_SCHRITT_NEUANMELDUNG;
                bRegServFound   = FALSE;
                TimeoutCnt    = 0;
                
                CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook,"Registration", "UDP_SCHRITT_RECEIVE_CONN", "not ready", ClientConnectResponse.Status, 999, udTimeTick);
                CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
              }
            }
            else
            {
              CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook,"Registration", "UDP_SCHRITT_RECEIVE_CONN", "crc fail", CRCCheckTemp, 999, udTimeTick);
              CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
              
              ConnReceiveFailed = TRUE;
            }
          }
          else
          {
            CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook,"Registration", "UDP_SCHRITT_RECEIVE_CONN", "wrong telegram header", 0, 999, udTimeTick);
            CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
            
            ConnReceiveFailed = TRUE;
          }
        }
        else
        {
          if ((UDPreceiveFub.status != ERR_FUB_BUSY) && (UDPreceiveFub.status != udpERR_NO_DATA))
          {
            CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook,"Registration", "UDP_SCHRITT_RECEIVE_CONN", "UDPreceiveFub.status Error", UDPreceiveFub.status, 999, udTimeTick);
            CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;            
          }
          ConnReceiveFailed = TRUE;
        }
      
        /* --- Telegramm nicht empfangen --------------------------------------------------------- */
        if (ConnReceiveFailed == TRUE)
        {
          ConnReceiveFailed = FALSE;
          TON(&TONFub);
        
          if (TONFub.Q == TRUE)
          {
            /* --- Zeit abgelaufen ? ---------------------------------------------------------- */
            TONFub.IN = FALSE;
            TON(&TONFub);             
            TimeoutCnt++;
            if (TimeoutCnt < TIMEOUT_RETRY_CONN)
            {
              UdpSend(&UDPsendFub);
              StepConnect.Step.StepNr = UDP_SCHRITT_SEND_CONN; 
            }
            else /* weiter mit nächstem Server */
            {              
              CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook,"Registration", "UDP_SCHRITT_RECEIVE_CONN", "Timeout", 0, StepConnect.Step.StepNr, udTimeTick);
              CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
              
              bRegServFound = FALSE;
              TimeoutCnt = 0;
              StepConnect.Step.StepNr = UDP_SCHRITT_NEUANMELDUNG;
                      
              pClientManagement->LifeCheck = STATE_LIFE_ERR;               /* --- Client versucht später mit Server Kontakt aufzunehmen */
              ResetIpAdrForServer(pClientVerwaltungStart, uiActRegServer); /* Ip Adresse bei konf. Hostnamen verwerfen*/
              Lebensueberwachung[uiActRegServer] = TRUE;                   /* --- kein Lebensüberwachungszeichen   */
              lifecnt++;
            
              /* --- Logbucheintrag ------------------------------------------------------------ */
              if ((UDPreceiveFub.status != udpERR_NO_DATA) && (UDPreceiveFub.status != ERR_FUB_BUSY))
              {
                if (LogbuchCnt < LOGBOOK_ENTRIES_CONN)
                {                  
                  LogbuchEintrag((UDINT*) pClientVerwaltungStart, uiActRegServer, LogbuchCnt, 1);
                  LogbuchCnt++; 
                }
              }
            } /* if (TimeoutCnt < TIMEOUT_RETRY) */
          }
        } /* (UDPreceiveFub.status == FUB_OK) */  
      }
      break; /* UDP_SCHRITT_NEUANMELDUNG */
    
        
    /* --- Senden der Variablen zum Server ----------------------------------------------- */
    case UDP_SCHRITT_SEND_OPEN_SET:
      brsstrcpy((UDINT) &StepConnect.Step.StepText, (UDINT) "UDP_SCHRITT_SEND_OPEN_SET");	// stephandling: copy actual step
      WriteStepDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "UDP_SCHRITT_SEND_OPEN_SET", StepConnect.Step.StepNr, udTimeTick);
			
      CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook,"Registration", "UDP_SCHRITT_SEND_OPEN_SET", "Register Variables", 0, 999, udTimeTick);
      CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
      
      /* --- Generierung der Anmeldung ------------------------------------------------- */
      uiLoopStartPv = uiActRegPv;
      
      if ((uiLoopStartPv >= MAX_VARIABLEN) &&     /* Abbruchbedingung, sobald die maximale Anzahl an Variablen überschritten ist */
        (bRegServFound == TRUE))                  /* Server fertig -> Eventauswertung auf Server kann angestartet werden */
      {
        bRegServFound = FALSE;
        StepConnect.Step.StepNr = UDP_SCHRITT_CLIENT_LINK_SET;
        break;
      }

      /* --- Abarbeitung der Variablenliste, zur Anmeldung an Server -------------------- */
      for (uiLoopOpenRequest = uiLoopStartPv; uiLoopOpenRequest < MAX_VARIABLEN; uiLoopOpenRequest++ )
      {
        /* --- Telegrammgenerierung --------------------------------------------------------- */
        if (pClientManagement->Modus == MODE_NEW)
        {
          GenerateOpenTelegrammX(&uiLoopOpenRequest, uiActRegServer, pVariablenVerwaltungStart, pClientVerwaltungStart, pSendBufOpen, &uiTelOpenLen, &sPvConnect, &uiPvCnt);
          uiActRegPv = uiLoopOpenRequest;
          
          CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook,"Registration", "UDP_SCHRITT_SEND_OPEN_SET", "Created Telegramm:NewMode", uiPvCnt, 999, udTimeTick);
          CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
          
          sServerConn[uiActRegServer].uiPvConnToServ+=uiPvCnt;
        }
        else if (pClientManagement->Modus == MODE_OLD)
        {
          bPvFound = GenerateOpenTelegramm(&uiLoopOpenRequest, uiActRegServer, pVariablenVerwaltungStart, pClientVerwaltungStart, pSendBufOpen, &uiTelOpenLen, &sPvConnect, &sPVOpenRequest);

          CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook,"Registration", "UDP_SCHRITT_SEND_OPEN_SET", "Created Telegramm:OldMode", bPvFound, 999, udTimeTick);
          CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
          
          if (bPvFound == FALSE)
          {
            uiPvCnt = 0;
            /* wenn keine PV gefunden wurde, kann abgebrochen werden -> LINK(en) */
            bRegServFound = FALSE;
            StepConnect.Step.StepNr = UDP_SCHRITT_CLIENT_LINK_SET;
            break;            
          }
          else
          {
            uiPvCnt = 1;
          }
          
          uiActRegPv = uiLoopOpenRequest;
          
          sServerConn[uiActRegServer].uiPvConnToServ+=uiPvCnt;
        }
        
        /* --- Versenden des Telegrammes -----------------------------  */
        UDPsendFub.enable   =   TRUE;
        UDPsendFub.ident  = UDPopenFub[0].ident;
        UDPsendFub.pHost  = (UDINT) &pClientManagement->IpAdr;
        UDPsendFub.pData  = (UDINT) pSendBufOpen;
        UDPsendFub.datalen  = uiTelOpenLen;
        UDPsendFub.port   = PortTarget[0];
        UDPsendFub.flags  =   udpSendFlags;
        UdpSend(&UDPsendFub);
        
        StepConnect.Step.StepNr = UDP_SCHRITT_SEND_OPEN;
        break;         
      }
            
      if ((uiLoopOpenRequest == MAX_VARIABLEN) && (bRegServFound == FALSE))
      {
        StepConnect.Step.StepNr = UDP_SCHRITT_NEUANMELDUNG;
      }      
      
      break; /* UDP_SCHRITT_SEND_CONN_SET */
    
    
    /* ---  Senden des OPEN Telegrammes --------------------------------------------------------------- */
    case UDP_SCHRITT_SEND_OPEN:
      brsstrcpy((UDINT) &StepConnect.Step.StepText, (UDINT) "UDP_SCHRITT_SEND_OPEN");	// stephandling: copy actual step
      WriteStepDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "UDP_SCHRITT_SEND_OPEN", StepConnect.Step.StepNr, udTimeTick);
			
      if (UDPsendFub.status != FUB_BUSY)
      {
        CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook,"Registration", "UDP_SCHRITT_SEND_OPEN", "UDPsendFub.status", UDPsendFub.status, 999, udTimeTick);
        CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
        
        if (UDPsendFub.status == FUB_OK)
        {
          /* --- mitloggen für Diagnosezwecke ------------------------------------------------------- */
          Diagnose.EventCnt.SendOpenReq++;
        
          /* --- Anmeldung erfolgreich? -------------------------------------------------------------- */
          StepConnect.Step.StepNr = UPD_SCHRITT_RECEIVE_OPEN_SET;
        }
        else
        {
          /* --- Abbruch, zur nächsten Client weiter --------------------------------------------- */
          StepConnect.Step.StepNr = UDP_SCHRITT_NEUANMELDUNG;
          
          /* --- Ackn failed -> spätere Neuanmeldung */
          NeuAnmeldungFehler[uiActRegServer] = TRUE;
            
          if (UDPsendFub.status != udpERR_NO_DATA)
          {
            Diagnose.EventCnt.SendOpenReqFailure++;
          }
        }
      }
      else
      {
        UdpSend(&UDPsendFub);
      }
      break; /* UDP_SCHRITT_SEND_OPEN */
    
    /* ---  Empfang des OPEN Telegrammes setzen -------------------------------------------------------- */
    case UPD_SCHRITT_RECEIVE_OPEN_SET:
      brsstrcpy((UDINT) &StepConnect.Step.StepText, (UDINT) "UPD_SCHRITT_RECEIVE_OPEN_SET");	// stephandling: copy actual step
      WriteStepDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "UPD_SCHRITT_RECEIVE_OPEN_SET", StepConnect.Step.StepNr, udTimeTick);
			
      UDPreceiveFub.ident = UDPopenFub[1].ident;
      StepConnect.Step.StepNr = UPD_SCHRITT_RECEIVE_OPEN;

      /* --- Zeitverzögerung aufziehen --------------------------------------------------------------- */
      TONFub.IN = TRUE;
      TONFub.PT = pClientManagement->Timeout;
      TON(&TONFub);
      
      /* --- hier ist kein Break notwendig -------------- */
    
    /* --- Empfang des Response - Telegrammes ------------------------------------------------- */
    case UPD_SCHRITT_RECEIVE_OPEN:
      brsstrcpy((UDINT) &StepConnect.Step.StepText, (UDINT) "UPD_SCHRITT_RECEIVE_OPEN");	// stephandling: copy actual step
      WriteStepDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "UPD_SCHRITT_RECEIVE_OPEN", StepConnect.Step.StepNr, udTimeTick);
			
      UdpRecv(&UDPreceiveFub);
      if (UDPreceiveFub.status == FUB_OK)
      {       
        Diagnose.EventCnt.RecvOpenSomething++;
          
        CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook,"Registration", "UPD_SCHRITT_RECEIVE_OPEN", "UDPreceiveFub.status", UDPreceiveFub.status, StepConnect.Step.StepNr, udTimeTick);
        CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
      
        /* --- Kennung umkopieren --------------------------------------------------------- */
        brsmemcpy((UDINT) &arKennung, (UDINT)pRecvBufOpen, 4);

        /* Kennung für 'OPEN' - alter Modus überprüfen */
        if (CheckTelegramHeader((UDINT*) &arKennung, (STRING*) TELEGRAMM_IDENT_OPEN) == TRUE)
        {
          CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "UPD_SCHRITT_RECEIVE_OPEN", "OLD MODE", 0x7FFFFFFF, StepConnect.Step.StepNr, udTimeTick);
          CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
        
          if (Diagnose.LogPointerInfo == TRUE)
          {
            ValidatePointerClient((UDINT) pRecvBufOpen, 15, (MemManagerDiag_type* ) &sMemDiag, 1);
          }
                      
          /* --- Telegrammauswertung des OPEN Schrittes --------------------------------- */
          CopyReceiveBufferToPVOpenResponse((UDINT*) pRecvBufOpen);
          
          Diagnose.EventCnt.RecvOpen++;
          CRCCheckTemp = CRCcheck((UDINT*) pRecvBufOpen, 20);
          if (CRCCheckTemp == PVOpenResponse.BCC)
          {       
            pClientManagement = pClientVerwaltungStart + PVOpenResponse.ClientIndex;
            if (Diagnose.LogPointerInfo == TRUE)
            {
              ValidatePointerClient((UDINT) pClientManagement, 16, (MemManagerDiag_type* ) &sMemDiag, 1);
            }
            
            /* --- Vgl. der Ip - Adressen, Fub gibt String zurück */            
            sdStrCmp = brsstrcmp((UDINT) &pClientManagement->IpAdr, (UDINT) UDPreceiveFub.pIpAddr);
            if (sdStrCmp == 0)
            {
              /* copy to new mode structure*/
              PVOpenResponseX.Status  = PVOpenResponse.Status;
              PVOpenResponseX.PVIndex = PVOpenResponse.PVIndex;
              PVOpenResponseX.PVIdent = PVOpenResponse.PVIdent;
              PVOpenResponseX.Laenge  = PVOpenResponse.Laenge;
              PVOpenResponseX.Wert    = PVOpenResponse.Wert;
              
              /* change actual variable access*/
              pVariablenManagement = pVariablenVerwaltungStart + (PVOpenResponse.PVIndex -1);
    
              /* copy connection values to the variable list entry*/
              uiState = AssignPvDataToListOpenX(PVOpenResponseX);
        
              /* --- Status zuordnen --------------------------------------------- */
              switch (uiState)
              {
                case 0:
                {
                  CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook,"Registration", (char*)  "UPD_SCHRITT_RECEIVE_OPEN", (char*) &pVariablenManagement->NameClient, 0x7FFFFFFF, StepConnect.Step.StepNr, udTimeTick);
                  CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
                                
                  SetPvStateReg((UDINT) pVariablenManagement, STAT_OK);
                  SetPvState((UDINT) pVariablenManagement, ERR_NO);
                  pVariablenManagement->StatusServ = 0;
                  sPvConnect.uiOk++;
                  sServerConn[uiActRegServer].uiPvLinked++;
                  break;
                }
                                            
                /* --- Server nicht ok ---------------------------------------- */
                case 1:
                {
                  CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook,"Registration", (char*)  "UPD_SCHRITT_RECEIVE_OPEN", (char*) &pVariablenManagement->NameClient, uiState, StepConnect.Step.StepNr, udTimeTick);
                  CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
                
                  SetPvStateReg((UDINT) pVariablenManagement, STAT_SERV_NOT_OK);
                  SetPvState((UDINT) pVariablenManagement, ERR_OPEN_FAILED);
                  pVariablenManagement->StatusServ = PVOpenResponseX.Status; 	/* Erweiterung, welcher Fehler kam den vom Server ?*/                  
                  sPvConnect.uiFailed++;
                  
                  break;
                }
                                            
                /* --- falscher Index ----------------------------------------- */
                case 2:
                {
                  CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook,"Registration", (char*) "UPD_SCHRITT_RECEIVE_OPEN", (char*) &pVariablenManagement->NameClient, uiState, StepConnect.Step.StepNr, udTimeTick);
                  CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
                
                  SetPvStateReg((UDINT) pVariablenManagement, STAT_PV_INDEX);
                  SetPvState((UDINT) pVariablenManagement, ERR_OPEN_FAILED);
                  sPvConnect.uiFailed++;
                  
                  break;
                }
                                          
                /* --- Nullpointer oder 0-Länge ------------------------------- */
                case 3:
                case 5:
                {
                  CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook,"Registration", (char*) "UPD_SCHRITT_RECEIVE_OPEN", (char*) &pVariablenManagement->NameClient, uiState, StepConnect.Step.StepNr, udTimeTick);
                  CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
                
                  SetPvStateReg((UDINT) pVariablenManagement, STAT_VAL_IS_NULL);
                  SetPvState((UDINT) pVariablenManagement, ERR_OPEN_FAILED);
                  sPvConnect.uiFailed++;
                  
                  break;
                }
                                            
                /* --- falsche Länge bei PV Wert - Zuordnung ------------------ */
                case 4:
                {
                  CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook,"Registration", (char*) "UPD_SCHRITT_RECEIVE_OPEN", (char*) &pVariablenManagement->NameClient, uiState, StepConnect.Step.StepNr, udTimeTick);
                  CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
                
                  SetPvStateReg((UDINT) pVariablenManagement, STAT_PV_ILL_LENGTH);
                  SetPvState((UDINT) pVariablenManagement, ERR_OPEN_FAILED);
                  sPvConnect.uiFailed++;
                  
                  break;
                }
                                          
                /* --- Default: Zustand nicht definiert */
                default:
                {
                  CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook,"Registration", (char*) "UPD_SCHRITT_RECEIVE_OPEN", (char*) &pVariablenManagement->NameClient, uiState, StepConnect.Step.StepNr, udTimeTick);
                  CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
                  
                  SetPvStateReg((UDINT) pVariablenManagement, uiState);
                  SetPvState((UDINT) pVariablenManagement, ERR_OPEN_FAILED);
                  sPvConnect.uiFailed++;
                  break;
                }
              }
            }
            else  /* (pClientVerwaltungStart->IPAdresse == UDPreceiveFub.ipadr) */
            {
              OpenFailed = TRUE;
            }
          }
          else  /* (CRCCheckTemp == PVOpenResponse.BCC */
          {
            OpenFailed = TRUE;
          }
      
          /* sollte der Fall auftreten, dass keine Variable angemeldet werden konnte, dann wird auf gescheitert geschaltet */
          if (sPvConnect.uiOk != 0)
          {
            StepConnect.Step.StepNr = UDP_SCHRITT_SEND_OPEN_SET;
          }
          else
          {
            /* Anmeldung gescheitert */
            OpenFailed = TRUE;
          }
        }
        /* --- Open für geblockten Modus - neuer Modus --------------------------------------------- */
        else if (CheckTelegramHeader((UDINT*) &arKennung, (STRING*) TELEGRAMM_IDENT_OPEX) == TRUE)
        {
          CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook,"Registration", "UPD_SCHRITT_RECEIVE_OPEN", "NEW MODE", 0x7FFFFFFF, StepConnect.Step.StepNr, udTimeTick);
          CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
          
          uiPvEntry = 0;
          Diagnose.EventCnt.RecvOpenX++;
          bValidate = ValidateDataFromRecvBuf(pRecvBufOpen, UDPreceiveFub.recvlen);
          if (bValidate)
          {         
            /* --- Anzahl der Variablen und ClientIndex ermitteln ---------------------- */
            GetMainDataFromRecvBufOpen(pRecvBufOpen, &uiPvCnt, &uiClientIndex);
              
            CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook,"Registration", "UPD_SCHRITT_RECEIVE_OPEN", "VAR-main-data", uiPvCnt, StepConnect.Step.StepNr, udTimeTick);
            CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
      
            /* --- alle Variablen aus Telegramm ermitteln ------------------------------ */
            do
            {
              /* --- eine Variable aus Telegramm ermitteln --------------------------- */
              bGetData = GetDataFromRecvBufOpen(pRecvBufOpen, uiPvCnt, &PVOpenResponseX, &uiPvEntry);
    
              /* --- gültige Daten wurden gefunden ------------------------------------ */
              if (bGetData)
              {                                 
                /* --- Clientliste ermitteln ---------------------------------------- */
                if ((PVOpenResponseX.PVIndex != 0) && (PVOpenResponseX.PVIndex < (MAX_VARIABLEN + 1)))
                {                 
                  pVariablenManagement = pVariablenVerwaltungStart + (PVOpenResponseX.PVIndex - 1);
                  pClientManagement = pClientVerwaltungStart + pVariablenManagement->Client;
                            
                  if (Diagnose.LogPointerInfo == TRUE)
                  {
                    ValidatePointerClient((UDINT) pVariablenManagement, 18, (MemManagerDiag_type* ) &sMemDiag, 1);
                    ValidatePointerClient((UDINT) pClientManagement, 19, (MemManagerDiag_type* ) &sMemDiag, 1);  
                  }
    
                  /* --- Vgl. der Ip - Adressen, Fub gibt String zurück ------------------ */
                  sdStrCmp = brsstrcmp((UDINT) &pClientManagement->IpAdr, (UDINT) UDPreceiveFub.pIpAddr);
                    
                  if (sdStrCmp == 0)
                  {
                    
                    uiState = AssignPvDataToListOpenX(PVOpenResponseX);
                                
                    /* --- Status zuordnen --------------------------------------------- */
                    switch (uiState)
                    {
                      case 0:
                      {
                        CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook,"Registration", (char*)  "UPD_SCHRITT_RECEIVE_OPEN", (char*) &pVariablenManagement->NameClient, 0x7FFFFFFF, StepConnect.Step.StepNr, udTimeTick);
                        CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
                          
                        SetPvStateReg((UDINT) pVariablenManagement, STAT_OK);
                        SetPvState((UDINT) pVariablenManagement, ERR_NO);
                        pVariablenManagement->StatusServ = 0;
                        sPvConnect.uiOk++;
                        sServerConn[uiActRegServer].uiPvLinked++;
                        break;
                      }
                                      
                      /* --- Server nicht ok ---------------------------------------- */
                      case 1:
                      {
                        CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook,"Registration", (char*)  "UPD_SCHRITT_RECEIVE_OPEN", (char*) &pVariablenManagement->NameClient, uiState, StepConnect.Step.StepNr, udTimeTick);
                        CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
                        
                        SetPvStateReg((UDINT) pVariablenManagement, STAT_SERV_NOT_OK);
                        SetPvState((UDINT) pVariablenManagement, ERR_OPEN_FAILED);
                        pVariablenManagement->StatusServ = PVOpenResponseX.Status; 	/* Erweiterung, welcher Fehler kam den vom Server ?*/
                        sPvConnect.uiFailed++;
                                         
                        break;
                      }
                                      
                      /* --- falscher Index ----------------------------------------- */
                      case 2:
                      {
                        CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook,"Registration", (char*) "UPD_SCHRITT_RECEIVE_OPEN", (char*) &pVariablenManagement->NameClient, uiState, StepConnect.Step.StepNr, udTimeTick);
                        CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
                        
                        SetPvStateReg((UDINT) pVariablenManagement, STAT_PV_INDEX);
                        SetPvState((UDINT) pVariablenManagement, ERR_OPEN_FAILED);
                        sPvConnect.uiFailed++;
                        
                        break;
                      }
                                    
                      /* --- Nullpointer oder 0-Länge ------------------------------- */
                      case 3:
                      case 5:
                      {
                        CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook,"Registration", (char*) "UPD_SCHRITT_RECEIVE_OPEN", (char*) &pVariablenManagement->NameClient, uiState, StepConnect.Step.StepNr, udTimeTick);
                        CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
                        
                        SetPvStateReg((UDINT) pVariablenManagement, STAT_VAL_IS_NULL);
                        SetPvState((UDINT) pVariablenManagement, ERR_OPEN_FAILED);
                        sPvConnect.uiFailed++;
                        
                        break;
                      }
                                      
                      /* --- falsche Länge bei PV Wert - Zuordnung ------------------ */
                      case 4:
                      {
                        CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook,"Registration", (char*) "UPD_SCHRITT_RECEIVE_OPEN", (char*) &pVariablenManagement->NameClient, uiState, StepConnect.Step.StepNr, udTimeTick);
                        CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
                        
                        SetPvStateReg((UDINT) pVariablenManagement, STAT_PV_ILL_LENGTH);
                        SetPvState((UDINT) pVariablenManagement, ERR_OPEN_FAILED);
                        sPvConnect.uiFailed++;
                        
                        break;
                      }
                                    
                      /* --- Default: Zustand nicht definiert */
                      default:
                      {
                        CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook,"Registration", (char*) "UPD_SCHRITT_RECEIVE_OPEN", (char*) &pVariablenManagement->NameClient, uiState, StepConnect.Step.StepNr, udTimeTick);
                        CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
                        
                        SetPvStateReg((UDINT) pVariablenManagement, uiState);
                        SetPvState((UDINT) pVariablenManagement, ERR_OPEN_FAILED);
                        
                        sPvConnect.uiFailed++;
                        break;
                      }
                    }
                  } /* if (sdStrCmp == 0) */
                }
                else  /*  if (PVOpenResponseX.PVIndex != 0) */
                {               
                  CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook,"Registration", "UPD_SCHRITT_RECEIVE_OPEN", "ERR_ILLEGAL_PV_INDEX", PVOpenResponseX.PVIndex, StepConnect.Step.StepNr, udTimeTick);
                  CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
            
                  OpenFailed = TRUE;
                }
              }
            } while (bGetData == TRUE);
          }
          
          /* sollte der Fall auftreten, dass keine Variable angemeldet werden konnte, dann wird auf gescheitert geschaltet */
          if (sPvConnect.uiOk != 0)
          {
            CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook,"Registration", "UPD_SCHRITT_RECEIVE_OPEN", "variable(s) registered", sPvConnect.uiOk, StepConnect.Step.StepNr, udTimeTick);
            CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;      
            StepConnect.Step.StepNr = UDP_SCHRITT_SEND_OPEN_SET;
          }
          else
          {
            CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook,"Registration", "UPD_SCHRITT_RECEIVE_OPEN", "no variable(s) registered", 0x7FFFFFFF, StepConnect.Step.StepNr, udTimeTick);
            CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
            
            /* Anmeldung gescheitert */
            OpenFailed = TRUE;
          }

          /* CRC */
          /* Ip - Adressen */
          /* Daten umkopieren */
        }
        else  /*  if ((arKennung[0] == 'O') && */
        {
          CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook,"Registration", "UPD_SCHRITT_RECEIVE_OPEN", "telegram sign wrong", arKennung[0], StepConnect.Step.StepNr, udTimeTick);
          CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
          
          OpenFailed = TRUE;
        }
      } 
      else  /* if (UDPreceiveFub.status == FUB_OK) */
      {
        if ((UDPreceiveFub.status != ERR_FUB_BUSY) && (UDPreceiveFub.status != udpERR_NO_DATA))
        {
          CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook,"Registration", "UPD_SCHRITT_RECEIVE_OPEN", "UDPreceiveFub.status", UDPreceiveFub.status, StepConnect.Step.StepNr, udTimeTick);
          CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
        
        }
        OpenFailed = TRUE;
      }
      
    /* --- Falls Auswertung bei Variable gescheitert ist, so wird unten über die weitere Vorgehensweise entschieden */  
    if (OpenFailed == TRUE) 
    {
      OpenFailed = FALSE;

      if (OpenErrorCnt == 0)
      {
        OpenErrorCnt++;
      }       
                            
      TON(&TONFub);
      
      if (TONFub.Q == TRUE)
      {
        TONFub.IN = FALSE;
        TON(&TONFub);
        
        TimeoutCnt++; /* Zeit abgelaufen*/
        if (TimeoutCnt < TIMEOUT_RETRY)
        {
          CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook,"Registration", "UPD_SCHRITT_RECEIVE_OPEN", "proceed to next step", 0x7FFFFFFF, StepConnect.Step.StepNr, udTimeTick);
          CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
              
          Diagnose.EventCnt.SendOpenResp++;
          TONFub.IN = TRUE;
          TON(&TONFub);
          UdpSend(&UDPsendFub);
          StepConnect.Step.StepNr = UDP_SCHRITT_SEND_OPEN;
        }
        else /* --- weiter mit nächstem Server */
        {
          CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook,"Registration", "UPD_SCHRITT_RECEIVE_OPEN", "TIMEOUT", 0x7FFFFFFF, StepConnect.Step.StepNr, udTimeTick);
          CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
              
          pVariablenManagement->Status   = ERR_TIMEOUT;
          brsmemcpy((UDINT) &pVariablenEvent->StatusTxt, (UDINT) ERR_TIMEOUT_TXT, sizeof(pVariablenEvent->StatusTxt));
          pVariablenManagement->Anmeldung  = STAT_TIMEOUT;
              
          /* --- für spätere Neuanmeldung --------------------------------- */
          /* nur bei Timeout wird ständig versucht, den kompletten Client neu anzumelden */
          NeuAnmeldungFehler[uiActRegServer] = TRUE;
                          
          TimeoutCnt = 0;
          StepConnect.Step.StepNr = UDP_SCHRITT_SEND_OPEN_SET;
            
          /* --- Logbucheintrag -------------------------------------------- */
          if ((UDPreceiveFub.status != udpERR_NO_DATA) || (UDPreceiveFub.status != ERR_FUB_BUSY)) 
          {
            if (LogbuchCnt < LOGBOOK_ENTRIES_CONN)
            {
              if (uiActRegPv < MAX_VARIABLEN)	/* gültiger Variablenindex */
              {
                LogbuchEintrag((UDINT*) pClientVerwaltungStart, uiActRegPv, LogbuchCnt, 2 );
                LogbuchCnt++; 
              }
            }
          } /* if (UDPreceiveFub.status != udpERR_NO_DATA) */
        } /* if (TimeoutCnt < TIMEOUT_RETRY) */
      } /*  (TONFub.Q == TRUE) */
    } /* if (OpenFailed == TRUE) */
    else
    {
      /* --- Timer rücksetzen  ------------------------------------ */
      TONFub.IN = FALSE;
      TON(&TONFub);
    } 
    break; /* UPD_SCHRITT_RECEIVE_OPEN */
    
    /* #################################### Senden des Link - Telegrammes #######################################*/
    case UDP_SCHRITT_CLIENT_LINK_SET:
      brsstrcpy((UDINT) &StepConnect.Step.StepText, (UDINT) "UDP_SCHRITT_CLIENT_LINK_SET");	// stephandling: copy actual step
      WriteStepDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "UDP_SCHRITT_CLIENT_LINK_SET", StepConnect.Step.StepNr, udTimeTick);

      CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "UDP_SCHRITT_CLIENT_LINK_SET", "Link Telegramm", 0x7FFFFFFF, StepConnect.Step.StepNr, udTimeTick);
      CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
      
      /* --- Telegrammkennung erzeugen ------------------------------------------------- */
      GenerateLinkHeaderTelegramm( (UDINT*) &ClientLink.Kennung, pClientManagement->Modus);

      ClientLink.ClientIndex = pClientManagement->ClientIndex;
    
      /* --- Sicherheitskopie für Diagnose ---- */
      brsmemcpy((UDINT) &ClientLinkSave, (UDINT) &ClientLink, sizeof(ClientLink_typ));
      
      CopyClientLinkToSendBuffer((UDINT*) pSendBufOpen);
      
      UDPsendFub.enable   =   TRUE;
      UDPsendFub.ident  = UDPopenFub[0].ident;
      UDPsendFub.pHost  = (UDINT) &pClientManagement->IpAdr;
      UDPsendFub.pData  = (UDINT) pSendBufOpen;
      UDPsendFub.datalen  = sizeof(ClientLink);
      UDPsendFub.port   = PortTarget[0];
      UDPsendFub.flags  =   udpSendFlags;
      UdpSend(&UDPsendFub);
      
      /* Timeout aufziehen*/
      TONFub.IN = TRUE;
      TON(&TONFub);
      StepConnect.Step.StepNr = UDP_SCHRITT_CLIENT_LINK;

    /* -> kein Break notwendig */
        
    /***********************************************************************************************************/
    case UDP_SCHRITT_CLIENT_LINK:
      brsstrcpy((UDINT) &StepConnect.Step.StepText, (UDINT) "UDP_SCHRITT_CLIENT_LINK");	// stephandling: copy actual step
      WriteStepDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "UDP_SCHRITT_CLIENT_LINK", StepConnect.Step.StepNr, udTimeTick);

      if (UDPsendFub.status != ERR_FUB_BUSY)
      {
        if (UDPsendFub.status == FUB_OK)
        {
          Diagnose.EventCnt.LinkRequest++;
        
          CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook,"Registration", "UDP_SCHRITT_CLIENT_LINK", "UDPsendFub.status", UDPsendFub.status, StepConnect.Step.StepNr, udTimeTick);
          CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
          
          StepConnect.Step.StepNr = UDP_SCHRITT_CLIENT_RECEIVE_SET;
        }
        else
        {
          
          CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook,"Registration", "UDP_SCHRITT_CLIENT_LINK", "UDPsendFub.status", UDPsendFub.status, StepConnect.Step.StepNr, udTimeTick);
          CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
          
          StepConnect.Step.StepNr = UDP_SCHRITT_SEND_OPEN_SET; /* Abbruch, zur nächsten Variable weiter */
        }                    
      }
      else
      {
        UdpSend(&UDPsendFub);
      }
      
      break; /* UDP_SCHRITT_CLIENT_LINK */
    
    /***********************************************************************************************************/   
    case UDP_SCHRITT_CLIENT_RECEIVE_SET:
      brsstrcpy((UDINT) &StepConnect.Step.StepText, (UDINT) "UDP_SCHRITT_CLIENT_RECEIVE_SET");	// stephandling: copy actual step
      WriteStepDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "UDP_SCHRITT_CLIENT_RECEIVE_SET", StepConnect.Step.StepNr, udTimeTick);

      UDPreceiveFub.ident = UDPopenFub[1].ident;
      StepConnect.Step.StepNr = UDP_SCHRITT_CLIENT_RECEIVE;
    /* -- kein break notwendig */

    /***********************************************************************************************************/       
    case UDP_SCHRITT_CLIENT_RECEIVE:
      brsstrcpy((UDINT) &StepConnect.Step.StepText, (UDINT) "UDP_SCHRITT_CLIENT_RECEIVE");	// stephandling: copy actual step
      WriteStepDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook, "Registration", "UDP_SCHRITT_CLIENT_RECEIVE", StepConnect.Step.StepNr, udTimeTick);

      UdpRecv(&UDPreceiveFub);
      if (UDPreceiveFub.status == FUB_OK)
      {
        CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook,"Registration", "UDP_SCHRITT_CLIENT_RECEIVE", "UDPreceiveFub.status", UDPreceiveFub.status, StepConnect.Step.StepNr, udTimeTick);
        CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
        
        /* Telegrammauswertung des OPEN Schrittes */
        CopyReceiveBufferToNAK((UDINT*) pRecvBufOpen);
  
        
        if (CheckTelegramHeader((UDINT*) &NAK.Kennung, (STRING*) TELEGRAMM_IDENT_ACKN) == TRUE)
        {          
          if (NAK.Status == FUB_OK)
          {
            CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook,"Registration", "UDP_SCHRITT_CLIENT_RECEIVE", "NAK.Status", NAK.Status, StepConnect.Step.StepNr, udTimeTick);
            CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
            
            /* --- Timer für Lebensüberwachung einschalten ---------------------------------- */
            pClientManagement->LifeCheckTime = udTimeTick;            
            pClientManagement->LifeCheck = STATE_LIFE_OK;            
                  
            Diagnose.EventCnt.LinkResponse++;
                
            /* --- Timeout wieder ausschalten ----------------------------------------------- */
            TONFub.IN = FALSE; 
            TON(&TONFub);
                
            /* --- Server x angemeldet, weiter zum nächsten --------------------------------- */          
            StepConnect.Step.StepNr = UDP_SCHRITT_NEUANMELDUNG;
                  
            /* --- Meldung f. Anzahl der Variablen ins Logbuch schreiben --------------------- */
            WriteConnPvInLogbook(sPvConnect, pClientManagement, uiActRegServer, TRUE);
      
            /* --- Zurücksetzen der Statistik - Werte ---------------------------------------- */
            ResetConnPv(&sPvConnect);
              
          } /*if (NAK.Status == FUB_OK)*/
          else
          {
            CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook,"Registration", "UDP_SCHRITT_CLIENT_RECEIVE", "NAK.Status", NAK.Status, StepConnect.Step.StepNr, udTimeTick);
            CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
            
            AcknowledgeFailed = TRUE;
          }
        } /*  (PVOpenResponse.Kennung[0] == 'A')  */
        else
        {
          CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook,"Registration", "UDP_SCHRITT_CLIENT_RECEIVE", "Checktelegramm header", NAK.Status, StepConnect.Step.StepNr, udTimeTick);
          CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;    
          
          AcknowledgeFailed = TRUE;
        }
      }/*  (UDPreceiveFub.status == FUB_OK)*/
      else
      {
        if ((UDPreceiveFub.status != ERR_FUB_BUSY) && (UDPreceiveFub.status != udpERR_NO_DATA))
        {
          CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook,"Registration", "UDP_SCHRITT_CLIENT_RECEIVE", "UDPreceiveFub.status", UDPreceiveFub.status, StepConnect.Step.StepNr, udTimeTick);
          CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
        }
        AcknowledgeFailed = TRUE;
      } /* if (UDPreceiveFub.status == FUB_OK)*/
      
      /* --- Falls kein Antworttelegramm empfangen wurde, muß entschieden werden ob nochmals angefragt wird */
      if (AcknowledgeFailed == TRUE)
      {
        AcknowledgeFailed = FALSE;

        TON(&TONFub);
        if (TONFub.Q == TRUE)
        {
          TONFub.IN = FALSE;
          TON(&TONFub);
          
          TimeoutCnt++; /* Zeit abgelaufen*/
          if (TimeoutCnt < TIMEOUT_RETRY)
          {
            CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook,"Registration", "UDP_SCHRITT_CLIENT_RECEIVE", "TIMEOUT", 0, StepConnect.Step.StepNr, udTimeTick);
            CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
            
            TONFub.IN = TRUE;
            TON(&TONFub);
            UdpSend(&UDPsendFub);
            StepConnect.Step.StepNr  = UDP_SCHRITT_SEND_OPEN;
            Lebensueberwachung[uiActRegServer] = TRUE;
          }
          else/* --- weiter mit nächstem Server -------- */
          {
            CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Connect, &CmdBurLogBook,"Registration", "UDP_SCHRITT_CLIENT_RECEIVE", "next server", 0, StepConnect.Step.StepNr, udTimeTick);
            CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
            
            /* --- Ackn failed -> spätere Neuanmeldung */
            NeuAnmeldungFehler[uiActRegServer] = TRUE;
            
            TimeoutCnt = 0;
            StepConnect.Step.StepNr = UDP_SCHRITT_SEND_OPEN_SET;
          }   
        } /*  (TON10msFub.Q == TRUE) */
      }/*if (AcknowledgeFailed == TRUE)*/
      else
      {
        /* --- Timer rücksetzen */
        TONFub.IN = FALSE;
        TON(&TONFub);
      }
      break; /* UDP_SCHRITT_CLIENT_RECEIVE */
      
  }/* switch (StepConnect.Step.StepNr) -> Ende Schrittschaltwerk Anmeldung */
}


/********************************************************************************************************
* --- Eventauswertung
********************************************************************************************************/

void CyclicSLSEventhandleToUst()
{
  UINT uiErrAssign;
  UINT uiErrAssignCnt;

  uiErrAssign = 0;
  uiErrAssignCnt = 0;
  // Stephandler: overhead /initialisierung
  if (StepEvent.StepHandling.Current.bTimeoutElapsed == 1)
  {
    StepEvent.StepHandling.Current.bTimeoutElapsed = 0;
    StepEvent.Step.StepNr = StepEvent.StepHandling.Current.nTimeoutContinueStep;
  }
  StepEvent.StepHandling.Current.nStepNr = (DINT) StepEvent.Step.StepNr;
  brsstrcpy((UDINT) &StepEvent.StepHandling.Current.sStepText, (UDINT) &StepEvent.Step.StepText);
  BrbStepHandler(&StepEvent.StepHandling);
  
  switch (StepEvent.Step.StepNr)
  {   
    case EVENT_SCHRITT_INIT:
      brsstrcpy((UDINT) &StepEvent.Step.StepText, (UDINT) "EVENT_SCHRITT_INIT");	// stephandling: copy actual step
      WriteStepDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Event, &CmdBurLogBook, "Events", "EVENT_SCHRITT_INIT", StepEvent.Step.StepNr, udTimeTick);
			
      StepEvent.Step.InitDone = TRUE;
      StepEvent.Step.StepNr = EVENT_SCHRITT_SET;
      break;
    
    case EVENT_SCHRITT_SET:
      brsstrcpy((UDINT) &StepEvent.Step.StepText, (UDINT) "EVENT_SCHRITT_SET");	// stephandling: copy actual step
      WriteStepDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Event, &CmdBurLogBook, "Events", "EVENT_SCHRITT_SET", StepEvent.Step.StepNr, udTimeTick);
      
      UDPreceiveEventFub.enable = TRUE;
      UDPreceiveEventFub.ident  = UDPopenFub[3].ident;
      UDPreceiveEventFub.pData  = (UDINT) pRecvBufEvnt;
      UDPreceiveEventFub.datamax  = MAX_UDP_FRAME_LEN;
      UDPreceiveEventFub.pIpAddr  = (UDINT) &arRecvEvntIpAdr;
      StepEvent.Step.StepNr = EVENT_SCHRITT;     
    /* hier kein break ! */
    
    /*#########################################################################################*/   
    case EVENT_SCHRITT:
      brsstrcpy((UDINT) &StepEvent.Step.StepText, (UDINT) "EVENT_SCHRITT");	// stephandling: copy actual step
      WriteStepDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Event, &CmdBurLogBook, "Events", "EVENT_SCHRITT", StepEvent.Step.StepNr, udTimeTick);
      
      UdpRecv(&UDPreceiveEventFub);
      if (UDPreceiveEventFub.status == FUB_OK)
      {   
        CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Event, &CmdBurLogBook, "Events", "EVENT_SCHRITT: data available: next entry ", "UDPreceiveEventFub.status", UDPreceiveEventFub.status, StepEvent.Step.StepNr, udTimeTick);
        CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
      
        /* --- Kennung umkopieren --------------------------------------------------------- */
        brsmemcpy((UDINT) &arKennung, (UDINT)pRecvBufEvnt, 4);
        bEventGet = FALSE;

        //TODO: macht nichts: code läuft immer drüber. => ändern 
        if (UDPreceiveEventFub.recvlen != 0)
        {             
          CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Event, &CmdBurLogBook, "Events", "EVENT_SCHRITT - data from station:", (char*) &arRecvEvntIpAdr, UDPreceiveEventFub.recvlen, StepEvent.Step.StepNr, udTimeTick);
          CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
           
          /* --- Telegrammauswertung für alten Modus (ein Telegramm für einen Wert) ------ */
          if (CheckTelegramHeader((UDINT*) &arKennung, (STRING*) TELEGRAMM_IDENT_EVNT) == TRUE)
          {         
            Diagnose.EventCnt.EvntReceiveData++;
            
            CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Event, &CmdBurLogBook, "Events", "EVENT_SCHRITT", "recv. data for old mode", 0x7FFFFFFF, StepEvent.Step.StepNr, udTimeTick);
            CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
            
            CopyReceiveBufferToPVEvent((UDINT*) pRecvBufEvnt);
       
            CRCCheckTemp = CRCcheck((UDINT*) pRecvBufEvnt, 14);

            /* 1. CRC             */
            /* 2. Kontrolle, ob gültiger Server vorhanden ist -> >0!*/
            /* 3. Längenvergleich der Variable            */
            if (CRCCheckTemp == PVEvent.BCC)
            {
              pVariablenEvent = pVariablenVerwaltungStart + PVEvent.PVIndex - 1;
              /* --- Zeiger auf Client Liste -------------------------------------------------------- */
              pClientEvent = pClientVerwaltungStart + pVariablenEvent->Client;
                
              if (Diagnose.LogPointerInfo == TRUE)
              {
                ValidatePointerClient((UDINT) pVariablenEvent, 30, (MemManagerDiag_type* ) &sMemDiag, 1);
                ValidatePointerClient((UDINT) pClientEvent, 31, (MemManagerDiag_type* ) &sMemDiag, 1);
              }
              
              /* --- Vergleich der IP - Adresse des UDPrcv Fubs mit abgespeicherter IP - Adresse */
              /* --- wenn die IP - Adressen nicht gleich sind, so kann das Telegramm verworfen werden */
              sdStrCmp = brsstrcmp((UDINT) &pClientEvent->IpAdr, (UDINT) UDPreceiveEventFub.pIpAddr);
              if (sdStrCmp != 0)
              {
                Diagnose.EventCnt.WrongStationCount++;
                break;
              }
              
              PVEventXCopy.PVIndex = PVEvent.PVIndex;
              PVEventXCopy.Laenge = PVEvent.Laenge ;
              PVEventXCopy.Wert = PVEvent.Wert;

              uiErrAssign = AssignDataForEvntPv(PVEventXCopy, &uiEventState);
              if (uiErrAssign != 0)
              {
                uiErrAssignCnt++;
              }
            } /* (CRCCheckTemp == PVEvent.BCC) */
            else
            {
              PVEventAcknowledge.Status   = ERR_BCC;
              
              /* Fehler Telegrammchecksum*/
              CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Event, &CmdBurLogBook, "Events", "EVENT_SCHRITT", "checksum wrong", PVEvent.BCC, StepEvent.Step.StepNr, udTimeTick);
              CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
            }

            PVEventAcknowledge.PVIndex    = PVEvent.PVIndex;          /* Index wird zurückgesendet */
            PVEventAcknowledge.EventCnt   = PVEvent.EventCnt;         /* Identisch zu ClientIndex*/
            PVEventAcknowledge.ClientIndex  = pClientEvent->ClientIndex;    /* Übergabe ClientIndex   */
            if (uiErrAssignCnt == 0)
            {
              bEventGet = TRUE;
            }
            else
            {
              bEventGet = FALSE;
            }
          }
          /* --- Telegrammauswertung für neuen Modus (ein Telegramm für mehrere Werte) ------ */
          else if (CheckTelegramHeader((UDINT*) &arKennung, (STRING*) TELEGRAMM_IDENT_EVNX) == TRUE)          
          {
            CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Event, &CmdBurLogBook, "Events", "EVENT_SCHRITT", "data for new mode", 0x7FFFFFFF, StepEvent.Step.StepNr, udTimeTick);
            CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
            
            Diagnose.EventCnt.EvntReceiveData++;
			
            /* --- CRC: sind die Daten ok ------------------------------------------------------- */
            bValidate = ValidateDataFromRecvBuf((UDINT*) pRecvBufEvnt, UDPreceiveEventFub.recvlen);
            if (bValidate)
            { 
              uiPvEntry = 0;
              uiPvCnt = 0;
              uiEventCnt = 0;
              bCheckIpAdr = FALSE;
              
              /* --- Anzahl der Variablen und ClientIndex (uiEventCnt) ermitteln --------- */
              GetMainDataFromRecvBufEvent(pRecvBufEvnt, &uiPvCnt, &uiEventCnt);
              PVEventAcknowledge.Status = ERR_NO;
          
              CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Event, &CmdBurLogBook, "Events", "EVENT_SCHRITT", "Pv data", uiPvCnt, StepEvent.Step.StepNr, udTimeTick);
              CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
              
              /* --- hier wird einmalig überprüft, ob Client mit Ip-Adresse übereinstimmt */
              /* --- geht nicht früher, da die Referenzierung des Clients über PV erfolgt */
              if (GetDataFromRecvBufEvent(pRecvBufEvnt, uiPvCnt, &PVEventX, &uiPvEntry))
              {
                if (CheckIpAdrForClient(PVEventX, UDPreceiveEventFub.pIpAddr) == FALSE)
                {
                  CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Event, &CmdBurLogBook, "Events", "EVENT_SCHRITT", "wrong ip adress", 0x7FFFFFFF, StepEvent.Step.StepNr, udTimeTick);
                  CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
                      
                  Diagnose.EventCnt.WrongStationCount++;
                  break;
                }

                /* --- Zeiger auf Client Liste für Timeout (Lifecheck) Verwaltung ------ */
                if ((PVEventX.PVIndex > 0) && (PVEventX.PVIndex <= MAX_VARIABLEN))
                {
                  pVariablenEvent = pVariablenVerwaltungStart + PVEventX.PVIndex - 1; 	/* welche Variable wird bearbeitet */
                  pClientEvent = pClientVerwaltungStart + pVariablenEvent->Client;		/* von welchem Server wird PV angemeldet */
  
                  if (Diagnose.LogPointerInfo == TRUE)
                  {
                    ValidatePointerClient((UDINT) pVariablenEvent, 32, (MemManagerDiag_type* ) &sMemDiag, 1);
                    ValidatePointerClient((UDINT) pClientEvent, 33, (MemManagerDiag_type* ) &sMemDiag, 1);
                  }
                }
                else
                {
                  if (Diagnose.LogPointerInfo == TRUE)
                  {
                    /* Fehler? Zusätzliche Einträge ins Logbuch - Index + Ip Adresse */
                    AddErrInfoEventX(PVEventX.PVIndex, (UDINT*) &arRecvEvntIpAdr);
                  }
                    
                  CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Event, &CmdBurLogBook, "Events", "EVENT_SCHRITT", "wrong pvi index", PVEventX.PVIndex, StepEvent.Step.StepNr, udTimeTick);
                  CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
                    
                  /* Abbruch, da falscher PV Index */
                  Diagnose.EventCnt.WrongPvIndex++;
                  break;
                }
                  
                  if (pClientEvent->LifeCheck != STATE_LIFE_OK)
                  {
                    Diagnose.EventCnt.EventLifeFailed++;
                    break;
                  }
              }
              uiPvEntry = 0;
              
              /* --- alle Variablen aus Telegramm ermitteln --------------------------- */
              do
              {
                /* --- eine Variable aus Telegramm ermitteln -------------------------- */
                bGetData = GetDataFromRecvBufEvent(pRecvBufEvnt, uiPvCnt, &PVEventX, &uiPvEntry);                
                if (bGetData == TRUE)
                {
                  Diagnose.EventCnt.EvntGetPVData++;
  
                  /* --- gültige Daten wurden gefunden ?------------------------------- */
                  uiErrAssign = AssignDataForEvntPv(PVEventX, &uiEventState);
                  if (uiErrAssign != 0)
                  {
                    uiErrAssignCnt++;
                  }
                }
              }while (bGetData == TRUE);
              
            } /* if (bValidate) */
            else
            {
              PVEventAcknowledge.Status = ERR_BCC;
             
              CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Event, &CmdBurLogBook, "Events", "EVENT_SCHRITT", "CRC error", 0, StepEvent.Step.StepNr, udTimeTick);
              CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
            }
                        
            PVEventAcknowledge.PVIndex      = PVEventX.PVIndex;             /* Index wird zurückgesendet */
            PVEventAcknowledge.EventCnt     = uiEventCnt;
            PVEventAcknowledge.ClientIndex  = pClientEvent->ClientIndex;    /* Übergabe ClientIndex   */
            if (uiErrAssignCnt == 0)
            { 
              /* hat alles funktioniert: dann Lifecheck anpassen und Antwort senden*/
              bEventGet = TRUE;
            }
          } /* neuer Modus */
        } /* if (UDPreceiveEventFub.recvlen != 0) */
        
        if (bEventGet)
        {
          /* --- Telegramm generieren und versenden --------------------------------------- */
          brsmemcpy((UDINT) &PVEventAcknowledge.Kennung, (UDINT) "ACKN", 4);
          brsmemcpy((UDINT) &PVEventAcknowledgeSave, (UDINT) &PVEventAcknowledge, sizeof(PVEventAcknowledge)); /* Sicherheitskopie für Diagnose */
  
          CopyPVEventAcknowledgeToSendBuffer((UDINT*)pSendBufEvnt);
          
          UDPsendEventFub.enable  = TRUE;
          UDPsendEventFub.ident   = UDPopenFub[2].ident;
          UDPsendEventFub.pHost   = UDPreceiveEventFub.pIpAddr;
          UDPsendEventFub.pData   = (UDINT) pSendBufEvnt;
          UDPsendEventFub.datalen = sizeof(PVEventAcknowledge);
          UDPsendEventFub.port    = PortTarget[3];
          UDPsendEventFub.flags   = udpSendFlags;
          UdpSend(&UDPsendEventFub);
  
          StepEvent.Step.StepNr = EVENT_SCHRITT_RESPONSE;
          
          /* --- Lifecheck ok --------------------------------------------------------------- */
          Lebensueberwachung[pVariablenEvent->Client] = FALSE;
        
          /* --- aktueller Server ist identisch mit Server Lifecheck -> Counter auf 0 ------- */
          if (LifeCntAct == pVariablenEvent->Client)
          {
            pClientEvent->LifeCheckTrial = 0;
          }

          pClientEvent->LifeCheck = STATE_LIFE_OK;    /* Verbindung vorhanden; Server sendet noch Daten */
          pClientEvent->LifeCheckTime = udTimeTick;   /* Lebensüberwachung: Timerwert aktualisieren -> Telegramm empfangen, Station vorhanden */
        }
      } /* (UDPreceiveEventFub.status == FUB_OK) */
      break;    
    
    /*###########################################################################################################*/
    case EVENT_SCHRITT_RESPONSE:
      brsstrcpy((UDINT) &StepEvent.Step.StepText, (UDINT) "EVENT_SCHRITT_RESPONSE");	// stephandling: copy actual step
      WriteStepDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Event, &CmdBurLogBook, "Events", "EVENT_SCHRITT_RESPONSE", StepEvent.Step.StepNr, udTimeTick);
      
      if (UDPsendEventFub.status != ERR_FUB_BUSY)
      {
        if (UDPsendEventFub.status == FUB_OK)
        {
          CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Event, &CmdBurLogBook, "Events", "EVENT_SCHRITT_RESPONSE", "UDPsendEventFub.status", UDPsendEventFub.status, StepEvent.Step.StepNr, udTimeTick);
          CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
          
          Diagnose.EventCnt.EvntResp++;
        }
        else
        {
          if (UDPsendEventFub.status != udpERR_NO_DATA)
          {
            CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Event, &CmdBurLogBook, "Events", "EVENT_SCHRITT_RESPONSE", "UDPsendEventFub.status", UDPsendEventFub.status, StepEvent.Step.StepNr, udTimeTick);
            CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
          }
        }
        StepEvent.Step.StepNr = EVENT_SCHRITT;
      }
      else
      {
        UdpSend(&UDPsendEventFub);
      }
      break;
  } /* switch (StepEvent.Step.StepNr) */
}


/************************************************************************************************************
* --- Lebensüberwachung
* --- Überprüfung, ob alle Timer ungleich 0 sind -> bei 0 wird Lifechecktelegramm gesendet
*************************************************************************************************************/

void CyclicSLSLifeCheckForUst()
{
  // Stephandler: overhead /initialisierung
  if (StepLifeCheck.StepHandling.Current.bTimeoutElapsed == 1)
  {
    StepLifeCheck.StepHandling.Current.bTimeoutElapsed = 0;
    StepLifeCheck.Step.StepNr = StepLifeCheck.StepHandling.Current.nTimeoutContinueStep;
  }
  StepLifeCheck.StepHandling.Current.nStepNr = (DINT) StepLifeCheck.Step.StepNr;
  brsstrcpy((UDINT) &StepLifeCheck.StepHandling.Current.sStepText, (UDINT) &StepLifeCheck.Step.StepText);
  BrbStepHandler(&StepLifeCheck.StepHandling);
  
  switch (StepLifeCheck.Step.StepNr)
  { 
    case LIFE_CHECK_INIT:
      brsstrcpy((UDINT) &StepLifeCheck.Step.StepText, (UDINT) "LIFE_CHECK_INIT");	// stephandling: copy actual step
      WriteStepDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Lifecheck, &CmdBurLogBook, "LifeCheck", "LIFE_CHECK_INIT", StepLifeCheck.Step.StepNr, udTimeTick);
			
      StepLifeCheck.Step.InitDone = TRUE;
      StepLifeCheck.Step.StepNr = LIFE_CHECK_SEND;
      break;
    
    case LIFE_CHECK_SEND:
      brsstrcpy((UDINT) &StepLifeCheck.Step.StepText, (UDINT) "LIFE_CHECK_SEND");	// stephandling: copy actual step
      WriteStepDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Lifecheck, &CmdBurLogBook, "LifeCheck", "LIFE_CHECK_SEND", StepLifeCheck.Step.StepNr, udTimeTick);
			
      for (LifeCnt = LifeCheck.Server; LifeCnt < MAX_SERVER; LifeCnt++)
      {   
        pClientLife = pClientVerwaltungStart + LifeCnt;
        
        if (Diagnose.LogPointerInfo == TRUE)
        {
          ValidatePointerClient((UDINT) pClientLife, 40, (MemManagerDiag_type* ) &sMemDiag, 1);
        }
        
        if ((pClientLife->IPAdresse > 0) && /* IP Adresse konfiguriert ?*/
          (pClientLife->Timeout > 0) &&   /* Timeout vorhanden ?*/
          (pClientLife->LifeCheck == STATE_LIFE_OK)) /* Client noch am Leben */
        {
          /* ------------------------ Timer ist abgelaufen ------------------------------------------- */
          if ((pClientLife->LifeCheckTime + (pClientLife->Timeout / 10)) <= udTimeTick)
          { 
            pClientLife->LifeCheckTrial ++;
            if (pClientLife->LifeCheckTrial <= TIMEOUT_LIFE_RETRY)
            {
              /* mit Systemtick abgleichen */
              pClientLife->LifeCheckTime = udTimeTick;
  
              /* Telegramm generieren und absenden */
              brsmemcpy((UDINT) &LifeCheckRequest.Kennung, (UDINT) "LIFE", 4);
              brsmemcpy((UDINT) &LifeCheckRequestSave, (UDINT) &LifeCheckRequest, sizeof(LifeCheckRequest_typ));
              
              CopyLifeCheckRequestToSendBuffer();
              
              UDPsendLifeFub.enable = TRUE;
              UDPsendLifeFub.ident  = UDPopenFub[4].ident;
              UDPsendLifeFub.pHost  = (UDINT) &pClientLife->IpAdr;
              UDPsendLifeFub.pData  = (UDINT)&arSendBufLife;
              UDPsendLifeFub.datalen  = sizeof(arSendBufLife);  
              UDPsendLifeFub.port   = PortTarget[5];
              UDPsendLifeFub.flags  = udpSendFlags;
              UdpSend(&UDPsendLifeFub);
              
              LifeCheck.Server = LifeCnt + 1; /* akt. Server + 1  für nächsten Lifecheck abspeichern */
              LifeCntAct = LifeCnt;	/* aktuell bearbeiteter Server */
              
              LifeCheck.AufrufeFub = 0;
              LifeCheck.TelegrammeCnt++;
              Diagnose.EventCnt.SendLifeCheckCount ++;
              
              StepLifeCheck.Step.StepNr = LIFE_CHECK_SEND_OK;
              break;
            }
            else
            {
              pClientLife->LifeCheck = STATE_LIFE_ERR;
              pClientLife->LifeCheckTrial = 0;
              ErsatzwerteSetzen ((UDINT*) pVariablenVerwaltungStart, LifeCnt);              
              Lebensueberwachung[LifeCnt] = TRUE;
              Diagnose.EventCnt.LostLifeCheckCount ++;  
              
              //WriteLoggerMessage((Logger_typ*) &sLogger, 0xF000, "Ersatzwerte setzen", 0x7FFFFFFF, PTOP_LOG_SEVERITY_INFO);
              CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Lifecheck, &CmdBurLogBook, "LifeCheck", "Ersatzwerte setzen (2)", (char*) &pClientLife->IpAdr, 0x7FFFFFFF, StepLifeCheck.Step.StepNr, udTimeTick);
              CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
            }
          }
        }
      }
      
      if (LifeCnt + 1 > MAX_SERVER)
      {
        LifeCheck.Server = 0;
      }
      break;
    
    /*********************************************************************************************************/
    case LIFE_CHECK_SEND_OK:
      brsstrcpy((UDINT) &StepLifeCheck.Step.StepText, (UDINT) "LIFE_CHECK_SEND_OK");	// stephandling: copy actual step
      WriteStepDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Lifecheck, &CmdBurLogBook, "LifeCheck", "LIFE_CHECK_SEND_OK", StepLifeCheck.Step.StepNr, udTimeTick);
			
      if (UDPsendLifeFub.status != ERR_FUB_BUSY)
      {
        if (UDPsendLifeFub.status == FUB_OK)
        {
          CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Lifecheck, &CmdBurLogBook, "LifeCheck", "LIFE_CHECK_SEND_OK", "UDPsendLifeFub.status", UDPsendLifeFub.status, StepLifeCheck.Step.StepNr, udTimeTick);
          CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
          
          Diagnose.EventCnt.LifeCheckSend++;
        }
        else 
        {
          /* Telegramm konnte nicht gesendet werden -> Eintrag in Logbuch u. weiter zum nächsten Client -> Client   */
          /* wird nicht aus ausgefallen gekennzeichnet !!!    */      
          CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Lifecheck, &CmdBurLogBook, "LifeCheck", "LIFE_CHECK_SEND_OK", "UDPsendLifeFub.status", UDPsendLifeFub.status, StepLifeCheck.Step.StepNr, udTimeTick);
          CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
          
          Diagnose.EventCnt.LifeCheckSendFailure++;
        }
        StepLifeCheck.Step.StepNr = LIFE_CHECK_SEND; /* auf jedem Fall zur nächsten Station weiter */
      } 
      else 
      {
        UdpSend(&UDPsendLifeFub);
      }
      break;
    
  } /* switch (StepLifeCheck.Step.StepNr) */


  /* ---- Empfang eines Life - Responsetelegramme -------------------------------------------------- */
  pClientLife = NULL;
      
  if (UDPreceiveLifeFub.enable)
  {
    /* FUB ist initialisiert */
    UdpRecv(&UDPreceiveLifeFub);
    if (UDPreceiveLifeFub.status == FUB_OK)
    {
      if ((arRecvBufLife[0] == 'L') &&
          (arRecvBufLife[1] == 'I') &&
          (arRecvBufLife[2] == 'F') &&
          (arRecvBufLife[3] == 'E'))
      {
        CopyReceiveBufferToLifeCheckResponse();

        Diagnose.EventCnt.ReceiveLifeCheckCount++;
       
        CRCCheckTemp = CRCcheck((UDINT*) &arRecvBufLife, 6);
        
        if (CRCCheckTemp == LifeCheckResponse.BCC)
        {         
          /* --- Pointer und Index von Server holen */
          pClientLife = (Client_List_Typ*)(GetServerByIpStr((UDINT*) UDPreceiveLifeFub.pIpAddr, &serverIndex));
         
          if (Diagnose.LogPointerInfo == TRUE)
          {
            ValidatePointerClient((UDINT) pClientLife, 41, (MemManagerDiag_type* ) &sMemDiag, 1);
          }
          
          if (pClientLife != NULL)
          {
            /* --- Client auf Server noch konfiguriert? ---------------------- */
            if (LifeCheckResponse.ClientStatus == STATE_LIFE_OK)
            {
              // Verbindung bereits auf aus? ein erneutes aktivieren ist nicht erlaubt              
              if (pClientLife->LifeCheck == STATE_LIFE_OK)
              {
                pClientLife->LifeCheck = STATE_LIFE_OK;     /* Verbindung vorhanden */
                pClientLife->LifeCheckTime = udTimeTick;    /* Timer neu aufziehen  */
                pClientLife->LifeCheckTrial = 0;
                Lebensueberwachung[serverIndex] = FALSE;
              }
              else
              {
                pClientLife->LifeCheck = STATE_LIFE_ERR;  /* Verbindung nicht mehr vorhanden */
                pClientLife->LifeCheckTrial = 0;
                Lebensueberwachung[serverIndex] = TRUE;
                ResetIpAdrForServer(pClientVerwaltungStart, serverIndex); /* Ip- Adresse bei Timeout verwerfen*/
                
                /* --- Client wird beim Server gerade angemeldet ---------------- */
                CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Lifecheck, &CmdBurLogBook, "LifeCheck", "Client nicht (mehr) aktiv: ", (char*) &pClientLife->IpAdr, 0x7FFFFFFF, StepLifeCheck.Step.StepNr, udTimeTick);
                CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
              }
            }
            else if (LifeCheckResponse.ClientStatus == STATE_LIFE_CLIENT_MISSING)
            {
              pClientLife->LifeCheck = STATE_LIFE_ERR;  /* Verbindung nicht mehr vorhanden */
              pClientLife->LifeCheckTrial = 0;
              ErsatzwerteSetzen ((UDINT*) pVariablenVerwaltungStart, serverIndex);              
              Lebensueberwachung[serverIndex] = TRUE;
              ResetIpAdrForServer(pClientVerwaltungStart, serverIndex); /* Ip- Adresse bei Timeout verwerfen*/
              
              CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Lifecheck, &CmdBurLogBook, "LifeCheck", "Ersatzwerte setzen (1)", (char*) &pClientLife->IpAdr, 0x7FFFFFFF, StepLifeCheck.Step.StepNr, udTimeTick);
              CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
            }
            else if (LifeCheckResponse.ClientStatus == STATE_LIFE_CONN)
            {
              /* --- Client wird beim Server gerade angemeldet ---------------- */
              CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Lifecheck, &CmdBurLogBook, "LifeCheck", "Client in Registrierungsphase", (char*) &pClientLife->IpAdr, 0x7FFFFFFF, StepLifeCheck.Step.StepNr, udTimeTick);
              CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
            }
          }
          else
          {
            /* --- Server-Ip-Adresse unbekannt ----------------------------------- */
            CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Lifecheck, &CmdBurLogBook, "LifeCheck", "Pointer NULL", "undef", 0x7FFFFFFF, StepLifeCheck.Step.StepNr, udTimeTick);
            CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
          }
        } /* (CRCCheckTemp == LifeCheckResponse.BCC) */
        else
        {
          /* Checksumme nicht ok */
          CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Lifecheck, &CmdBurLogBook, "LifeCheck", "CRC failed ", (char*) &pClientLife->IpAdr, CRCCheckTemp, StepLifeCheck.Step.StepNr, udTimeTick);
          CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
        }
      }
      else
      {
        /* Falsche Telegramm-Kennung */
        CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Lifecheck, &CmdBurLogBook, "LifeCheck", "Telegram Header wrong", (char*) &pClientLife->IpAdr, 0x7FFFFFFF, StepLifeCheck.Step.StepNr, udTimeTick);
        CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
      }
    }
    else
    {
      if ((UDPreceiveLifeFub.status != FUB_BUSY) && (UDPreceiveLifeFub.status != udpERR_NO_DATA))
      {             
        CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Lifecheck, &CmdBurLogBook, "LifeCheck", "UDPreceiveLifeFub.status", (char*) &pClientLife->IpAdr, UDPreceiveLifeFub.status, StepLifeCheck.Step.StepNr, udTimeTick);
        CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
      }
    }
  }
}


/******************************************************************************************************************
*   Überwachung der Usereingaben für einen Reconnect
******************************************************************************************************************/

void CyclicTriggerReconnect()
{
  Client_List_Typ* pServerAct;
  UINT uiReconnect;
  
  // Anforderungen überarbeiten
  for (uiReconnect = uiSaveServerReconn; uiReconnect < MAX_SERVER; uiReconnect++)
  {
    // aktuellen Status überprüfen: Sonderfall#1
    // keine Variablen angemeldet aber Lifecheck läuft? = Reset

    pServerAct = pClientVerwaltungStart + uiReconnect;
//    if ((sServerConn[uiReconnect].uiPvConf != 0) && (sServerConn[uiReconnect].uiPvConfFound != 0) &&
//        (sServerConn[uiReconnect].uiPvConnToServ == 0) && (sServerConn[uiReconnect].uiPvLinked == 0) && (pServerAct->LifeCheck == STATE_LIFE_OK))
//    {
//      //NeuAnmeldungFehler[uiReconnect] = TRUE;
//      #warning kommentar wieder weg!
//    }

    // Neuanmeldung überprüfen und durchziehen
    if ((NeuAnmeldungHand[uiReconnect] == TRUE) ||
      (NeuAnmeldungFehler[uiReconnect] == TRUE) ||
      (sServerConn[uiReconnect].bCmdReConn == TRUE))
    {
      ResetIpAdrForServer(pClientVerwaltungStart, uiReconnect);
      pServerAct = pClientVerwaltungStart + uiReconnect;
	  
      if (NeuAnmeldungHand[uiReconnect] == TRUE)
      {
        Diagnose.EventCnt.TriggerReconnManual++;
      }
	  
      if (NeuAnmeldungFehler[uiReconnect] == TRUE)
      {
        Diagnose.EventCnt.TriggerReconnErr++;
      }
	  
      if (sServerConn[uiReconnect].bCmdReConn == TRUE)
      {
        Diagnose.EventCnt.TriggerReconnExtern++;
      }

      /* wenn richtig konfiguriert, dann wird eine Anfrage zur Station generiert -> Neuanmeldung sobald möglich */
      if ((pServerAct->ConfByHost == 1) && (pServerAct->HostName[0] != 0))
      {
        uiSaveServerReconn = uiReconnect;
        bRequestConn[uiReconnect] = TRUE;
	    
        NeuAnmeldungHand[uiReconnect] = FALSE;
        NeuAnmeldungFehler[uiReconnect] = FALSE;
        sServerConn[uiReconnect].bCmdReConn = FALSE;
        
        pServerAct->LifeCheck = STATE_LIFE_ERR;
      }
      else if ((pServerAct->ConfByHost == 0) && (pServerAct->IpAdr[0] != 0))
      {
        uiSaveServerReconn = uiReconnect;
        bRequestConn[uiReconnect] = TRUE;
	    
        NeuAnmeldungHand[uiReconnect] = FALSE;
        NeuAnmeldungFehler[uiReconnect] = FALSE;
        sServerConn[uiReconnect].bCmdReConn = FALSE;
        
        pServerAct->LifeCheck = STATE_LIFE_ERR;
      }
      else
      {
        /* Station nicht richtig konfiguriert: Anfragen zurücksetzen */
        NeuAnmeldungHand[uiReconnect] = FALSE;
        NeuAnmeldungFehler[uiReconnect] = FALSE;
        sServerConn[uiReconnect].bCmdReConn = FALSE;
      }
      break;
    }  
  }
  
  if (uiReconnect >= MAX_SERVER)
  {
    uiSaveServerReconn = 0;
  }
  
}


/******************************************************************************************************************
*   Ermittlung von Ip Adressen - nur INTEL(ab V2.67)
******************************************************************************************************************/

void CyclicSLSGetIpAdr()
{

#if INTEL

  // Stephandler: overhead /initialisierung
  if (StepGetIpAdr.StepHandling.Current.bTimeoutElapsed == 1)
  {
    StepGetIpAdr.StepHandling.Current.bTimeoutElapsed = 0;
    StepGetIpAdr.Step.StepNr = StepGetIpAdr.StepHandling.Current.nTimeoutContinueStep;
  }
  StepGetIpAdr.StepHandling.Current.nStepNr = (DINT) StepGetIpAdr.Step.StepNr;
  brsstrcpy((UDINT) &StepGetIpAdr.StepHandling.Current.sStepText, (UDINT) &StepGetIpAdr.Step.StepText);
  BrbStepHandler(&StepGetIpAdr.StepHandling);
  
  switch (StepGetIpAdr.Step.StepNr)
  {
    case GET_HOST_INIT:
      brsstrcpy((UDINT) &StepGetIpAdr.Step.StepText, (UDINT) "GET_HOST_INIT");	// stephandling: copy actual step
      WriteStepDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.HostByName, &CmdBurLogBook, "GetIpAdr", "GET_HOST_INIT", StepGetIpAdr.Step.StepNr, udTimeTick);
			
      StepGetIpAdr.Step.InitDone = TRUE;
      StepGetIpAdr.Step.StepNr = GET_HOST;
      break;
    
    case GET_HOST:
      brsstrcpy((UDINT) &StepGetIpAdr.Step.StepText, (UDINT) "GET_HOST");	// stephandling: copy actual step
      WriteStepDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.HostByName, &CmdBurLogBook, "GetIpAdr", "GET_HOST", StepGetIpAdr.Step.StepNr, udTimeTick);
			
      if (uiGetIpServer + 1 < MAX_SERVER) 
      {       
        pClientGetIpAdr = pClientVerwaltungStart + uiGetIpServer;
        if (pClientGetIpAdr->ConfByHost == 1)
        {
          /* --- Name vorhanden und noch keine Ip - Adresse -------------------------- */
          if (((pClientGetIpAdr->HostName[0] != 0) &&  /* Hostname vorhanden*/                  
            (pClientGetIpAdr->IPAdresse == 0)) ||      /* keine Hex - Ip - Adresse; String wird nicht mehr abgefragt! */
            (pClientGetIpAdr->IpHostTimeout + TIMEOUT_HOST_CHECK_IP <= udTimeTick)) /* ODER Zeit abgelaufen */
          {
            sHostByName.enable = TRUE;
            sHostByName.pName  = (UDINT) &pClientGetIpAdr->HostName;
            HostByName(&sHostByName);
            
            if (sHostByName.status != FUB_BUSY)
            {
              if (sHostByName.status == FUB_OK)
              {
                /* --- Ip Adresse hat sich geändert, von gültiger Ip Adresse ausgehend ? */
                /* --- Station wird verworfen, komplette Neuanmeldung anstossen */
                if ((pClientGetIpAdr->IPAdresse != 0) && (pClientGetIpAdr->IPAdresse != sHostByName.address))
                {
                  Diagnose.EventCnt.GetHostNameIpChanged++;
                  
                  CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.HostByName, &CmdBurLogBook, "HostByName", "Ip Changed for: ", (char*) &pClientGetIpAdr->HostName, 1, 999, udTimeTick);
                  CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;  
                  
                  /* --- Station als ungültig erklären */
                  NeuAnmeldungFehler[uiGetIpServer] = TRUE;
                }
                else
                {
                  Diagnose.EventCnt.GetIpAdr++;
                  pClientGetIpAdr->IPAdresse = sHostByName.address;
                
                  CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.HostByName, &CmdBurLogBook, "HostByName", "New Ip Adress for: ", (char*) &pClientGetIpAdr->HostName, 0, 999, udTimeTick);
                  CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;  
                  
                  /* --- Umwandlung der Adresse in einen String und ablegen in interner Verwaltung */
                  DIAG_lenConvertIpAdr = ConvertIpAdrToAscii((UDINT*) pClientGetIpAdr);
                  sServerConn[uiGetIpServer].Alarm.AlarmList.HostnameUnresolved = FALSE;
                }
              }
              else
              {
                CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.HostByName, &CmdBurLogBook, "HostByName", "Error: ", (char*) &pClientGetIpAdr->HostName, sHostByName.status, 999, udTimeTick);
                CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;  
                
                if (sHostByName.status == ERR_ASHOST_RESOLV_NAME)
                {
                  /* --- Der Name konnte nicht aufgelöst werden ... */
                  sServerConn[uiGetIpServer].Alarm.AlarmActive = TRUE;
                  sServerConn[uiGetIpServer].Alarm.AlarmList.HostnameUnresolved = TRUE;
                }
                Diagnose.EventCnt.GetHostNameErr++;
              }
              pClientGetIpAdr->IpHostTimeout = udTimeTick;		/* Zeit nach Hostnamen - Aktualisierung auf Stand bringen, egal ob erfolgreich oder nicht! */
              uiGetIpServer++;
            }
            else
            {
              /* --- zum nächsten Schritt weiter ------------------ */
              StepGetIpAdr.Step.StepNr = GET_HOST_FUB;
            }
          }
        }
        /* inkrementieren, falls man nicht zum nächsten Schritt geht ..*/
        if (StepGetIpAdr.Step.StepNr != GET_HOST_FUB)
        {
          uiGetIpServer++;
        }
      }
      else
      {
        /* --- wieder von vorne beginnen, Station kann zu diesem Zeitpunkt nicht vorhanden sein */
        uiGetIpServer  = 0;
      }     
      break;
  
  
    case GET_HOST_FUB:
      brsstrcpy((UDINT) &StepGetIpAdr.Step.StepText, (UDINT) "GET_HOST_FUB");	// stephandling: copy actual step
      WriteStepDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.HostByName, &CmdBurLogBook, "GetIpAdr", "GET_HOST_FUB", StepGetIpAdr.Step.StepNr, udTimeTick);
			
      /* --- Aufruf der Funktion, bis Ergebnis  ----------------------------------------- */
      HostByName(&sHostByName);
      if (sHostByName.status != FUB_BUSY)
      {
        if (sHostByName.status == FUB_OK)
        {
          Diagnose.EventCnt.GetIpAdr++;
          
          /* --- Ip Adresse hat sich geändert, von gültiger Ip Adresse ausgehend ? */
          if ((pClientGetIpAdr->IPAdresse != 0 ) && (pClientGetIpAdr->IPAdresse != sHostByName.address))
          {
            Diagnose.EventCnt.GetHostNameIpChanged++;

            CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.HostByName, &CmdBurLogBook, "HostByName", "Get ip adress ", (char*) &pClientGetIpAdr->HostName, sHostByName.status, StepGetIpAdr.Step.StepNr, udTimeTick);
            CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;  
            
            /* Station als ungültig erklären */
            NeuAnmeldungFehler[uiGetIpServer] = TRUE;
          }

          pClientGetIpAdr->IPAdresse = sHostByName.address;         
          
          /* --- Umwandlung der Adresse in einen String und ablegen in interner Verwaltung */
          DIAG_lenConvertIpAdr = ConvertIpAdrToAscii((UDINT*) pClientGetIpAdr);
        }
        else
        {  
          Diagnose.EventCnt.GetHostNameErr++; /* Error Counter */
        
          CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.HostByName, &CmdBurLogBook, "HostByName", "Error 2: ", (char*) &pClientGetIpAdr->HostName, sHostByName.status, StepGetIpAdr.Step.StepNr, udTimeTick);
          CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;  
        }
        pClientGetIpAdr->IpHostTimeout = udTimeTick;		/* Zeit nach Hostnamen - Aktualisierung auf Stand bringen, egal ob erfolgreich oder nicht! */
        uiGetIpServer++;
        StepGetIpAdr.Step.StepNr = GET_HOST;
      }

      break;
  }
#endif
}


/************************************************************************************************************
* --- ClientMonitor
************************************************************************************************************/

void CyclicClientMonitor()
{
  
  if (Monitor.Lock == TRUE)
  {
    if (Monitor.PVIndex < MAX_VARIABLEN)
    {
      brsmemcpy((UDINT)&PvMonitor, (UDINT)(pVariablenVerwaltungStart + Monitor.PVIndex), sizeof(PvList_Typ));
    }
    
    if (Monitor.ClientIndex < MAX_SERVER)
    {
      brsmemcpy((UDINT) &ClientMonitor, (UDINT)(pClientVerwaltungStart + Monitor.ClientIndex), sizeof(Client_List_Typ));

#if INTEL     
      /* --- IP-Adresse richtig drehen für Kontrolle ---------------------------- */
      brsmemcpy((UDINT) &usIpAdrMonitor, (UDINT) &ClientMonitor.IPAdresse, sizeof(usIpAdrMonitor));
      ClientMonitor.IPAdresse = usIpAdrMonitor[0];
      ClientMonitor.IPAdresse = (ClientMonitor.IPAdresse << 8) + usIpAdrMonitor[1];
      ClientMonitor.IPAdresse = (ClientMonitor.IPAdresse << 8) + usIpAdrMonitor[2];
      ClientMonitor.IPAdresse = (ClientMonitor.IPAdresse << 8) + usIpAdrMonitor[3];
#endif            
    }
  }
}


/*****************************************************************************************************************/
/*                        CRC Check                              */
/*****************************************************************************************************************/

UINT CRCcheck (
  UDINT* pTelegrammStartAdresse, 
  UINT Telegrammlaenge)
{
  USINT data = 0;
  USINT *sData = NULL;
  UINT  bcs = 0;
  UINT    uiLoopLen = 0;
  UINT  uiLoopByte = 0; 
  
  sData = (USINT*) pTelegrammStartAdresse;
  
  for (uiLoopLen = 0; uiLoopLen < Telegrammlaenge; uiLoopLen++)
    {
    data = sData[uiLoopLen];
    for (uiLoopByte = 0; uiLoopByte < 8; uiLoopByte++)
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


/*****************************************************************************************************************/
/*                    Vorbelegung der Variablen (TEST!)                      */
/*****************************************************************************************************************/

void Vorbelegung (UDINT* StartAdressePV, UDINT* StartAdresseServer)
{

  PvList_Typ      *pVariablenbelegungStart, *pVariablenbelegung;
  Client_List_Typ   *pClientbelegungStart, *pClientbelegung;

  UINT  uiLoopTest;
  USINT HelpString1[PV_LENGTH_ARR];
  USINT HelpString2[5];
  USINT HelpString3[25];
  
  pVariablenbelegungStart = (PvList_Typ*) StartAdressePV;
  pClientbelegungStart = (Client_List_Typ*) StartAdresseServer;
  
  for (uiLoopTest = 0; uiLoopTest < 100; uiLoopTest++)
    {

    /* Variablenname Client*/
    pVariablenbelegung = pVariablenbelegungStart + uiLoopTest;
    
    brsstrcpy((UDINT) &HelpString1, (UDINT) "test:Variable[");
    brsitoa((DINT) &uiLoopTest, (UDINT) &HelpString2);
    brsstrcat((UDINT) &HelpString1, (UDINT) &HelpString2);
    
    brsstrcpy((UDINT) &HelpString3, (UDINT) "]");
    brsstrcat((UDINT) &HelpString1, (UDINT) &HelpString3);
        
    brsmemcpy((UDINT) &pVariablenbelegung->NameClient, (UDINT) &HelpString1[0],sizeof(HelpString1));
    
    /* Variablenname auf Server*/
    brsstrcpy ((UDINT) &HelpString1, (UDINT) "server:PVServ[");
    brsitoa((DINT) &uiLoopTest, (UDINT) &HelpString2);
    brsstrcat((UDINT) &HelpString1, (UDINT) &HelpString2);
    
    brsstrcpy ((UDINT) &HelpString3, (UDINT) "]");
    brsstrcat((UDINT) &HelpString1, (UDINT) &HelpString3);
        
    brsmemcpy((UDINT) &pVariablenbelegung->NameServer, (UDINT) &HelpString1[0],sizeof(HelpString1));
  
    /*  Ersatzwert  */
    pVariablenbelegung->Ersatzwert  = uiLoopTest;
    
    pVariablenbelegung->Remanent  = FALSE;
    pVariablenbelegung->Init    = uiLoopTest + 2; 
    pVariablenbelegung->Hysterese = 10;
    pVariablenbelegung->Client    = 0;
  
  }
  
  /* --- Die Hälfte der Variablen wird nun von einem 2ten Server gesendet: */
  for (uiLoopTest = 50; uiLoopTest < 100; uiLoopTest++)
    {
    pVariablenbelegung = pVariablenbelegungStart + uiLoopTest;
    pVariablenbelegung->Client  = uiLoopTest;
  }

  pClientbelegung         =   pClientbelegungStart;
  pClientbelegung->Timeout    =   3000; /*  in ms*/
  pClientbelegung->IPAdresse    =   0x0A310AEC;
  
  pClientbelegung         =   pClientbelegungStart + 1;
  pClientbelegung->Timeout    =   3000; /*  in ms*/
  pClientbelegung->IPAdresse    =   0x0A310AED; /*  in ms*/
}


/*****************************************************************************************************************/
/*                    Bestimmung der Variablenpointer                        */
/*****************************************************************************************************************/
void GetPointerOfPV(
  UDINT* pStartAdresse, 
  UINT AnzahlVariablen)
{
  PvList_Typ    *pVariablenbelegungStart, *pVariablenbelegung;
  UINT  cnt;
  UINT  Status;
  USINT HelpString[PV_LENGTH_ARR];
  UDINT VarLen;
  UDINT VarAdresse;
  UDINT strlength;
  UDINT data_type;
  UINT Dimension;
  
  if (pStartAdresse == NULL)
  {
    return;
  }
    
  pVariablenbelegungStart = (PvList_Typ*) pStartAdresse;
  
  for (cnt = 0; cnt < AnzahlVariablen; cnt++)
  {
    /* Variablenname Client*/
    pVariablenbelegung = pVariablenbelegungStart + cnt;
    strlength = brsstrlen((UDINT) &pVariablenbelegung->NameClient);
    
    if  (strlength > 0)
    {
      brsmemset((UDINT) &HelpString, 0, sizeof(HelpString));
      brsmemcpy((UDINT) &HelpString, (UDINT) &pVariablenbelegung->NameClient, sizeof(pVariablenbelegung->NameClient));
      Status = PV_xgetadr((char*) &HelpString, &VarAdresse,(UDINT*) &VarLen);
          
      if (Status == FUB_OK)
      {
        pVariablenbelegung->pPv = VarAdresse;
        pVariablenbelegung->PvLaenge = VarLen;
        
        Status = PV_ninfo((char*) &HelpString, (UDINT*) &data_type, &VarLen, &Dimension);
        
        if (Status == FUB_OK)
        {
          pVariablenbelegung->DataType = data_type;
        }
      }       
      else
      {
        pVariablenbelegung->pPv = 0;
        pVariablenbelegung->Status = Status;
        pVariablenbelegung->DataType = 0;
      }
    }
  }
}


/*****************************************************************************************************************/
/*              Setzen der Ersatzwert bei Timeout für einen x-beliebigen Server            */
/*****************************************************************************************************************/

void ErsatzwerteSetzen(
  UDINT *pStartAdresse,   /* Anfangsadresse der Variablenverwaltung   */
  UINT Server)      /* Servernr.                */
{
  PvList_Typ   *pVariablenbelegungStart, *pVariablenbelegung;
  UINT  cnt = 0;

  USINT*  pusPvVal = NULL;
  UINT* puiPvVal = NULL;
  UDINT*  pudPvVal = NULL;

  if (pStartAdresse == NULL)
  {
    return;
  }

  pVariablenbelegungStart = (PvList_Typ*) pStartAdresse;
  pVariablenbelegung = pVariablenbelegungStart;
  if (pVariablenbelegungStart != NULL)
  {
    for (cnt = 0; cnt < MAX_VARIABLEN; cnt++)
      {
      pVariablenbelegung = pVariablenbelegungStart + cnt;
      if (pVariablenbelegung->pPv != NULL)
      {
        if (Server == pVariablenbelegung->Client)
        {
          if (pVariablenbelegung->Ersatzwert != GRENZWERT_NO)
          {
            switch (pVariablenbelegung->PvLaenge)
            {
              case 1:
                pusPvVal = (USINT*)pVariablenbelegung->pPv;
                *pusPvVal = pVariablenbelegung->Ersatzwert;
                break;
              
              case 2:
                puiPvVal = (UINT*)pVariablenbelegung->pPv;
                *puiPvVal = pVariablenbelegung->Ersatzwert;
                break;
              
              case 4:
                pudPvVal = (UDINT*)pVariablenbelegung->pPv;
                *pudPvVal = pVariablenbelegung->Ersatzwert;
                break;
            
            }
          }
          pVariablenbelegung->LetzterWert = pVariablenbelegung->Ersatzwert;
          pVariablenbelegung->LetzterWertCnt++;
        }
      }
    }
  }

}


/*****************************************************************************************************************/
/*                      Suche nach Servern + Variablen                     */
/*****************************************************************************************************************/

void ParserLogbuch (
  UDINT *pStartAdresse)
{
  PvList_Typ    *pVariablenbelegungStart, *pVariablenbelegung;
  UINT  cnt;
  UINT  VariablenAnzahl;
  UINT  ServerAnzahl;
  UINT  LastServer;
  USINT HelpString1[32];
  USINT HelpString2[32];

  if (pStartAdresse == NULL)
  {
    return;
  }

  pVariablenbelegungStart = (PvList_Typ*) pStartAdresse;
  pVariablenbelegung = pVariablenbelegungStart;
  VariablenAnzahl = 0;
  ServerAnzahl = 0;
  LastServer = MAX_SERVER;
  
  for (cnt = 0; cnt < MAX_VARIABLEN; cnt++)
    {
    pVariablenbelegung = pVariablenbelegungStart + cnt;
    
    if (pVariablenbelegung->pPv != NULL)
    {
      VariablenAnzahl++;
      if (pVariablenbelegung->Client != LastServer)
      {
        ServerAnzahl++;
        LastServer = pVariablenbelegung->Client;
      }
    } /* if (pVariablenbelegung->pPv > 0) */
  } /* for (cnt = 0; cnt < MAX_VARIABLEN; cnt++) */
  
  brsmemset((UDINT) &HelpString1, 0, sizeof(HelpString1));
  brsmemset((UDINT) &HelpString2, 0, sizeof(HelpString2));

  /********************************************************************************/
  /* Eintrag der komplett konfigurierten Variablen auf System */  
  brsstrcpy((UDINT) &HelpString1, (UDINT) "client:ConfigPv=");
  brsitoa((DINT) AktVar, (UDINT) &HelpString2);
  brsstrcat((UDINT) &HelpString1, (UDINT) &HelpString2);  
  WriteLoggerMessage((Logger_typ*) &sLogger, 0xF000, (STRING*) &HelpString1, 0x7FFFFFFF, arEVENTLOG_SEVERITY_INFO);
  
  /* Eintrag der ermittelten Variablen in das Logbuch */
  brsstrcpy((UDINT) &HelpString1, (UDINT) "client:RpsPv=");
  brsitoa((DINT) VariablenAnzahl, (UDINT) &HelpString2);
  brsstrcat((UDINT) &HelpString1, (UDINT) &HelpString2);
  WriteLoggerMessage((Logger_typ*) &sLogger, 0xF000, (STRING*) &HelpString1, 0x7FFFFFFF, arEVENTLOG_SEVERITY_INFO);
    
  /*Eintrag der ermittelten Server in das Logbuch */
  brsstrcpy((UDINT) &HelpString1, (UDINT) "client:Server=");
  brsitoa((DINT) ServerAnzahl, (UDINT) &HelpString2);
  brsstrcat((UDINT) &HelpString1, (UDINT) &HelpString2);  
  WriteLoggerMessage((Logger_typ*) &sLogger, 0xF000, (STRING*) &HelpString1, 0x7FFFFFFF, arEVENTLOG_SEVERITY_INFO);  
}


/*****************************************************************************************************************
* Taskende
*****************************************************************************************************************/

_EXIT void taskExit(void)
{

  /* --- Bei Beenden des Task werden die UDP - Ports wieder geschlossen */
  for (uiLoopClosePort = 0; uiLoopClosePort < 6; uiLoopClosePort++)
    {
    UDPcloseFub[uiLoopClosePort].ident  = UDPopenFub[uiLoopClosePort].ident;
    UDPcloseFub[uiLoopClosePort].enable = TRUE;
    UdpClose(&UDPcloseFub[uiLoopClosePort]);
  }
  
  WriteLoggerMessage((Logger_typ*) &sLogger, 0xF000, (STRING*) "client:exit", 0x7FFFFFFF, arEVENTLOG_SEVERITY_INFO);
}


/*****************************************************************************************************************/
/* StartAdresse der Client bzw. Variablenverwaltung                                */
/* Index: PV bzw. Clientindex                                            */
/* LogbuchCnt: laufender Index                                           */
/* TypErr: 1 -> Client; 2 -> PV                                          */
/* Statusmeldung                                                 */
/*****************************************************************************************************************/

void LogbuchEintrag(
  UDINT* pStartAdresse,   /* Startadresse der Fehlereinträge      */
  UINT ClientIndex,     /* Index des Clients in Clientverwaltung  */
  USINT LogbuchCnt,     /* laufende Nummer für Logbucheinträge    */
  USINT TypErr)     /* Fehlertyp                */
{
  PvList_Typ    *pVariablenStart;
  PvList_Typ    *pVariablen;
  Client_List_Typ *pClientStart;
  Client_List_Typ *pClient;
  USINT LogString1[32];
  USINT LogString2[32];
  USINT LogString3[32];
  USINT LogString4[32];
  
  if (pStartAdresse == NULL)
  {
    return;
  }
  
  pClient = NULL;
  pClientStart = NULL;
  pVariablen = NULL;
  pVariablenStart = NULL;
  
  UNUSED(pClient);        // for the compiler
  UNUSED(pVariablen);     // for the compiler
  
  brsmemset((UDINT) &LogString1, 0, sizeof(LogString1));
  brsmemset((UDINT) &LogString2, 0, sizeof(LogString2));
  brsmemset((UDINT) &LogString3, 0, sizeof(LogString3));
  brsmemset((UDINT) &LogString4, 0, sizeof(LogString4));
  
  if (TypErr == 1)
  {
    pClientStart = (Client_List_Typ*) pStartAdresse;
    pClient = pClientStart + ClientIndex;
    
    brsstrcpy((UDINT) &LogString1, (UDINT) "client:Conn #");
    brsitoa( (DINT)LogbuchCnt, (UDINT) &LogString2);
    brsstrcat((UDINT) &LogString1, (UDINT) &LogString2);
    
    brsstrcpy((UDINT) &LogString4, (UDINT) ": Client ");
    brsstrcat((UDINT) &LogString1, (UDINT) &LogString4);
      
    brsitoa((DINT) ClientIndex, (UDINT) &LogString3); 
    brsstrcat((UDINT) &LogString1, (UDINT) &LogString3);
    
    WriteLoggerMessage((Logger_typ*) &sLogger, 0xF000, (STRING*) &LogString1, 0x7FFFFFFF, arEVENTLOG_SEVERITY_INFO);
  }
  else if (TypErr == 2)
  {
    pVariablenStart = (PvList_Typ*) pStartAdresse;
    pVariablen = pVariablenStart + ClientIndex;
    
    brsstrcpy((UDINT) &LogString1, (UDINT) "client:PV #");
    brsitoa(((UDINT) &LogbuchCnt), (UDINT) &LogString2);
    brsstrcat((UDINT) &LogString1, (UDINT) &LogString2);

    brsstrcpy((UDINT) &LogString4, (UDINT) ": ");
    brsstrcat((UDINT) &LogString1, (UDINT) &LogString4);
    
    brsitoa((DINT) ClientIndex, (UDINT) &LogString3); 
    brsstrcat((UDINT) &LogString1, (UDINT) &LogString3);
    WriteLoggerMessage((Logger_typ*) &sLogger, 0xF000, (STRING*) &LogString1, 0x7FFFFFFF, arEVENTLOG_SEVERITY_INFO);
  } 

}


/*****************************************************************************************************************/
/*              Swap-Funktionen für Intel                              */
/*****************************************************************************************************************/

UDINT SwapUdintForIntel(
  UDINT value)
{
#if INTEL
  return swapUDINT(value);
#else
  return value;
#endif
}

UINT SwapUintForIntel(
  UINT value)
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
UDINT TimerFunction (
  UDINT *AdrTickCount)
{

#if INTEL
  SysInfo_typ SysInfoFub;
    
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
/*              timer auslesen                                       */
/*****************************************************************************************************************/

void GetClock(
  UDINT *pudActClock)
{
  *pudActClock = (UDINT) TIM_musec();
}


/*****************************************************************************************************************/
/*              Ip-Adresse auslesen - für zyklische Abarbeitung !                  */
/*****************************************************************************************************************/

void CyclicGetOwnIpAdr(
  UDINT *pIpAdr)
{
  UDINT retIpAdr = 0;
  UDINT IpAddress = 0;

  if (sIpAdrSource.bEnable == FALSE)
  {
    return;
  }
  
  switch (sIpAdrSource.usStep)
  {
    case 0:
      
      sIpAdrSource.CfgGetIpAddrFub.enable = TRUE;
      sIpAdrSource.CfgGetIpAddrFub.pDevice = (UDINT) "IF2";
      sIpAdrSource.CfgGetIpAddrFub.pIPAddr = (UDINT) &sIpAdrSource.arIpAddressString;
      sIpAdrSource.CfgGetIpAddrFub.Len = sizeof(sIpAdrSource.arIpAddressString);
      CfgGetIPAddr(&sIpAdrSource.CfgGetIpAddrFub);
      
      sIpAdrSource.usStep = 1;
      break;
    
    case 1:
      if (sIpAdrSource.CfgGetIpAddrFub.status != 0xFFFF)
      {
        if (sIpAdrSource.CfgGetIpAddrFub.status == 0)
        {
          ethInetAton((UDINT) &sIpAdrSource.arIpAddressString, (UDINT) &retIpAdr);
          IpAddress = SwapUdintForIntel(retIpAdr);
          //IpAddress = SwapUdintForIntel(inet_addr((UDINT)sIpAdrSource.arIpAddressString));
          sIpAdrSource.bEnable = FALSE;
          *pIpAdr = IpAddress;
                    
          /* --- Umwandlung der Adresse in einen String und ablegen in interner Verwaltung */
          ConvertIpAdrToAsciiX(IpAddress, (UDINT*) &arIpAdrSource);
        }
      }
      else
      {
        CfgGetIPAddr(&sIpAdrSource.CfgGetIpAddrFub);
      }
      break;
  }
  
}


/*****************************************************************************************************************/
/*              Ip-Adresse auslesen - Achtung: nur im INIT verwenden !                 */
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



/*****************************************************************************************************************/
/*              Ip-Adresse auslesen - Achtung: nur im INIT verwenden !                 */
/*****************************************************************************************************************/

UDINT GetOwnIpAddress()
{
#if INTEL
  CfgGetIPAddr_typ      CfgGetIpAddrFub;
  USINT           IpAddressString[16];
  UDINT           IpAddress;
  UDINT           retIpAdr;
  
  IpAddressString[0] = 0;
  retIpAdr = 0;
  CfgGetIpAddrFub.enable = 1;
  CfgGetIpAddrFub.pDevice = (UDINT) "IF2";
  CfgGetIpAddrFub.pIPAddr = (UDINT) &IpAddressString;
  /* interne Variablen auf 0 setzten, ansonsten wird Schrott zurückgeliefert !*/
  CfgGetIpAddrFub.i_state = 0;
  CfgGetIpAddrFub.i_result = 0;
  CfgGetIpAddrFub.i_tmp = 0;
  CfgGetIpAddrFub.Len = sizeof(IpAddressString);
  do
  {
    CfgGetIPAddr(&CfgGetIpAddrFub);
  }while (CfgGetIpAddrFub.status == 0xFFFF);

  if (CfgGetIpAddrFub.status == 0)
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
  return ETHNodeFub.pIpAddr;
#endif
}


/****************************************************************************************************************/
/*              Übertragen eines ClientConnectRequest-Telegramms                                                */
/*              aus der Struktur "ClientConnectRequest" in den Sendebuffer                                      */
/*              mit Eintragung der Checksum                                                                     */
/****************************************************************************************************************/

void CopyClientConnectRequestToSendBuffer(
  UDINT *pSendBuf,
  ClientConnectRequest_typ *pClientConnect)
{
  UINT  ClientIndex;              /* Byte 4/5 */
  UINT  Timeout;                /* Byte 6/7 */
  UINT  BCC;                  /* Byte 8/9 */
  UINT  BCCraw;
  USINT arTempBuf[OPEN_BUFFER_LEN];

  /* Sendebuffer löschen */
  brsmemset((UDINT) &arTempBuf[0], 0, sizeof(arTempBuf));
  /* Kennung */
  *(UDINT*)&arTempBuf[0] = *(UDINT*)&pClientConnect->Kennung;
  /* ClientIndex */
  ClientIndex = SwapUintForIntel(pClientConnect->ClientIndex);
  *(UINT*)&arTempBuf[4] = *(UINT*)&ClientIndex;
  /* Timeout */
  Timeout = SwapUintForIntel(pClientConnect->Timeout);
  *(UINT*)&arTempBuf[6] = *(UINT*)&Timeout;
  /* BCC */
  BCCraw = CRCcheck((UDINT*) &arTempBuf, 8);
  pClientConnect->BCC = BCCraw;
  BCC = SwapUintForIntel(BCCraw);
  *(UINT*)&arTempBuf[8] = *(UINT*)&BCC;
  
  brsmemcpy((UDINT) pSendBuf, (UDINT) &arTempBuf, sizeof(arTempBuf));
  
}


/****************************************************************************************************************/
/*              Übertragen eines ClientConnectResponse-Telegramms                   */
/*              aus dem Empfangsbuffer                                */
/*              in Struktur "ClientConnectResponse"                         */
/****************************************************************************************************************/
void CopyReceiveBufferToClientConnectResponse(
  UDINT* pRecvBuf)
{
  UINT  ClientIndex;  /* Byte 4/5 */
  UINT  Status;     /* Byte 6/7 */
  UINT  BCC;      /* Byte 8/9 */
  USINT arTempBuf[OPEN_BUFFER_LEN];
  
  brsmemcpy((UDINT) &arTempBuf, (UDINT) pRecvBuf, sizeof(arTempBuf)); 

  /* Kennung */
  *(UDINT*)&ClientConnectResponse.Kennung = *(UDINT*)&arTempBuf[0];
  /* ClientIndex */
  *(UINT*)&ClientIndex = *(UINT*)&arTempBuf[4];
  ClientConnectResponse.ClientIndex = SwapUintForIntel(ClientIndex);
  /* Status */
  *(UINT*)&Status = *(UINT*)&arTempBuf[6];
  ClientConnectResponse.Status = SwapUintForIntel(Status);
  /* BCC */
  *(UINT*)&BCC = *(UINT*)&arTempBuf[8];
  ClientConnectResponse.BCC = SwapUintForIntel(BCC);
  
}


/****************************************************************************************************************/
/*              Übertragen eines PVOpenRequest-Telegramms                       */
/*              aus der Struktur "PVOpenRequest" in den Sendebuffer                 */
/*              mit Eintragung der Checksum                             */
/****************************************************************************************************************/

void CopyPVOpenRequestToSendBuffer(
  UDINT *pSendBuf,
  PVOpenRequest_typ *pPVOpenRequest)
{
  UINT  ClientIndex;  /* Byte 4/5 */
  UINT  PVIndex;    /* Byte 56/57 */
  UINT  Laenge;     /* Byte 58/59 */
  UDINT Hysterese;    /* Byte 60-63 */
  UINT  SyncTime;   /* Byte 64/65 */
  UDINT Reserve[2];   /* Byte 66/73 */
  UINT  BCC;      /* Byte 74/75 */
  UINT  BCCraw;
  UINT    BufferIndex = 0;
  USINT arTempBuf[OPEN_BUFFER_LEN];
 
  /* --- Sendebuffer löschen */
  brsmemset((UDINT) &arTempBuf, 0, sizeof(arTempBuf));
  BufferIndex = 0;
  
  /* --- Kennung */
  *(UDINT*)&arTempBuf[BufferIndex] = *(UDINT*)&pPVOpenRequest->Kennung;
  BufferIndex += 4;
  
  /* ClientIndex */
  ClientIndex = SwapUintForIntel(pPVOpenRequest->ClientIndex);
  *(UINT*)&arTempBuf[BufferIndex] = *(UINT*)&ClientIndex;
  BufferIndex += 2;
  
  /* Variablenname */
  brsmemcpy ((UDINT) &arTempBuf[BufferIndex], (UDINT) &pPVOpenRequest->Variablenname, sizeof(pPVOpenRequest->Variablenname));
  BufferIndex += sizeof(PVOpenRequest.Variablenname);
  
  /* PVIndex */
  PVIndex = SwapUintForIntel(pPVOpenRequest->PVIndex);
  *(UINT*)&arTempBuf[BufferIndex] = *(UINT*)&PVIndex;
  BufferIndex += 2;
  
  /* Laenge */
  Laenge = SwapUintForIntel(pPVOpenRequest->Laenge);
  *(UINT*)&arTempBuf[BufferIndex] = *(UINT*)&Laenge;
  BufferIndex += 2;
  
  /* Hysterese */
  Hysterese = SwapUdintForIntel(pPVOpenRequest->Hysterese);
  *(UDINT*)&arTempBuf[BufferIndex] = *(UDINT*)&Hysterese;
  BufferIndex += 4;
  
  /* SyncTime */
  SyncTime = SwapUintForIntel(pPVOpenRequest->SyncTime);
  *(UINT*)&arTempBuf[BufferIndex] = *(UINT*)&SyncTime;
  BufferIndex += 2;
  
  /* Reserve[0] */
  Reserve[0] = SwapUdintForIntel(pPVOpenRequest->Reserve[0]);
  *(UDINT*)&arTempBuf[BufferIndex] = *(UDINT*)&Reserve[0];
  BufferIndex += 4;
  
  /* Reserve[1] */
  Reserve[1] = SwapUdintForIntel(pPVOpenRequest->Reserve[1]);
  *(UDINT*)&arTempBuf[BufferIndex] = *(UDINT*)&Reserve[1];
  
  /* BCC */
  BCCraw = CRCcheck((UDINT*) &arTempBuf, CRC_OPEN_TELEGRAM);
  pPVOpenRequest->BCC = BCCraw;
  BCC = SwapUintForIntel(BCCraw);
  *(UINT*)&arTempBuf[CRC_OPEN_TELEGRAM] = *(UINT*)&BCC;
  
  brsmemcpy((UDINT) pSendBuf, (UDINT) &arTempBuf, OPEN_BUFFER_LEN);
}


/****************************************************************************************************************/
/*              Übertragen eines PVOpenResponse-Telegramms                      */
/*              aus dem Empfangsbuffer                                */
/*              in Struktur "PVOpenResponse"                            */
/****************************************************************************************************************/

void CopyReceiveBufferToPVOpenResponse(
  UDINT* pRecvBuf)
{
  UINT  Status;       /* Byte 4/5 */
  UINT  ClientIndex;    /* Byte 6/7 */
  UINT  PVIndex;      /* Byte 8/9 */
  UDINT PVIdent;      /* Byte 10-13 */
  UINT  Laenge;       /* Byte 14/15 */
  UDINT Wert;       /* Byte 16-19 */
  UINT  BCC;        /* Byte 20/21 */
  USINT arTempBuf[OPEN_BUFFER_LEN];
  
  brsmemcpy((UDINT) &arTempBuf, (UDINT) pRecvBuf, sizeof(arTempBuf));
  
  /* Kennung */
  brsmemcpy ((UDINT) &PVOpenResponse.Kennung, (UDINT) &arTempBuf[0],sizeof(PVOpenResponse.Kennung));
  /* Status */
  brsmemcpy ((UDINT) &Status, (UDINT) &arTempBuf[4],sizeof(Status));
  PVOpenResponse.Status = SwapUintForIntel(Status);
  /* ClientIndex */
  brsmemcpy ((UDINT) &ClientIndex, (UDINT) &arTempBuf[6],sizeof(ClientIndex));
  PVOpenResponse.ClientIndex = SwapUintForIntel(ClientIndex);
  /* PVIndex */
  brsmemcpy ((UDINT) &PVIndex, (UDINT) &arTempBuf[8], sizeof(PVIndex));
  PVOpenResponse.PVIndex = SwapUintForIntel(PVIndex);
  /* PVIdent */
  brsmemcpy ((UDINT) &PVIdent, (UDINT) &arTempBuf[10],sizeof(PVIdent));
  PVOpenResponse.PVIdent = SwapUdintForIntel(PVIdent);
  /* Laenge */
  brsmemcpy ((UDINT) &Laenge, (UDINT) &arTempBuf[14],sizeof(Laenge));
  PVOpenResponse.Laenge = SwapUintForIntel(Laenge);
  /* Wert */
  brsmemcpy ((UDINT) &Wert,(UDINT) &arTempBuf[16],sizeof(Wert));
  PVOpenResponse.Wert = SwapUdintForIntel(Wert);
  /* BCC */
  brsmemcpy ((UDINT) &BCC, (UDINT) &arTempBuf[20],sizeof(BCC));
  PVOpenResponse.BCC = SwapUintForIntel(BCC);
  

  /* Kennung */
  *(UDINT*)&PVOpenResponse.Kennung = *(UDINT*)&arTempBuf[0];
  /* Status */
  *(UINT*)&Status = *(UINT*)&arTempBuf[4];
  PVOpenResponse.Status = SwapUintForIntel(Status);
  /* ClientIndex */
  *(UINT*)&ClientIndex = *(UINT*)&arTempBuf[6];
  PVOpenResponse.ClientIndex = SwapUintForIntel(ClientIndex);
  /* PVIndex */
  *(UINT*)&PVIndex = *(UINT*)&arTempBuf[8];
  PVOpenResponse.PVIndex = SwapUintForIntel(PVIndex);
  /* PVIdent */
  *(UDINT*)&PVIdent = *(UDINT*)&arTempBuf[10];
  PVOpenResponse.PVIdent = SwapUdintForIntel(PVIdent);
  /* Laenge */
  *(UINT*)&Laenge = *(UINT*)&arTempBuf[14];
  PVOpenResponse.Laenge = SwapUintForIntel(Laenge);
  /* Wert */
  *(UDINT*)&Wert = *(UDINT*)&arTempBuf[16];
  PVOpenResponse.Wert = SwapUdintForIntel(Wert);
  /* BCC */
  *(UINT*)&BCC = *(UINT*)&arTempBuf[20];
  PVOpenResponse.BCC = SwapUintForIntel(BCC);
  
}


/****************************************************************************************************************/
/*              Übertragen eines ClientLink-Telegramms                        */
/*              aus der Struktur "ClientLink" in den Sendebuffer                  */
/*              mit Eintragung der Checksum                             */
/****************************************************************************************************************/

void CopyClientLinkToSendBuffer(
  UDINT* pSendBuf)
{
  UINT  ClientIndex;              /* Byte 4/5 */
  UINT  BCC;                  /* Byte 6/7 */
  UINT  BCCraw;
  USINT arTempBuf[OPEN_BUFFER_LEN];
  
  /* Sendebuffer löschen */
  brsmemset((UDINT) &arTempBuf[0], 0, sizeof(arTempBuf));
  /* Kennung */
  *(UDINT*)&arTempBuf[0] = *(UDINT*)&ClientLink.Kennung;
  /* ClientIndex */
  ClientIndex = SwapUintForIntel(ClientLink.ClientIndex);
  *(UINT*)&arTempBuf[4] = *(UINT*)&ClientIndex;
  /* BCC */
  BCCraw = CRCcheck((UDINT*) &arTempBuf, 6);
  ClientLink.BCC = BCCraw;  
  BCC = SwapUintForIntel(BCCraw);
  *(UINT*)&arTempBuf[6] = *(UINT*)&BCC;

  brsmemcpy((UDINT) pSendBuf, (UDINT) &arTempBuf, sizeof(arTempBuf));
}


/****************************************************************************************************************/
/*              Übertragen eines LinkResponse-Telegramms                      */
/*              aus dem Empfangsbuffer                                */
/*              in Struktur "NAK"                                 */
/****************************************************************************************************************/

void CopyReceiveBufferToNAK(
  UDINT* pRecvBuf)
{
  UINT  Status;                 /* Byte 4/5 */
  UINT  Reserve2;               /* Byte 6/7 */
  UINT  BCC;                  /* Byte 8/9 */
  USINT arTempBuf[OPEN_BUFFER_LEN];

  brsmemcpy((UDINT) &arTempBuf, (UDINT) pRecvBuf, sizeof(arTempBuf));
  
  /* Kennung */
  brsmemcpy((UDINT) &NAK.Kennung, (UDINT) &arTempBuf[0],sizeof(NAK.Kennung));
  /* Status */
  brsmemcpy ((UDINT) &Status,(UDINT) &arTempBuf[4],sizeof(Status));
  NAK.Status = SwapUintForIntel(Status);
  /* Reserve2 */
  brsmemcpy ((UDINT) &Reserve2, (UDINT) &arTempBuf[6],sizeof(Reserve2));
  NAK.Reserve2 = SwapUintForIntel(Reserve2);
  /* BCC */
  brsmemcpy ((UDINT) &BCC, (UDINT) &arTempBuf[8],sizeof(BCC));
  NAK.BCC = SwapUintForIntel(BCC);


  /* Kennung */
  *(UDINT*)&NAK.Kennung = *(UDINT*)&arTempBuf[0];
  /* Status */
  *(UINT*)&Status = *(UINT*)&arTempBuf[4];
  NAK.Status = SwapUintForIntel(Status);
  /* Reserve2 */
  *(UDINT*)&Reserve2 = *(UDINT*)&arTempBuf[4];
  NAK.Reserve2 = SwapUintForIntel(Reserve2);
  /* BCC */
  *(UINT*)&BCC = *(UINT*)&arTempBuf[8];
  NAK.BCC = SwapUintForIntel(BCC);

}


/****************************************************************************************************************/
/*              Übertragen eines PVEvent-Telegramms                         */
/*              aus dem Empfangsbuffer                                */
/*              in Struktur "PVEvent"                               */
/****************************************************************************************************************/

void CopyReceiveBufferToPVEvent(
  UDINT* pRecvBuf)
{
  UINT  PVIndex;                /* Byte 4/5 */
  UINT  Laenge;                 /* Byte 6/7 */
  UDINT Wert;                 /* Byte 8-11 */
  UINT  EventCnt;               /* Byte 12/13 */
  UINT  BCC;                  /* Byte 14/15 */
  USINT arTempBuf[20];

  brsmemcpy((UDINT)arTempBuf, (UDINT)pRecvBuf, sizeof(arTempBuf));

  /* Kennung */
  *(UDINT*)&PVEvent.Kennung = *(UDINT*)&arTempBuf[0];
  /* PVIndex */
  *(UINT*)&PVIndex = *(UINT*)&arTempBuf[4];
  PVEvent.PVIndex = SwapUintForIntel(PVIndex);
  /* Laenge */
  *(UINT*)&Laenge = *(UINT*)&arTempBuf[6];
  PVEvent.Laenge = SwapUintForIntel(Laenge);
  /* Wert */
  *(UDINT*)&Wert = *(UDINT*)&arTempBuf[8];
  PVEvent.Wert = SwapUdintForIntel(Wert);
  /* EventCnt */
  *(UINT*)&EventCnt = *(UINT*)&arTempBuf[12];
  PVEvent.EventCnt = SwapUintForIntel(EventCnt);
  /* BCC */
  *(UINT*)&BCC = *(UINT*)&arTempBuf[14];
  PVEvent.BCC = SwapUintForIntel(BCC);

}


/****************************************************************************************************************
* --- Übertragen eines PVEventAcknowledge-Telegramms                    
* --- aus der Struktur "PVEventAcknowledge" in den Sendebuffer              
* --- mit Eintragung der Checksum                             
****************************************************************************************************************/

void CopyPVEventAcknowledgeToSendBuffer(
  UDINT* pRecvBuf)
{
  UINT  Status;                 /* Byte 4/5 */
  UINT  PVIndex;                /* Byte 6/7 */
  UINT  ClientIndex;              /* Byte 8/9 */
  UINT  EventCnt;               /* Byte 10/11 */
  UINT  BCC;                  /* Byte 12/13 */  
  UINT  BCCraw;
  USINT arTempBuf[20];
  
  /* Sendebuffer löschen */
  brsmemset((UDINT) &arTempBuf[0], 0, sizeof(arTempBuf));
  /* Kennung */
  brsmemcpy ((UDINT) &arTempBuf[0], (UDINT) &PVEventAcknowledge.Kennung,sizeof(PVEventAcknowledge.Kennung));
  /* Status */
  Status = SwapUintForIntel(PVEventAcknowledge.Status);
  brsmemcpy ((UDINT) &arTempBuf[4],(UDINT) &Status,sizeof(Status));
  /* PVIndex */
  PVIndex = SwapUintForIntel(PVEventAcknowledge.PVIndex);
  brsmemcpy ((UDINT) &arTempBuf[6], (UDINT) &PVIndex,sizeof(PVIndex));
  /* ClientIndex */
  ClientIndex = SwapUintForIntel(PVEventAcknowledge.ClientIndex);
  brsmemcpy ((UDINT) &arTempBuf[8], (UDINT) &ClientIndex,sizeof(ClientIndex));
  /* EventCnt */
  EventCnt = SwapUintForIntel(PVEventAcknowledge.EventCnt);
  brsmemcpy ((UDINT) &arTempBuf[10], (UDINT) &EventCnt,sizeof(EventCnt));
  /* BCC */
  BCCraw = CRCcheck((UDINT*) &arTempBuf, 12);
  PVEventAcknowledge.BCC = BCCraw;  
  BCC = SwapUintForIntel(BCCraw);
  brsmemcpy ((UDINT) &arTempBuf[12],(UDINT) &BCC,sizeof(BCC));


  /* Sendebuffer löschen */
  brsmemset((UDINT) &arTempBuf[0], 0, sizeof(arTempBuf));
  /* Kennung */
  *(UDINT*)&arTempBuf[0] = *(UDINT*)&PVEventAcknowledge.Kennung;
  /* Status */
  Status = SwapUintForIntel(PVEventAcknowledge.Status);
  *(UINT*)&arTempBuf[4] = *(UINT*)&Status;
  /* PVIndex */
  PVIndex = SwapUintForIntel(PVEventAcknowledge.PVIndex);
  *(UINT*)&arTempBuf[6] = *(UINT*)&PVIndex;
  /* ClientIndex */
  ClientIndex = SwapUintForIntel(PVEventAcknowledge.ClientIndex);
  *(UINT*)&arTempBuf[8] = *(UINT*)&ClientIndex;
  /* EventCnt */
  EventCnt = SwapUintForIntel(PVEventAcknowledge.EventCnt);
  *(UINT*)&arTempBuf[10] = *(UINT*)&EventCnt;
  /* BCC */
  BCCraw = CRCcheck((UDINT*) &arTempBuf, 12);
  PVEventAcknowledge.BCC = BCCraw;  
  BCC = SwapUintForIntel(BCCraw);
  *(UINT*)&arTempBuf[12] = *(UINT*)&BCC;
  
  brsmemcpy((UDINT)pRecvBuf, (UDINT) &arTempBuf, sizeof(arTempBuf));

}


/****************************************************************************************************************/
/*              Übertragen eines LifeCheckRequest-Telegramms                    */
/*              aus der Struktur "LifeCheckRequest" in den Sendebuffer                */
/*              mit Eintragung der Checksum                             */
/****************************************************************************************************************/

void CopyLifeCheckRequestToSendBuffer()
{
  UINT  Reserve1;               /* Byte 4/5 */
  UINT  BCC;                  /* Byte 6/7 */
  UINT  BCCraw;

  /* Sendebuffer löschen */
  brsmemset((UDINT) &arSendBufLife[0], 0, sizeof(arSendBufLife));
  
  /* Kennung */
  brsmemcpy ((UDINT) &arSendBufLife[0], (UDINT) &LifeCheckRequest.Kennung, sizeof(LifeCheckRequest.Kennung));
  /* Reserve1 */
  Reserve1 = SwapUintForIntel(LifeCheckRequest.Reserve1);
  brsmemcpy ((UDINT) &arSendBufLife[4],(UDINT) &Reserve1,sizeof(Reserve1));
  /* BCC */
  BCCraw = CRCcheck((UDINT*) &arSendBufLife, 6);
  LifeCheckRequest.BCC = BCCraw;  
  BCC = SwapUintForIntel(BCCraw);
  brsmemcpy ((UDINT) &arSendBufLife[6], (UDINT) &BCC,sizeof(BCC));


  /* Sendebuffer löschen */
  brsmemset((UDINT) &arSendBufLife[0], 0, sizeof(arSendBufLife));
  /* Kennung */
  *(UDINT*)&arSendBufLife[0] = *(UDINT*)&LifeCheckRequest.Kennung;
  /* Reserve1 */
  Reserve1 = SwapUintForIntel(LifeCheckRequest.Reserve1);
  *(UINT*)&arSendBufLife[4] = *(UINT*)&Reserve1;
  /* BCC */
  BCCraw = CRCcheck((UDINT*) &arSendBufLife, 6);
  LifeCheckRequest.BCC = BCCraw;  
  BCC = SwapUintForIntel(BCCraw);
  *(UINT*)&arSendBufLife[6] = *(UINT*)&BCC;

}


/****************************************************************************************************************/
/*              Übertragen eines LifeCheckResponse-Telegramms                     */
/*              aus dem Empfangsbuffer                                */
/*              in Struktur "LifeCheckResponse"                           */
/****************************************************************************************************************/

void CopyReceiveBufferToLifeCheckResponse()
{
  UINT  ClientStatus;             /* Byte 4/5 */
  UINT  BCC;                  /* Byte 6/7 */

  /* Kennung */
  brsmemcpy ((UDINT) &LifeCheckResponse.Kennung, (UDINT) &arRecvBufLife[0], sizeof(LifeCheckResponse.Kennung));
  /* ClientStatus */
  brsmemcpy ((UDINT) &ClientStatus, (UDINT) &arRecvBufLife[4], sizeof(ClientStatus));
  LifeCheckResponse.ClientStatus = SwapUintForIntel(ClientStatus);
  /* BCC */
  brsmemcpy ((UDINT) &BCC,(UDINT) &arRecvBufLife[6],sizeof(BCC));
  LifeCheckResponse.BCC = SwapUintForIntel(BCC);


  /* Kennung */
  *(UDINT*)&LifeCheckResponse.Kennung = *(UDINT*)&arRecvBufLife[0];
  /* ClientStatus */
  *(UINT*)&ClientStatus = *(UINT*)&arRecvBufLife[4];
  LifeCheckResponse.ClientStatus = SwapUintForIntel(ClientStatus);
  /* BCC */
  *(UINT*)&BCC = *(UINT*)&arRecvBufLife[6];
  LifeCheckResponse.BCC = SwapUintForIntel(BCC);

}


/****************************************************************************************************************/
/*              Ermittlung eines Servers über eine IP - Adresse                   */
/*    -> Eingabe: IPAdresse (numerisch z.B.: 0x0A430A0C                           */
/*    -> Rückgabe: Adresse auf Servereintrag                                  */
/*    -> Rückgabe: Index (Offset) des Servers in Liste                            */
/****************************************************************************************************************/

UDINT GetServerByIp(
  UDINT IpAddr,       /* Ip Adresse des zu suchenden Servers */
  UINT* pServerIndex)   /* Adresse des Servereintrages       */
{
  UINT  uiLoop;
  Client_List_Typ*  pServer;

  if ((IpAddr == 0) || (pServerIndex == 0))
  {
    return NULL;
  }
  for (uiLoop = 0; uiLoop < MAX_SERVER; uiLoop++)
    {
    pServer = pClientVerwaltungStart + uiLoop;
    if (pServer->IPAdresse == IpAddr)
    {
      *pServerIndex = uiLoop;
      return (UDINT)pServer;
    }
  }
  return NULL;
}


/****************************************************************************************************************/
/*              Ermittlung eines Servers über eine IP - Adresse                   */
/*    -> Eingabe: IPAdresse (String)                                      */
/*    -> Rückgabe: Adresse auf Servereintrag                                  */
/*    -> Rückgabe: Index (Offset) des Servers in Liste                            */
/****************************************************************************************************************/

UDINT GetServerByIpStr(
  UDINT* pIpAddr,     /* Adresse auf String mit Ip Adresse des zu suchenden Servers */
  UINT* pServerIndex)   /* Adresse des Servereintrages                    */
{
  UINT uiLoop = 0;
  Client_List_Typ*  pServer = NULL;
  DINT sdRes = 0;

  if ((pIpAddr == NULL) || (pServerIndex == NULL))
  {
    return NULL;
  }
  
  for (uiLoop = 0; uiLoop < MAX_SERVER; uiLoop++)
    {
    pServer = pClientVerwaltungStart + uiLoop;
    sdRes = brsstrcmp((UDINT) &pServer->IpAdr, (UDINT) pIpAddr);
    if (sdRes == 0)
    {
      *pServerIndex = uiLoop;
      return (UDINT)pServer;
    }
  }
  return NULL;
}


/****************************************************************************************************************/
/*              Status innerhalb der Variablenliste setzen                      */
/*    -> Eingabe: Adresse auf PV Eintrag                                    */
/*    -> Eingabe: numerischer Status                                      */
/****************************************************************************************************************/

void SetPvState(
  UDINT   pPvAdr,   /* Adresse eines Variablenelements  */
  UINT  State)    /* Zustand der Variable       */
{
  PvList_Typ* pListTyp;


  if (pPvAdr == NULL)
  {
    return;
  }
  
  pListTyp = (PvList_Typ*) pPvAdr;
  pListTyp->Status = State;

  brsmemset((UDINT) &pListTyp->StatusTxt, 0, sizeof(pListTyp->StatusTxt));  
  
  switch (State)
  {
    case ERR_NO:
      brsmemcpy((UDINT) &pListTyp->StatusTxt, (UDINT) ERR_NO_TXT, brsstrlen((UDINT) ERR_NO_TXT));
      break;
      
    case ERR_BCC:
      brsmemcpy((UDINT) &pListTyp->StatusTxt, (UDINT) ERR_BCC_TXT, brsstrlen((UDINT) ERR_BCC_TXT));
      break;
      
    case ERR_TIMEOUT:
      brsmemcpy((UDINT) &pListTyp->StatusTxt, (UDINT) ERR_TIMEOUT_TXT, brsstrlen((UDINT) ERR_TIMEOUT_TXT));
      break;
      
    case ERR_ILLEGAL_LENGTH:
      brsmemcpy((UDINT) &pListTyp->StatusTxt, (UDINT) ERR_ILLEGAL_LENGTH_TXT, brsstrlen((UDINT) ERR_ILLEGAL_LENGTH_TXT));
      break;
      
    case ERR_OPEN_FAILED:
      brsmemcpy((UDINT) &pListTyp->StatusTxt, (UDINT) ERR_OPEN_FAILED_TXT, brsstrlen((UDINT) ERR_OPEN_FAILED_TXT));
      break;
      
    case ERR_VALUE_TO_LONG:
      brsmemcpy((UDINT) &pListTyp->StatusTxt, (UDINT) ERR_VALUE_TO_LONG_TXT, brsstrlen((UDINT) ERR_VALUE_TO_LONG_TXT));
      break;
    
    case ERR_VALUE_NOTX_SERVER:
      brsmemcpy((UDINT) &pListTyp->StatusTxt, (UDINT) ERR_VALUE_NOTX_SERVER_TXT, brsstrlen((UDINT) ERR_VALUE_NOTX_SERVER_TXT));
      break;
    
    case ERR_OPEN_SEND_FAILED:
      brsmemcpy((UDINT) &pListTyp->StatusTxt, (UDINT) ERR_OPEN_SEND_FAILED_TXT, brsstrlen((UDINT) ERR_OPEN_SEND_FAILED_TXT));
      break;

    case ERR_ILLEGAL_PV_INDEX:
      brsmemcpy((UDINT) &pListTyp->StatusTxt, (UDINT) ERR_ILLEGAL_PV_INDEX_TXT, brsstrlen((UDINT) ERR_ILLEGAL_PV_INDEX_TXT));
      break;
    
    case ERR_NO_CLIENT_SPACE:
      brsmemcpy((UDINT) &pListTyp->StatusTxt, (UDINT) ERR_NO_CLIENT_SPACE_TXT, brsstrlen((UDINT) ERR_NO_CLIENT_SPACE_TXT));
      break;

    case ERR_NO_PV_SPACE:
      brsmemcpy((UDINT) &pListTyp->StatusTxt, (UDINT) ERR_NO_PV_SPACE_TXT, brsstrlen((UDINT) ERR_NO_PV_SPACE_TXT));
      break;
    
    default:
      brsmemcpy((UDINT) &pListTyp->StatusTxt, (UDINT) ERR_NO_PV_UNDEF_TXT, brsstrlen((UDINT) ERR_NO_PV_UNDEF_TXT));
      break;
    
  }     
  
  return;
}


/****************************************************************************************************************/
/*              Status anhand PV Status auf Server setzen                       */
/*    -> Eingabe: Adresse auf PV Eintrag                                    */
/*    -> Eingabe: numerischer Status                                      */
/****************************************************************************************************************/

void SetServerState(
  UDINT pPvEintrag,
  UINT uiServerState)
{
  PvList_Typ* pPv;

  if (pPvEintrag == NULL)
  {
    return;
  }

  pPv = (PvList_Typ*) pPvEintrag;
  
  switch (uiServerState)
  { 
    case STATUS_SERV_NO_VAR:
      SetPvState((UDINT) pPv, ERR_VALUE_NOTX_SERVER);
      break;
    
    case STATUS_SERV_NO_OPEN:
      SetPvState((UDINT) pPv, ERR_OPEN_FAILED);
      break;
      
    case STATUS_SERV_NO_CLIENT_SPACE:
      SetPvState((UDINT) pPv, ERR_NO_CLIENT_SPACE);
      break;
      
    case STATUS_SERV_NO_PV_SPACE:
      SetPvState((UDINT) pPv, ERR_NO_PV_SPACE);
      break;

    case STATUS_SERV_VAR_ILL_LENGTH:
      SetPvState((UDINT) pPv, ERR_ILLEGAL_LENGTH);
      break;
  }

}


/****************************************************************************************************************/
/*              Status anhand PV Status auf Server setzen                       */
/*    -> Eingabe: Adresse auf PV Eintrag                                    */
/*    -> Eingabe: numerischer Status                                      */
/****************************************************************************************************************/

UINT GetPvStateFromServerState(
  UINT uiServerState)
{

  switch (uiServerState)
  { 
    case STATUS_SERV_NO_VAR:
      return ERR_VALUE_NOTX_SERVER;
      break;
    
    case STATUS_SERV_NO_OPEN:
      return ERR_OPEN_FAILED;
      break;
      
    case STATUS_SERV_NO_CLIENT_SPACE:
      return ERR_NO_CLIENT_SPACE;
      break;
      
    case STATUS_SERV_NO_PV_SPACE:
      return ERR_NO_PV_SPACE;
      break;

    case STATUS_SERV_VAR_ILL_LENGTH:
      return ERR_ILLEGAL_LENGTH;
      break;
  }

  return 0;
}


/****************************************************************************************************************/
/*            Neuanmeldung zu allen Servern neu starten                       */
/****************************************************************************************************************/

void ResetLifeCheck(Client_List_Typ* pClientStart)
{
  UINT i = 0;
  Client_List_Typ* pServer;
  
  /* --- Alle Server auf Fehler setzen ---------- */
  for (i = 0; i < MAX_SERVER; i++)
  {
    pServer = pClientStart + i;   
    if (pServer->IPAdresse != 0)
    {
      pServer->LifeCheck = STATE_LIFE_ERR;
      pServer->LifeCheckTime = 0;

      ResetIpAdrForServer(pClientStart, i); /* Ip Adresse bei Hostnamen löschen -> wird neu angefordert */
    }
  }  
}


/****************************************************************************************************************/
/*            Neuanmeldung zu einem Servern neu starten                       */
/****************************************************************************************************************/

void ResetIpAdrForServer(Client_List_Typ* pClientStart, UINT uiServerNr)
{
  Client_List_Typ* pServer;
  
  if (uiServerNr >= MAX_SERVER)
  {
    return;
  }
  
  pServer = pClientStart + uiServerNr;
  if (pServer->ConfByHost == 1)
  {
    pServer->IPAdresse = 0;
    brsmemset((UDINT) &pServer->IpAdr, 0, sizeof(pServer->IpAdr));
  }
}


/***************************************************************************************************************
*           Generierung des Telegramms für Conn
****************************************************************************************************************/

/* komplettes Telegramm für Anmeldung generieren: */
void BuildConnTelegram(
  ClientConnectRequest_typ* pRequest, // Zielstruktur
  UINT iTimeout,
  UINT iClientIndex,
  UINT iMode
  )
{
  
  GenerateConnHeaderTelegramm((UDINT*) &pRequest->Kennung, iMode);
  pRequest->ClientIndex = iClientIndex;
  pRequest->Timeout = iTimeout;
  
}


/***************************************************************************************************************
*           Erzeugen der Telegram Kennung nach Modus
****************************************************************************************************************/

void GenerateConnHeaderTelegramm(
  UDINT *pKennung,  /* Kennung  */
  UINT iMode)      /* Modus    */
{
  if (iMode == MODE_OLD)
  {    
    brsmemcpy((UDINT) pKennung, (UDINT) TELEGRAMM_IDENT_CONN, 4);
  }
  else if (iMode == MODE_NEW)
  {
    brsmemcpy((UDINT) pKennung, (UDINT) TELEGRAMM_IDENT_CONX, 4);    
  }  
}


/***************************************************************************************************************
*           Generierung des Telegramms für Link
****************************************************************************************************************/

void GenerateLinkHeaderTelegramm(
  UDINT *pKennung,  /* Kennung  */
  UINT iMode)       /* Mode  */
{
  if (iMode == MODE_OLD)
  {   
    brsmemcpy((UDINT) pKennung, (UDINT) TELEGRAMM_IDENT_LINK, 4);
  }
  else if (iMode == MODE_NEW)
  {
    brsmemcpy((UDINT) pKennung, (UDINT) TELEGRAMM_IDENT_LINX, 4);          
  }
}


/*****************************************************************************************************************
*            Generierung des Telegramms für Open (OPEN)
*****************************************************************************************************************/

BOOL GenerateOpenTelegramm(
  UINT* puiPvOffset,      /* aktuelle Variable in PV Liste*/
  UINT uiActServ,         /* Anmeldung erfolgt f. Server x*/
  PvList_Typ* pPvStartAdr,
  Client_List_Typ* pClientStartAdr,
  UDINT* pStartAdrBuf,        /* Buffer für Senden - Startadresse */  
  UINT* puiTelLen,            /* Telegrammlänge*/
  PvConnect_Typ* pPvConnect,  /* Pv Cnt */
  PVOpenRequest_typ *pPvOpenRequest
  ) 
{
  PvList_Typ    *pPvEntry = NULL;
  Client_List_Typ *pClientEntry = NULL;
  UINT      uiPvEntry = *puiPvOffset;
  UINT      uiPvList = 0;  
  USINT     *pBufOpenWork = NULL;
  USINT     *pBufInsertPvCnt = NULL;
  BOOL      bRetVal = FALSE;  /*Rückgabewert: eine Variable wurde gefunden */
  
  UNUSED(pBufInsertPvCnt);    // for the compiler
  
  pBufOpenWork = (USINT*) pStartAdrBuf;  
  
  /* --- Sendebuffer löschen ---------------------------------------------- */
  brsmemset((UDINT) pStartAdrBuf, 0, (UDINT) MAX_UDP_FRAME_LEN);

  /* === Telegrammgenerierung ============================================= */
  
  /* --- Kennung ---------------------------------------------------------- */  
  brsmemcpy((UDINT) &pPvOpenRequest->Kennung, (UDINT) TELEGRAMM_IDENT_OPEN, 4);
  
  /* --- Länge ----------------------------------------------------------- */
  pBufInsertPvCnt = pBufOpenWork; /* für späteres Einfügen !*/  
  
  for (uiPvList = uiPvEntry; uiPvList < MAX_VARIABLEN; uiPvList++)
    {
    pPvEntry = pPvStartAdr + uiPvList;
    
    if (Diagnose.LogPointerInfo == TRUE)
    {
      ValidatePointerClient((UDINT) pPvEntry, 13, (MemManagerDiag_type* ) &sMemDiag, 1);
    }
    
    /* --- nur Variablen für Server x werden bearbeitet ---------------- */
    if (pPvEntry->Client == uiActServ)
    {
      /* --- gültige Variable vorhanden ------------------------------ */
      if ((pPvEntry->pPv != NULL) && (pPvEntry->PvLaenge > 0))
      {
        if  ((PVOpenCnt == 0) || ((PVOpenCnt == 1)  && 
          ((pPvEntry->Anmeldung  == STAT_DEFAULT)  ||     /* PV noch nicht abgearbeitet */
          (pPvEntry->Anmeldung  == STAT_PV_NOT_AVAILABLE) || /* PV am Server nicht vorhanden */
          (pPvEntry->Anmeldung  == STAT_TIMEOUT)  ||     /* Timeout während der Übertragung */
          (pPvEntry->Anmeldung  == STAT_PV_INDEX) ||     /* falscher PV Index während der Übertragung */
          (pPvEntry->Anmeldung  == STAT_BCC))))        /* Checksummenfehler    */  
        {
  
          if (bRetVal == FALSE) /* noch nichts gefunden, erste PV auf Sendebuffer legen*/
          {
            pClientEntry = pClientStartAdr + pPvEntry->Client;
          
            if (Diagnose.LogPointerInfo == TRUE)
            {
              ValidatePointerClient((UDINT) pClientEntry, 14, (MemManagerDiag_type* ) &sMemDiag, 1);
            }
          
            brsmemset((UDINT) &pPvOpenRequest->Variablenname, 0, sizeof(pPvOpenRequest->Variablenname));
          
            brsstrcpy((UDINT) &pPvOpenRequest->Variablenname, (UDINT) &pPvEntry->NameServer);
            pPvOpenRequest->PVIndex     = uiPvList + 1;
            pPvOpenRequest->ClientIndex = pClientEntry->ClientIndex;
            pPvOpenRequest->Laenge      = pPvEntry->PvLaenge;
            pPvOpenRequest->Hysterese   = pPvEntry->Hysterese;
            pPvOpenRequest->SyncTime    = pPvEntry->SyncTime;

            // Telegrammdaten auf den Buffer kopieren
            CopyPVOpenRequestToSendBuffer((UDINT*) pBufOpenWork, pPvOpenRequest);       

            pPvEntry->Anmeldung = STAT_DEFAULT;
            bRetVal = TRUE;
          }
          break; /* eine Variable gefunden,... */
        }
      }
    }
  }
  
  /* --- Rückgabe der Werte ---------------------------------------------- */
  *puiTelLen = OPEN_BUFFER_LEN;
  *puiPvOffset = uiPvList + 1; // auf nächste Variable gehen...
  return bRetVal;
}


/*****************************************************************************************************************
*            Generierung des Telegramms für Open (OPEX)
*****************************************************************************************************************/

void GenerateOpenTelegrammX(
  UINT* puiPvOffset,            /* aktuelle Variable in PV Liste*/
  UINT uiActServ,               /* Anmeldung erfolgt f. Server x*/
  PvList_Typ* pPvStartAdr,      /* Variablen Startadresse */
  Client_List_Typ* pClientStartAdr, /* Server Liste Startadresse*/
  UDINT* pStartAdrBuf,          /* Buffer für Senden - Startadresse */  
  UINT* puiTelLen,              /* Telegrammlänge*/
  PvConnect_Typ* pPvConnect,    /* Pv Cnt */
  UINT* puiPvCnt                /* Anzahl der Variablen in diesem Telegramm */
  ) 
{
  PvList_Typ    *pPvEntry = NULL;
  Client_List_Typ *pClientEntry = NULL;
  UINT      uiPvEntry = *puiPvOffset;
  UINT      uiPvList = 0;
  PVOpenRequestX_typ sPvOpenRequest;
  USINT       *pBufOpenWork = NULL;
  USINT       *pBufInsertPvCnt = NULL;
  UINT        uiTelCnt = 0;
  UINT      uiBCCVal = 0;
  UINT      uiPvEntryCnt = 0;

  pBufOpenWork = (USINT*) pStartAdrBuf;
  *puiPvCnt = 0;
  
  /* --- Sendebuffer löschen ---------------------------------------------- */
  brsmemset((UDINT) pStartAdrBuf, 0, MAX_UDP_FRAME_LEN);

  /* === Telegrammgenerierung ============================================= */
  
  /* --- Kennung ---------------------------------------------------------- */
  brsmemcpy((UDINT) pBufOpenWork, (UDINT) TELEGRAMM_IDENT_OPEX, 4);
  pBufOpenWork+=4;
  uiTelCnt+=4;
  
  /* --- Länge ----------------------------------------------------------- */
  pBufInsertPvCnt = pBufOpenWork; /* für späteres Einfügen !*/
  pBufOpenWork+=2;
  uiTelCnt +=2;
  
  for (uiPvList = uiPvEntry; uiPvList < MAX_VARIABLEN; uiPvList++)
    {
    pPvEntry = pPvStartAdr + uiPvList;
    
    if (Diagnose.LogPointerInfo == TRUE)
    {
      ValidatePointerClient((UDINT) pPvEntry, 13, (MemManagerDiag_type* ) &sMemDiag, 1);
    }
    
    /* --- nur Variablen für Server x werden bearbeitet ---------------- */
    if (pPvEntry->Client == uiActServ)
    {
      /* --- gültige Variable vorhanden ------------------------------ */
      if ((pPvEntry->pPv != NULL) && (pPvEntry->PvLaenge > 0))
      {
        if  ((PVOpenCnt == 0) || ((PVOpenCnt == 1)  && 
         ((pPvEntry->Anmeldung  == STAT_DEFAULT)  ||     /* PV noch nicht abgearbeitet */
          (pPvEntry->Anmeldung  == STAT_PV_NOT_AVAILABLE) || /* PV am Server nicht vorhanden */
          (pPvEntry->Anmeldung  == STAT_TIMEOUT)  ||     /* Timeout während der Übertragung */
          (pPvEntry->Anmeldung  == STAT_PV_INDEX) ||     /* falscher PV Index während der Übertragung */
          (pPvEntry->Anmeldung  == STAT_BCC))))        /* Checksummenfehler    */  
        {
          /* --- Telegrammlänge erreicht ------------------------ */
          if (uiTelCnt + sizeof(sPvOpenRequest) + 2 > MAX_UDP_FRAME_LEN)  /* nächste Variable + CRC geht nicht in Telegramm*/
          {
            break;
          }
  
          pClientEntry = pClientStartAdr + pPvEntry->Client;
          
          if (Diagnose.LogPointerInfo == TRUE)
          {
            ValidatePointerClient((UDINT) pClientEntry, 14, (MemManagerDiag_type* ) &sMemDiag, 1);
          }
          
          brsmemset((UDINT) &sPvOpenRequest.Variablenname, 0, sizeof(sPvOpenRequest.Variablenname));
          
          brsstrcpy((UDINT) &sPvOpenRequest.Variablenname, (UDINT) &pPvEntry->NameServer);
          sPvOpenRequest.PVIndex      = uiPvList + 1;
          sPvOpenRequest.ClientIndex  = pClientEntry->ClientIndex;
          sPvOpenRequest.Laenge       = pPvEntry->PvLaenge;
          sPvOpenRequest.Hysterese    = pPvEntry->Hysterese;
          sPvOpenRequest.SyncTime     = pPvEntry->SyncTime;

          /* 1x komplette PV an Sendebuffer anhängen */
          brsmemcpy((UDINT) pBufOpenWork, (UDINT) &sPvOpenRequest, sizeof(sPvOpenRequest));
          pBufOpenWork += sizeof(sPvOpenRequest);
          uiTelCnt += sizeof(sPvOpenRequest);
          uiPvEntryCnt++;

          pPvEntry->Anmeldung = STAT_DEFAULT ;
          
          /* --- Anzahl der Variablen f. Anmeldung -------------- */
          pPvConnect->uiTry++;
          
          (*puiPvCnt)++;
          
        }
      }
    }
  }
  
  /* --- Anzahl der Variableneinträge nachziehen ------------------------- */
  *pBufInsertPvCnt = uiPvEntryCnt;
  
  /* --- CRC - Check ----------------------------------------------------- */
  uiBCCVal = CRCcheck((UDINT*) pStartAdrBuf, uiTelCnt);
  brsmemcpy((UDINT) pBufOpenWork, (UDINT) &uiBCCVal, 2);
  uiTelCnt+=2;  
  
  /* --- Rückgabe der Werte ---------------------------------------------- */
  *puiPvOffset = uiPvList;
  *puiTelLen = uiTelCnt;    
}


/*****************************************************************************************************************
*            Überprüfung, ob Neuanmeldung zulässig ist
*****************************************************************************************************************/

BOOL ServIsValidForOpen (
  UDINT *pServEntry,    /* Adresse des Servereintrags   */
  BOOL bReg             /* Neue Anmeldung */
  )
{
  Client_List_Typ* pActServ = NULL;
  
  pActServ = (Client_List_Typ*)pServEntry;
  
  if (((pActServ->IpAdr[0] != 0) &&    	/* Ip-Adresse als String */
    (pActServ->IPAdresse != 0) &&   	/* Ip-Adresse als numerischer Wert  */
    (pActServ->Timeout > 0)) &&      /* Timout konfiguriert   */     
    ((bReg == TRUE) ||        		/* Neuanmeldung per Hand */
    (pActServ->LifeCheck == STATE_LIFE_ERR)))
  {	
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}


/****************************************************************************************************************
* --- Überprüfung des Bufferinhalts
*****************************************************************************************************************/

BOOL ValidateDataFromRecvBuf(
  UDINT *pRecvBuf,        /* Adresse des Empfangsbuffers  */
  UINT uiTelLen)          /* Telegrammlänge       */
{
  UINT uiCrcCheck = 0;
  UINT uiCrcFromTel = 0;
  
  uiCrcCheck = CRCcheck((UDINT*) pRecvBuf, uiTelLen - 2);
  brsmemcpy((UDINT) &uiCrcFromTel, (UDINT) pRecvBuf + uiTelLen - 2, 2);
  
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
* --- Zuordnung der empfangenen Daten - OPEX
*****************************************************************************************************************/

UINT AssignPvDataToListOpenX(
  PVOpenResponseX_typ sPv)    /* ein Variableneintrag */
{
  PvList_Typ* pPvEntry;
  USINT*  pusPvVal = NULL;
  UINT* puiPvVal = NULL;
  UDINT*  pudPvVal = NULL;

  if (sPv.Status != STATUS_SERV_OK)
  {
    return 1;
  }
  
  if (sPv.PVIndex == 0)
  {
    return 2;
  }
  
  pPvEntry = pVariablenVerwaltungStart + (sPv.PVIndex - 1);
  
  if ((sPv.PVIdent == NULL) || (sPv.Laenge == 0))
  {
    return 3;
  }
  
  if (pPvEntry->pPv == NULL)
  {
    return 5;
  }
        
  switch (sPv.Laenge)
  {
    case 1:
      {
        pusPvVal  = (USINT*)pPvEntry->pPv;
        *pusPvVal = sPv.Wert;
        break;
      }
    
    case 2:
      {
        puiPvVal  = (UINT*)pPvEntry->pPv;
        *puiPvVal = sPv.Wert;
        break;
      }
    
    case 4:
      {
        pudPvVal  = (UDINT*)pPvEntry->pPv;
        *pudPvVal = sPv.Wert;
        break;
      }
    
    default:
      {
        return 4;
        break;
      }
  }

  pPvEntry->pPvServer = sPv.PVIdent;
  pVariablenManagement->Anmeldung = STAT_OK;
  pVariablenManagement->LetzterWert = sPv.Wert;
  pVariablenManagement->LetzterWertCnt++;

  return 0;
}


/****************************************************************************************************************
* --- Ermittlung des nächsten PV Eintrags aus Empfangsbuffer - OPEX
*****************************************************************************************************************/

BOOL GetDataFromRecvBufOpen(
  UDINT* pRecvBufOpen,          /* Empfangsbuffer*/
  UINT  uiGetMaxPv,           /* Anzahl der Variablen im Telegramm */
  PVOpenResponseX_typ* pPVOpenResponseX,  /* ein Variableneintrag */
  UINT* puiPvEntry)
{
  USINT* pusStartData = NULL;
  PVOpenResponseX_typ* pGetData = NULL;

  /* --- auf Start der eigentlichen Daten verweisen: +4(Kennung); +2(Variable) + ClientIndex */
  pusStartData = (USINT*) pRecvBufOpen + 4  + 2 + 2;
  
  if (uiGetMaxPv > *puiPvEntry)
  {
    pGetData = (PVOpenResponseX_typ*) pusStartData + (*puiPvEntry);
    brsmemcpy((UDINT) pPVOpenResponseX, (UDINT) pGetData, sizeof(PVOpenResponseX_typ));
    (*puiPvEntry)++;
    return TRUE;
  } 
  else
  {
    return FALSE;
  }
}


/****************************************************************************************************************
* --- Ermittlung des nächsten PV Eintrags aus Empfangsbuffer - OPEX
*****************************************************************************************************************/

void SetPvStateReg(
  UDINT   pudPvAdr, /* Adresse eines Variablenelements  */
  UINT  uiState)  /* Zustand der Variable       */
{
  PvList_Typ* pPvEntry = NULL;

  pPvEntry = (PvList_Typ*)pudPvAdr;
  pPvEntry->Anmeldung = uiState;
}

/****************************************************************************************************************
* --- Ermittlung der Basisdaten aus Empfangsbuffer - EVNX
*****************************************************************************************************************/

void GetMainDataFromRecvBufEvent(
  UDINT* pRecvBufOpen, 
  UINT*  puiPvCnt,
  UINT*  puiEventCnt)
{
  /* --- Anzahl der Variablen im Telegramm ermitteln ------------------------- */
  brsmemcpy((UDINT) puiPvCnt, (UDINT) pRecvBufOpen + 4, 2);
  
  /* --- Anzahl der Variablen im Telegramm ermitteln ------------------------- */
  brsmemcpy((UDINT) puiEventCnt, (UDINT) pRecvBufOpen + 6, 2); 
}


/****************************************************************************************************************
* --- Ermittlung des nächsten PV Eintrags aus Empfangsbuffer - EVNX
*****************************************************************************************************************/

BOOL GetDataFromRecvBufEvent(
  UDINT* pRecvBufOpen,      /* Empfangsbuffer*/
  UINT  uiGetMaxPv,         /* Anzahl der Variablen im Telegramm */
  PVEventX_typ* pPVEvent,   /* ein Variableneintrag -> Rückgabe aus Funktion */
  UINT* puiPvEntry)         /* Index der aktuell zu ermittelnden Variable aus Telegramm */
{
  USINT* pusStartData = NULL;
  PVEventX_typ* pGetData = NULL;

  /* --- auf Start der eigentlichen Daten verweisen: +4(Kennung); +2(Anzahl der Variablen [UINT]) +2 (EventCnt [UINT]) */
  pusStartData = (USINT*) pRecvBufOpen + sizeof(USINT) * 4 + sizeof(UINT) + sizeof(UINT);
  
  if (Diagnose.LogPointerInfo == TRUE)
  {
    ValidatePointerClient((UDINT) pusStartData, 34, (MemManagerDiag_type* ) &sMemDiag, 1);
  }

  if (uiGetMaxPv > *puiPvEntry)
  {
    pGetData = (PVEventX_typ*) pusStartData + (*puiPvEntry);
    brsmemcpy((UDINT)pPVEvent, (UDINT) pGetData, sizeof(PVEventX_typ));
    (*puiPvEntry)++;
    return TRUE;
  } 
  else
  {
    return FALSE;
  }
}


/****************************************************************************************************************
* --- Wert wird einem Datenpunkt zugewiesen
*****************************************************************************************************************/

UINT AssignDataForEvntPv(
  PVEventX_typ sPvEvent,    /* --- ein Variablenpacket vom Server   */
  UINT* puiState)       /* --- Status der Zuweisung       */
{
  PvList_Typ* pPvEvnt = NULL;
  Client_List_Typ* pCliEvnt = NULL;

  USINT* pusPvVal = NULL;
  UINT*  puiPvVal = NULL;
  UDINT* pudPvVal = NULL;
  UINT uiRet = 0;

  /* --- falscher Index f. Variable in Liste (muss zw. 1 u. MAX_VARIABLEN sein) ----- */
  if ((sPvEvent.PVIndex == 0) || (sPvEvent.PVIndex > MAX_VARIABLEN))
  {   
    *puiState = ERR_ILLEGAL_PV_INDEX;
 
    CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Event, &CmdBurLogBook,"AssignDataForEvntPv", "Assign value ", "wrong index", sPvEvent.PVIndex -1, 999, udTimeTick);
    CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
    
    return 2;
  }

  pPvEvnt = pVariablenVerwaltungStart + sPvEvent.PVIndex - 1;
  pCliEvnt = pClientVerwaltungStart + pPvEvnt->Client;

  if (Diagnose.LogPointerInfo == TRUE)
  {
    ValidatePointerClient((UDINT) pPvEvnt, 35, (MemManagerDiag_type* ) &sMemDiag, 1);
    ValidatePointerClient((UDINT) pCliEvnt, 36, (MemManagerDiag_type* ) &sMemDiag, 1);
  }
  
  if (pPvEvnt->pPv == NULL)
  {
    pPvEvnt->Status = ERR_NULL_POINTER_CLNT;
    *puiState = ERR_NULL_POINTER_CLNT;
    
    /* --- Fehler Nullpointer */
    CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Event, &CmdBurLogBook,"AssignDataForEvntPv", "Assign value ", "null pointer", sPvEvent.PVIndex - 1, 999, udTimeTick);
    CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
    
    return 3;
  }

  if ((pPvEvnt->pPvServer != NULL) && 
    (pCliEvnt->LifeCheck == STATE_LIFE_OK))
  {
    /* --- PVs mit neuem Wert beschreiben ---------------------------- */
    if (pPvEvnt->PvLaenge == sPvEvent.Laenge)
    {
      
      CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Event, &CmdBurLogBook,"AssignDataForEvntPv", "Assign value ", "ok", sPvEvent.Laenge, 999, udTimeTick);     
      CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
      
      switch (sPvEvent.Laenge)
      {
        case 1:
          /* BOOL'sche Variablen abprüfen */
          /* Datentyp == BOOL UND Wert <= 1  ODER */
          /* Datentyp != BOOL */
          if (((pPvEvnt->DataType == 1) && (sPvEvent.Wert <= 1)) || (pPvEvnt->DataType != 1))
          {
            pusPvVal  = (USINT*)pPvEvnt->pPv;
            *pusPvVal = sPvEvent.Wert;
            SetPvState((UDINT)pPvEvnt, ERR_NO);
            *puiState = ERR_NO;
          }
          break;
        
        case 2:
          puiPvVal  = (UINT*)pPvEvnt->pPv;
          *puiPvVal   = sPvEvent.Wert;
          SetPvState((UDINT)pPvEvnt, ERR_NO);
          *puiState = ERR_NO;         
          break;
          
        case 4:
          pudPvVal  = (UDINT*)pPvEvnt->pPv;
          *pudPvVal   = sPvEvent.Wert;
          SetPvState((UDINT)pPvEvnt, ERR_NO);
          *puiState = ERR_NO;                                                   
          break;
      }
                  
      pPvEvnt->LetzterWert = sPvEvent.Wert;
      pPvEvnt->LetzterWertCnt++;
      pPvEvnt->LetzterWertServ = pPvEvnt->Client;
      brsstrcpy((UDINT) &pPvEvnt->LetzterWertServIp, (UDINT) &pCliEvnt->IpAdr);
      
      Diagnose.EventCnt.EvntAssignPVData++;
      uiRet = 0;
    }
    else
    {      
      /* --- Fehler Variablenlänge ---------------------------- */
      CmdBurLogBook.FubState = WriteDataToBuRLogger(&sLogger, &CmdBurLogBook.Steps.Event, &CmdBurLogBook,"AssignDataForEvntPv", "Assign value ", "length wrong", sPvEvent.Laenge, 999, udTimeTick);
      CmdBurLogBook.FubStateInc+=CmdBurLogBook.FubState;
      
      pPvEvnt->Status = ERR_VALUE_TO_LONG;
      SetPvState((UDINT) pPvEvnt, ERR_VALUE_TO_LONG);
      
      *puiState = ERR_VALUE_TO_LONG;
      Diagnose.EventCnt.EvntErrAssignData++;
      
      uiRet = 1;
    }/* if (pVariablenManagement->PvLaenge >= PVEvent.Laenge) */
  }
  else
  {
    uiRet = 1;
  }

  return uiRet;
}


/****************************************************************************************************************
* --- Vergleich der IP - Adresse des UDPrcv Fubs mit abgespeicherter IP - Adresse
* --- wenn die IP - Adressen nicht gleich sind, so kann das Telegramm verworfen werden
*****************************************************************************************************************/

BOOL CheckIpAdrForClient(
  PVEventX_typ sPvEvent,    /* --- eine Variable vom Server */
  UDINT pRecvIpAdr)     /* --- Adresse der Ip von Empfangsfub */
{
  DINT strcmp = 0;
  PvList_Typ* pPvEvnt = NULL;
  Client_List_Typ* pCliEvnt = NULL;

  if (sPvEvent.PVIndex == 0)
  {
    return FALSE;   
  }
  
  pPvEvnt = pVariablenVerwaltungStart + sPvEvent.PVIndex - 1;
  pCliEvnt = pClientVerwaltungStart + pPvEvnt->Client;
  
  strcmp = brsstrcmp((UDINT) &pCliEvnt->IpAdr, (UDINT) pRecvIpAdr);
  if (strcmp == 0)
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}


/****************************************************************************************************************
* --- Zurücksetzen der Statistik Pv für Anmeldung von PVs ins Logbuch
*****************************************************************************************************************/

void ResetStatPv(
  ServerConn_typ* pServerConn
  )
{

  pServerConn->uiPvConnToServ = 0;
  pServerConn->uiPvLinked = 0;
}


/****************************************************************************************************************
* --- Zurücksetzen der Statistik Pv für Anmeldung von PVs ins Logbuch
*****************************************************************************************************************/

void ResetConnPv(
  PvConnect_Typ* sPvConnect)
{
  sPvConnect->uiTry = 0;
  sPvConnect->uiOk = 0;
  sPvConnect->uiFailed = 0;
}


/****************************************************************************************************************
* --- Schreiben der Statistikinformationen für Anmeldung von PVs ins Logbuch
*****************************************************************************************************************/

void WriteConnPvInLogbook(
  PvConnect_Typ sPvConnect,   /* */
  Client_List_Typ* pActClient,  /* Pointer auf Verwaltung */
  UINT uiActServ,         /* Index des aktuell zu bearbeitenden Servers */
  BOOL bEnable
  )
{
  USINT usTemp[80];
  USINT usTemp1[6];

  if (bEnable == TRUE)
  {
    return;
  }
  
  //Versuch: alle als gelungen gezeichneten Einträge werden ins Logbuch geschrieben
  /* falls keine einzige PV angemeldet werden konnte: Standardfehlertext */
//  if ((sPvConnect.uiOk == 0) && (sPvConnect.uiFailed == 0))
//  {    
//    return;
//  }
  
  usTemp[0] = 0;
  usTemp1[0] = 0;
  brsstrcpy((UDINT) &usTemp, (UDINT) "client:IpAdr.: ");
  brsstrcat((UDINT) &usTemp, (UDINT) &pActClient->IpAdr);    
  brsstrcat((UDINT) &usTemp, (UDINT) " (PV-Ok: ");
  brsitoa((DINT) sPvConnect.uiOk, (UDINT) &usTemp1);
  brsstrcat((UDINT) &usTemp, (UDINT) &usTemp1); 
  brsstrcat((UDINT) &usTemp, (UDINT) ";  PV-Err: "); 
  usTemp1[0] = 0;
  brsitoa((DINT) sPvConnect.uiFailed, (UDINT) &usTemp1);
  brsstrcat((UDINT) &usTemp, (UDINT) &usTemp1);
  brsstrcat((UDINT) &usTemp, (UDINT) " )");
  WriteLoggerMessage((Logger_typ*) &sLogger, 0xF000, (STRING*) &usTemp, 0x7FFFFFFF, arEVENTLOG_SEVERITY_INFO);
  
  usTemp[0] = 0;
  usTemp1[0] = 0;
  brsstrcpy((UDINT) &usTemp, (UDINT) "client: Anmeldung - Station Nr. ");
  brsitoa((DINT) uiActServ, (UDINT) &usTemp1);
  brsstrcat((UDINT) &usTemp, (UDINT) &usTemp1);
  
  /* Falls über Host konfiguriert wurde, wird noch der Name ins Logbuch geschrieben */
  if (pActClient->ConfByHost == 1)
  {
    brsstrcat((UDINT) &usTemp, (UDINT) "; Hostname: ");
    brsstrcat((UDINT) &usTemp, (UDINT) pActClient->HostName);
  }
  
  WriteLoggerMessage((Logger_typ*) &sLogger, 0xF000, (STRING*) &usTemp, 0x7FFFFFFF, arEVENTLOG_SEVERITY_INFO);
}


/*****************************************************************************************************************
* SetPtrToNull: alle Pointer auf NULL setzen, da Datobj weg!
*****************************************************************************************************************/

void SetPtrToNull()
{
  pVariablenManagement  = NULL;
  pClientManagement   = NULL;
  pVariablenEvent     = NULL;
  pClientEvent      = NULL;
  pClientLife       = NULL;
  pClientGetIpAdr   = NULL;

  pSendBufOpen = NULL;
  pRecvBufOpen = NULL;
  pSendBufEvnt = NULL;
  pRecvBufEvnt = NULL;
  pPVInit = NULL;
}

/*****************************************************************************************************************
* CreateDataObjMem: Datenobjekt erzeugen (ausschließlich im INIT verwenden!)
*****************************************************************************************************************/

UINT CreateDataObjMem(
  MemManager_type * pMemManger,
  DatObjCreate_typ  *pDataObjCreateFub,
  DatObjInfo_typ    *pDatObjInfoFub,
  DatObjDelete_typ  *pDatObjDeleteFub
  )
{
  BOOL bCreate = FALSE;
  
  /* --- Info über Modul anfordern --------------------------------- */
  pDatObjInfoFub->enable = TRUE;
  pDatObjInfoFub->pName  = (UDINT) DATA_OBJ_MEM_NAME_CLIENT;
    
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
        }
        else
        {
          return pDatObjDeleteFub->status;
        }
        WriteLoggerMessage((Logger_typ*) &sLogger, 0xF000, (STRING*) "client:DM gelöscht", 0x7FFFFFFF, arEVENTLOG_SEVERITY_INFO);
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
      pDataObjCreateFub->pName  = (UDINT) DATA_OBJ_MEM_NAME_CLIENT;   
      pDataObjCreateFub->MemType  = doTEMP;
      pDataObjCreateFub->Option   = 1;
      pDataObjCreateFub->pCpyData = 0;
    
      DatObjCreate(pDataObjCreateFub);
    } while (pDataObjCreateFub->status == 0xFFFF);
    
    if (pDataObjCreateFub->status == 0)
    {
      pMemManger->udDOPtr = pDataObjCreateFub->pDatObjMem;
      pMemManger->udIdent = pDataObjCreateFub->ident;
    }
    WriteLoggerMessage((Logger_typ*) &sLogger, 0xF000, (STRING*) "client:DM erzeugt", 0x7FFFFFFF, arEVENTLOG_SEVERITY_INFO);
  }
  
  
  return pDataObjCreateFub->status;
}

/*****************************************************************************************************************
* ValidatePointerClient: Überprüfung - nur als Debugfunktion für V 3.07.x 
*****************************************************************************************************************/

UDINT ValidatePointerClient(
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
 
  if (udAdr == NULL)
  {
    bNullPointer = TRUE;
    bErr = TRUE;
  }
  else if ((udAdr > NULL) && (udAdr <= 0x3FFF))
  {
    bSysPointer = TRUE;
    bErr = TRUE;
  }
  else
  {
    /* Basisprüfung überstanden: weiter */
    
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
    /* "client#1 Pointer im falschen Bereich " */
    /* "client#2 Adresse - 938204402" */
    /* "client#3 Start: 120230: Stop: 34093402 */
    
    if (bNullPointer == TRUE)
    {
      WriteLoggerMessage((Logger_typ*) &sLogger, 0xF000, (STRING*) "client#1: Pointer auf NULL", 0x7FFFFFFF, arEVENTLOG_SEVERITY_INFO);
    }
    else if (bSysPointer == TRUE)
    {
      WriteLoggerMessage((Logger_typ*) &sLogger, 0xF000, (STRING*) "client#1: Pointer im System Bereich", 0x7FFFFFFF, arEVENTLOG_SEVERITY_INFO);
    }
    else if (bErrPointer == TRUE)
    {
      WriteLoggerMessage((Logger_typ*) &sLogger, 0xF000, (STRING*) "client#1: Pointer im falschen Bereich", 0x7FFFFFFF, arEVENTLOG_SEVERITY_INFO);
    }
  	
    Messagestring2[0] = 0;
    brsitoa(udAdr, (UDINT) &Messagestring2);

    Messagestring1[0] = 0;
    brsstrcpy((UDINT) &Messagestring1, (UDINT) "client#2: Adresse - ");
    brsstrcat((UDINT) &Messagestring1, (UDINT) &Messagestring2);
    WriteLoggerMessage((Logger_typ*) &sLogger, 0xF000, (STRING*) &Messagestring1, 0x7FFFFFFF, arEVENTLOG_SEVERITY_INFO);
        
    Messagestring1[0] = 0;
    Messagestring2[0] = 0;
    brsstrcpy((UDINT) &Messagestring1, (UDINT) "client#3: Start: ");
    brsitoa((DINT) pMemDiag->udStartDM, (UDINT) &Messagestring2);
    brsstrcat((UDINT) &Messagestring1, (UDINT) &Messagestring2);
    brsstrcat((UDINT) &Messagestring1, (UDINT) " ;Ende: ");
    Messagestring2[0] = 0;
    brsitoa((DINT) pMemDiag->udStopDM, (UDINT) &Messagestring2);
    brsstrcat((UDINT) &Messagestring1, (UDINT) &Messagestring2);
    WriteLoggerMessage((Logger_typ*) &sLogger, 0xF000, (STRING*) &Messagestring1, 0x7FFFFFFF, arEVENTLOG_SEVERITY_INFO);
    return NULL;
  }

  return udAdr;
}


void AddErrInfoEventX(
  UINT uiPVIndex,
  UDINT* pStrMEssage
  )
{
  USINT  Messagestring1[64];

  Messagestring1[0] = 0;
  brsstrcpy((UDINT) &Messagestring1, (UDINT) "client: PVIndex err - ");
  brsstrcat((UDINT) &Messagestring1, (UDINT) pStrMEssage);
  
  WriteLoggerMessage((Logger_typ*) &sLogger, 0xF000, (STRING*) &Messagestring1, 0x7FFFFFFF, arEVENTLOG_SEVERITY_WARNING);
}


/*****************************************************************************************************************
* Zuweisung der Daten für Anbindung an Visu
*****************************************************************************************************************/

void AssignMonitorDataInit(
  Client_List_Typ* pClientStart,  /* Pointer auf Verwaltung der Server*/
  ServerConn_typ* pServerConn,
  PvList_Typ* pVariableStart
  )
{

  UINT i = 0;
  UINT cnt = 0;
  Client_List_Typ* pServer;
  ServerConn_typ* pActServer;
  PvList_Typ* pActVariable;

  /* --- Alle Server auf Fehler setzen ---------- */
  for (i = 0; i < MAX_SERVER; i++)
    {
    pServer = pClientStart + i;
    pActServer = pServerConn + i;
    if (pServer->IPAdresse != 0)
    {
      pActServer->Info.uiNr = i;
      brsstrcpy((UDINT) &pActServer->Info.Hostname, (UDINT) &pServer->HostName);
      brsstrcpy((UDINT) &pActServer->Info.IpAdr, (UDINT) &pServer->IpAdr);
      
      pActServer->uiPvConf = 0;
      for (cnt = 0; cnt < MAX_VARIABLEN; cnt++)
        {
        pActVariable = pVariableStart + cnt;
        	
        if (pActVariable->pPv != NULL)
        {          
          if (pActVariable->Client == i)
          {
            pActServer->uiPvConf++;
          }
        } /* if (pVariablenbelegung->pPv > 0) */
        
        if (pActVariable->NameClient[0] != 0)
        {
          if (pActVariable->Client == i)
          {
            pActServer->uiPvConfFound++;
          }
        }
        
      } /* for (cnt = 0; cnt < MAX_VARIABLEN; cnt++) */
    }
    
  }

}


/* einmaliger Eintrag in das ptop Logbuch mit 4 einzelnen Texten welche mit '||' zusammengesetzt werden */
DINT WriteDiagDataToBuRLoggerOneTime(
  BOOL* pEntered ,    /* is it possible to enter the message to the logbook ?*/
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
  
  if (*pEntered == TRUE) /* was entered into the logbook*/
  {    
    return 0;    
  }
  *pEntered = TRUE;
  
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
  iRetVal = WriteLoggerMessage((Logger_typ*) &sLogger, 0xF000, (STRING*) &HelpString1, 0x7FFFFFFF, arEVENTLOG_SEVERITY_INFO);
  
  return iRetVal;
}


/* Funktion zur Protokollierung von Schritten:
1. Schritte vollständig protokollieren		Func: CyclicEvents || EVENT_SCHRITT_INIT
2. Datenverteilung (Eventbetrieb)
*/
UINT WriteStepDataToBuRLogger(
  Logger_typ* pLogger,	/* Main Part*/
  ClntBurLogCtrlStepsInt_typ* pDetailLogger,
  ClntBurLog_typ* pLogManager,
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
      brsstrcpy((UDINT) &HelpString1, (UDINT) &START_LOG_TXT);
      brsstrcat((UDINT) &HelpString1, (UDINT) &pLogManager->Title);
      pLogger->Fubs.ArEventLogWrite_Fub.AddData = (UDINT) &HelpString1;
      iStrLen = brsstrlen((UDINT) &HelpString1);
   
      pLogger->Fubs.ArEventLogWrite_Fub.AddDataSize = iStrLen + 1;
      pLogger->Fubs.ArEventLogWrite_Fub.Execute = TRUE;
      pLogger->Fubs.ArEventLogWrite_Fub.Ident = pLogger->Ident;
      pLogger->Fubs.ArEventLogWrite_Fub.TimeStamp = 0;
      pLogger->Fubs.ArEventLogWrite_Fub.AddDataFormat = 1;// arEVENTLOG_ADDFORMAT_TEXT;  
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
  iStrLen = brsstrlen((UDINT) &HelpString1);
   
  pLogger->Fubs.ArEventLogWrite_Fub.AddDataSize = iStrLen + 1;
  pLogger->Fubs.ArEventLogWrite_Fub.Execute = TRUE;
  pLogger->Fubs.ArEventLogWrite_Fub.Ident = pLogger->Ident;
  pLogger->Fubs.ArEventLogWrite_Fub.TimeStamp = 0;
  pLogger->Fubs.ArEventLogWrite_Fub.AddDataFormat = 1; //arEVENTLOG_ADDFORMAT_TEXT;  
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
  ClntBurLogCtrlStepsInt_typ* pDetailLogger,
  ClntBurLog_typ* pLogManager,
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
    brsstrcat((UDINT) &HelpString1,(UDINT)  &HelpString2);
    brsstrcat((UDINT) &HelpString1,(UDINT) ")");	
  }
	
  pLogger->Fubs.ArEventLogWrite_Fub.AddData = (UDINT) &HelpString1;
  iStrLen = brsstrlen((UDINT) HelpString1);
   
  pLogger->Fubs.ArEventLogWrite_Fub.AddDataSize = iStrLen + 1;
  pLogger->Fubs.ArEventLogWrite_Fub.Execute = TRUE;
  pLogger->Fubs.ArEventLogWrite_Fub.Ident = pLogger->Ident;
  pLogger->Fubs.ArEventLogWrite_Fub.TimeStamp = 0;
  pLogger->Fubs.ArEventLogWrite_Fub.AddDataFormat = 1; // arEVENTLOG_ADDFORMAT_TEXT;  
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


UDINT RemoveCompilerWarning(UDINT Param)
{
DINT iFor=0;
  
  iFor+= Param;
  
  return iFor;
}


/* search through the variable list and get data for up to 10 variables with some problems */
BOOL GetClientPvListProblems(
  UDINT* pPvStartAdr,		/* Start of Varible list */
  UDINT* pClientStartAdr, /* Start of Client list */
  PvClntListMain_typ* pPvListMain /* Variable Result List*/
  )
{
  PvList_Typ* pPv;
  UINT i;
  UINT iPvResult = 0;
	
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
  for (i = 0, pPv = (PvList_Typ*)pPvStartAdr; i < MAX_VARIABLEN; i++, pPv++)
  {
    if (pPv->NameClient[0] != 0)	// something is configured
    {
      if ((pPv->pPv == NULL) || (pPv->pPvServer == NULL) || (pPv->PvLaenge == 0)) 
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
			
        if (pPv->pPvServer != NULL)
        {
          pPvListMain->Result[iPvResult].Info.pPvServer = PV_STATUS_OK;
        }
        else
        {
          pPvListMain->Result[iPvResult].Info.pPvServer = PV_STATUS_ERR;
        }
			
        if (pPv->PvLaenge != 0)
        {
          pPvListMain->Result[iPvResult].Info.PvLaenge = PV_STATUS_OK;
        }
        else
        {
          pPvListMain->Result[iPvResult].Info.PvLaenge = PV_STATUS_ERR;
        }
        pPvListMain->Result[iPvResult].Info.Status = pPv->Status;
			
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
