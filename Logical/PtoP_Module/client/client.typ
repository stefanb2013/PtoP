(*- - - datatypes für allgclnt*)

TYPE
	DiagClnt_Open_typ : 	STRUCT 
		Open : UINT;
		SendConn : UINT;
		RetryConn : UINT;
		ConnResp : UINT;
		ErrCnt : UINT;
		GetIpAdr : UINT;
		CheckPvBreak : UINT;
		SendOpenReq : UINT;
		SendOpenResp : UINT;
		SendOpenReqFailure : UINT;
		RecvOpen : UINT;
		RecvOpenX : UINT;
		RecvOpenSomething : UINT;
		SendLifeCheckCount : UINT;
		ReceiveLifeCheckCount : UINT;
		LostLifeCheckCount : UINT;
		WrongStationCount : UINT;
		EventLifeFailed : UINT;
		LinkRequest : UINT;
		LinkResponse : UINT;
		WrongPvIndex : UINT;
		EvntReceiveData : UINT;
		EvntGetPVData : UINT;
		EvntAssignPVData : UINT;
		EvntErrAssignData : UINT;
		GetHostNameIpChanged : UINT;
		GetHostNameErr : UINT;
		TriggerReconnErr : UINT;
		TriggerReconnManual : UINT;
		TriggerReconnExtern : UINT;
		EvntResp : UINT;
		LifeCheckSend : UINT;
		LifeCheckSendFailure : UINT;
	END_STRUCT;
	GetIpAdrSource_type : 	STRUCT 
		bEnable : BOOL;
		usStep : USINT;
		CfgGetIpAddrFub : CfgGetIPAddr;
		arIpAddressString : ARRAY[0..15]OF USINT;
	END_STRUCT;
	MemManager_type : 	STRUCT 
		udMemory : UDINT;
		udDOPtr : UDINT;
		udIdent : UDINT;
		sPVSpeicherPlatz : MemStrct_type;
		sClientSpeicherPlatz : MemStrct_type;
		sSendBufOpen : MemStrct_type; (*Buffer für Telegramm "OPEX"					*)
		sRecvBufOpen : MemStrct_type; (*Buffer für Telegramm "OPEX"					*)
		sSendBufEvnt : MemStrct_type; (*Buffer für Telegramm "EVNX"					*)
		sRecvBufEvnt : MemStrct_type; (*Buffer für Telegramm "EVNX"					*)
	END_STRUCT;
	MemStrct_type : 	STRUCT 
		udLen : UDINT;
		udAdr : UDINT;
	END_STRUCT;
END_TYPE

(*datatypes for errclnt*)

TYPE
	Err_List_Typ : 	STRUCT 
		DateTime : DATE_AND_TIME;
		SystemTick : UDINT; (* Systemtick									*)
		MainStep : USINT; (* Schrittschaltwerk							*)
		Step : USINT; (* Schrittschaltwerk Unterschritt				*)
		ActError : USINT; (* Markiert den letzten Fehler in Buffer		*)
		PVIndex : UINT; (* Referenzierung auf PV in Verwaltungsstruktur	*)
		TimeMilliSecond : UINT; (* Zeit - Milisekunde 							*)
		Status : UINT; (* Fehlernummer									*)
		IpAdr : ARRAY[0..15]OF USINT; (* IP Adresse des UDP Funktionsblockes 	 		*)
		PortNumber : UINT;
	END_STRUCT;
END_TYPE

(*datatypes for ptop pars*)

TYPE
	debug_typ : 	STRUCT 
		AnzahlKeys : UINT;
	END_STRUCT;
	ReadUST_typ : 	STRUCT 
		Hysterese : UINT;
		InitWert : UINT;
		Remanenz : UINT;
		Ersatzwert : UINT;
		SyncTime : UINT;
	END_STRUCT;
	ReadVar_typ : 	STRUCT 
		NameServer : ARRAY[0..PV_LENGTH_ARR]OF USINT;
		NameClient : ARRAY[0..PV_LENGTH_ARR]OF USINT;
		Hysterese : UINT;
		InitWert : UINT;
		Remanenz : UINT;
		Ersatzwert : UINT;
		SyncTime : UINT; (*Synchronisierungszeit je PV*)
	END_STRUCT;
END_TYPE

(*datatypes for ptop clnt*)

TYPE
	Verwaltung_typ : 	STRUCT  (*Verwaltungsstruktur für Anmeldevorgang*)
		ClientNumber : UINT;
	END_STRUCT;
	Lifecheck_typ : 	STRUCT  (*Lifecheck*)
		Server : UINT; (*an welchen Server wird als nächstes Lifecheck gesendet*)
		AufrufeFub : UINT; (*Anzahl, wie oft Fub aufgerufen wurde*)
		TelegrammeCnt : UINT;
	END_STRUCT;
	MonitorClient_typ : 	STRUCT 
		Lock : UINT;
		PVIndex : UINT;
		ClientIndex : UINT;
	END_STRUCT;
	PvConnect_Typ : 	STRUCT 
		uiTry : UINT;
		uiOk : UINT;
		uiFailed : UINT;
	END_STRUCT;
	visu_type : 	STRUCT 
		uiNr : UINT;
		Hostname : ARRAY[0..31]OF USINT;
		IpAdr : ARRAY[0..31]OF USINT;
	END_STRUCT;
	AlarmList_type : 	STRUCT 
		HostnameUnresolved : BOOL;
		PvNotConnected : BOOL;
	END_STRUCT;
	alarm_type : 	STRUCT 
		AlarmActive : BOOL;
		AlarmList : AlarmList_type;
	END_STRUCT;
	Test_typ : 	STRUCT 
		OpenSet : ARRAY[0..MAX_VARIABLEN_ARR]OF USINT;
		Open : ARRAY[0..MAX_VARIABLEN_ARR]OF USINT;
		OpenReceiveSet : ARRAY[0..MAX_VARIABLEN_ARR]OF USINT;
		OpenReceive : ARRAY[0..MAX_VARIABLEN_ARR]OF USINT;
		ErrConnect : BOOL;
		ErrLink : BOOL;
	END_STRUCT;
	ServerConn_typ : 	STRUCT 
		bUsed : BOOL; (*--- Station parametriert*)
		uiPvConf : UINT; (*--- Anzahl Variablen, welche konfiguriert wurden*)
		uiPvConfFound : UINT; (*--- Anzahl Variablen, welche konfiguriert und am client gefunden wurden*)
		uiPvConnToServ : UINT; (*--- Anzahl Variablen, welche angemeldet wurden*)
		uiPvLinked : UINT; (*--- Anzahl Variablen, welche tatsächlich bei Server erfolgreich angemeldet wurden*)
		bCmdReConn : BOOL; (*--- Befehl, um sich an Server erneut anzumelden	*)
		Info : visu_type; (*--- zusätzliche Infos*)
		Alarm : alarm_type; (*--- Alarmliste*)
	END_STRUCT;
END_TYPE

(*- - - Diagnose*)

TYPE
	PvClntListMain_typ : 	STRUCT 
		Start : BOOL;
		Result : ARRAY[0..DIAG_PV_RESULT_LIST]OF PvClntResultDetail_typ;
	END_STRUCT;
	DiagnoseClntMain_typ : 	STRUCT 
		LogPointerInfo : BOOL;
		PvList : PvClntListMain_typ;
		EventCnt : DiagClnt_Open_typ;
	END_STRUCT;
	PvClntResultDetail_typ : 	STRUCT 
		PvIndex : UINT;
		Info : PvClntInfo_typ;
	END_STRUCT;
	PvClntInfo_typ : 	STRUCT 
		Status : UINT;
		pPv : PvDiagStatus_enum;
		pPvServer : PvDiagStatus_enum;
		PvLaenge : PvDiagStatus_enum;
	END_STRUCT;
END_TYPE
