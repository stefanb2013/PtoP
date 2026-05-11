(********************************************************************
 * COPYRIGHT -- Bernecker+Rainer
 ********************************************************************
 * Program: server
 * File: server.typ
 * Author: B&R
 * Created: July 13, 2018
 ********************************************************************
 * Local data types of program server
 ********************************************************************)
(*- - - config.h*)

TYPE
	PtopConf_Typ : 	STRUCT 
		uiPipeLen : UINT; (*Pipegroesse: so viele Werte können zwischengelagert werden*)
		uiPvPerCycle : UINT; (*Anzahl der bearbeiteten Variablen pro Zyklus*)
		uiPvStored : UINT; (*Anzahl der zu verwaltenden Variablen*)
		uiClientStored : UINT; (*Anzahl der zu verwaltenden Clients*)
	END_STRUCT;
END_TYPE

(*= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = *)
(*- - - ptopserv.h*)
(*= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = *)
(*- - - Sendeabbild für Eventauswertung	*)

TYPE
	EventSend_typ : 	STRUCT 
		PipeCnt : UINT;
		ActPipe : UINT;
		AbbildCnt : UINT; (*aktuelle Position des Sendezeigers*)
		RetryCnt : UINT;
		Telegramm : UINT;
		TimeCnt : UINT;
	END_STRUCT;
END_TYPE

(*- - - Diagnosestrukutr - - - - - - - - - - -*)

TYPE
	DIAG_Event_typ : 	STRUCT 
		WriteToPipe : UINT;
		TryWriteIntoPipe : UINT;
		WriteToPipeFailed : UINT;
		ReadFromPipe : UINT;
		TryToReadFromPipe : UINT;
		WriteToEvent : UINT;
		SendTelegrammCmd : UINT;
		SendTelegrammOk : UINT;
		ReceiveTelegramm : UINT;
		ReceiveAckFromClient : UINT;
		FailedTelegramm : UINT;
		NoData : UINT;
		ClientFailed : UINT;
		SearchPv : UINT;
		SearchClient : UINT;
	END_STRUCT;
	DIAG_REGISTER_typ : 	STRUCT 
		OpenSend : UINT;
		OpenSendFailure : UINT;
		OpenRequest : UINT;
		OpenRequestOk : UINT;
		OpenRequestCRCFailure : UINT;
		OpenRequestIpFailure : UINT;
		OpenResponse : UINT;
		DelClient : UINT;
		DelPV : UINT;
		ConnRequest : UINT;
		ConnResponse : UINT;
		ConnSend : UINT;
		ConnSendFailure : UINT;
		Link : UINT;
	END_STRUCT;
	Diag_Life_typ : 	STRUCT 
		TelegrammCnt : UINT;
		TelegrammSend : UINT;
		IpAddr : UINT;
		TelegrOk : UINT;
	END_STRUCT;
	Diag_typ : 	STRUCT 
		Events : DIAG_Event_typ;
		PvCheck : ARRAY[0..MAX_CLIENT_VARIABLEN]OF DiagPvCheck_typ;
		PipeRead : ARRAY[0..MAX_ANZAHL_CLIENTS_ARR]OF UINT;
		bCmdGetPipeOccupied : BOOL;
		PipeOccupied : ARRAY[0..MAX_ANZAHL_CLIENTS_ARR]OF UINT;
		SendDataToClient : ARRAY[0..MAX_ANZAHL_CLIENTS_ARR]OF UINT;
		RegisterPv : DIAG_REGISTER_typ;
		RegisterPvX : DIAG_REGISTER_typ;
		MaxVariables : UINT;
		MaxClients : UINT;
		LifeTelegrCnt : UINT;
		LifeTelegrOk : UINT;
		LifeSend : UINT;
		LifeCheck : Diag_Life_typ;
	END_STRUCT;
END_TYPE

(*= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = *)
(*- - - allgserv.h*)
(*= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = *)

TYPE
	MemStrctServ_type : 	STRUCT 
		udLen : UDINT;
		udAdr : UDINT;
	END_STRUCT;
	MemManagerServ_type : 	STRUCT 
		udMemory : UDINT;
		udDOPtr : UDINT;
		udIdent : UDINT;
		sPVListeSpeicherPlatz : MemStrctServ_type;
		sClientListeSpeicherPlatz : MemStrctServ_type;
		sPipeSpeicherPlatz : MemStrctServ_type;
		sSendBufOpen : MemStrctServ_type; (*Buffer fuer Telegramm "OPEX"					*)
		sRecvBufOpen : MemStrctServ_type; (*Buffer fuer Telegramm "OPEX"					*)
		sSendBufEvnt : MemStrctServ_type; (*Buffer fuer Telegramm "EVNX"					*)
		sRecvBufEvnt : MemStrctServ_type; (*Buffer fuer Telegramm "EVNX"					*)
	END_STRUCT;
	DiagPvCheck_typ : 	STRUCT 
		uiCheckPv : UINT; (*Variable auf Wertänderung überpruefen*)
		uiCheckPvOk : UINT; (*Wert der Variable hat sich tatsächlich geaendert*)
		uiCheckPvTimeOk : UINT; (*Wert der Variable wird zyklisch gepollt -> Wert sichern*)
		uiPipeLocked : UINT; (*Eintrag in Pipe nicht möglich*)
		uiReqPipeEntry : UINT; (*Wert der Variable für Eintrag in Pipe ok*)
		uiWriteToPipe : UINT; (*Wert wird in die Pipe eingetragen*)
		TryToReadFromPipe : UINT; (* Wert aus Pipe holen*)
	END_STRUCT;
END_TYPE

(*- - - 	Monitorstruktur für Client u. Variablenliste*)

TYPE
	EventAbbildMonitor_Typ : 	STRUCT 
		Lock : UINT;
		EventPlace : UINT;
	END_STRUCT;
	MonitorServer_typ : 	STRUCT 
		Lock : UINT;
		PVIndex : UINT;
		ClientIndex : UINT;
	END_STRUCT;
END_TYPE

(*- - - Struktur zur Eventauswertung*)

TYPE
	Event_typ : 	STRUCT 
		ActClient : UINT;
		ActPV : UINT;
		CntPV : UINT;
		Wert : UDINT;
		DiffWert : UDINT;
		PipeNr : UINT; (*welche Pipe wird mit Daten gefüllt*)
	END_STRUCT;
END_TYPE

(*= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = *)
(*- - - allgserv.h*)
(*= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = *)
(*- - - Diagnose*)

TYPE
	PvServInfo_typ : 	STRUCT 
		pPv : PvDiagStatus_enum;
		pPvServer : PvDiagStatus_enum;
		PvLaenge : PvDiagStatus_enum;
	END_STRUCT;
	PvServResultDetail_typ : 	STRUCT 
		PvIndex : UINT;
		Info : PvServInfo_typ;
	END_STRUCT;
	PvServListMain_typ : 	STRUCT 
		Start : BOOL;
		Result : ARRAY[0..DIAG_PV_RESULT_LIST]OF PvServResultDetail_typ;
	END_STRUCT;
	DiagnoseServMain_typ : 	STRUCT 
		LogPointerInfo : BOOL; (*mitloggen von Pointerabfragen usw.: Defaultwert = false*)
		PvList : PvServListMain_typ; (*Variable List Checker*)
		Base : Diag_typ; (* bereits vorhandene Diagnose 1:1*)
	END_STRUCT;
END_TYPE
