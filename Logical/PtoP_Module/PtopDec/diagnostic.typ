(*****************************************************************
  File: Global_diagnostic.typ
  Date: 2018.07.12 10:28
*****************************************************************)

TYPE
	MemManagerDiag_type : 	STRUCT 
		udStartDM : UDINT; (*Beginn des Datenobjektes f�r Datenverwaltung*)
		udStopDM : UDINT; (*Ende des Datenobjektes f�r Datenverwaltung*)
	END_STRUCT;
	TelegrammDiag_typ : 	STRUCT 
		arIpAdr : ARRAY[0..15]OF USINT; (*von welcher Station*)
		usBuffer : ARRAY[0..99]OF USINT; (*Daten*)
	END_STRUCT;
END_TYPE

(*- - - Manipulate the B&R Logger*)

TYPE
	Logger_Create_enum : 
		(
		stLogger_Create_Idle,
		stLogger_Create_Ident,
		stLogger_Create_Create,
		stLogger_Create_Finished,
		stLogger_Create_Err
		);
	Logger_Event_typ : 	STRUCT 
		Severity : USINT; (*Severity of the last event (read).*)
		Customer : BOOL; (*Customer bit of the last event (read).*)
		Facility : UINT; (*Facility of the last event (read).*)
		Code : UINT; (*Code of the last event (read).*)
		EventID : DINT; (*EventID for write event (write - is not set by reading an event).*)
		ErrorNumber : UDINT; (*Error number of the event - set when the read event is no event id event (read).*)
		EnteredBy : STRING[36]; (*User/object identification (read/write - optional).*)
		OriginID : ArEventLogRecordIDType; (*Record id of the origin record (read/write - optional).*)
		RecordID : ArEventLogRecordIDType; (*Record id of the event to read (read - set automatically).*)
		TimeStamp : ArEventLogTimeStampType;
		AdditionalDataSize : UDINT; (*Size of the additional data (read/write - optional)*)
		AdditionalDataFormat : USINT; (*Format of the additional data (read/write)*)
		AdditionalData : ARRAY[0..128]OF USINT; (*Additional data (read/write)*)
	END_STRUCT;
	Logger_Fub_typ : 	STRUCT 
		ArEventLogCreate_Fub : ArEventLogCreate;
		ArEventLogGetIdent_Fub : ArEventLogGetIdent;
		ArEventLogWrite_Fub : ArEventLogWrite;
	END_STRUCT;
	Logger_Steps_typ : 	STRUCT 
		Create : Logger_Create_enum;
	END_STRUCT;
	Logger_Diag_typ : 	STRUCT 
		CreateLogger : UDINT;
		CreateEntry : UDINT;
	END_STRUCT;
	Logger_typ : 	STRUCT 
		IsActive : BOOL;
		Ident : ArEventLogIdentType; (*ident of the logger*)
		LastError : DINT;
		Fubs : Logger_Fub_typ; (*fubs*)
		Event : Logger_Event_typ; (*ein event fuer das Logbuch*)
		Steps : Logger_Steps_typ;
		Diag : Logger_Diag_typ;
		iStatus : DINT;
	END_STRUCT;
END_TYPE

(*- - - Logbook entries*)

TYPE
	BurLog_Measure_enum : 
		(
		LOG_ENTRIES,
		LOG_TIME,
		LOG_INFINITE
		);
	PvDiagStatus_enum : 
		(
		PV_STATUS_OK,
		PV_STATUS_ERR
		);
	CmdBurLogBookOnetime_typ : 	STRUCT 
		Hostname : BOOL;
		HostnameInfo : BOOL;
	END_STRUCT;
	CmdBurLogBook_typ : 	STRUCT 
		Conn : BOOL;
		ConnErr : BOOL;
		Event : BOOL;
		EventErr : BOOL;
		FubState : DINT;
		FubStateInc : UDINT;
		OneTime : CmdBurLogBookOnetime_typ;
	END_STRUCT;
END_TYPE

(*- - - Client logger types*)

TYPE
	ClntBurLogCtrlInt_typ : 	STRUCT 
		LogEntries : UINT;
		ActTime : UDINT;
		StartLogging : BOOL;
	END_STRUCT;
	ClntBurLogCtrlSteps_typ : 	STRUCT 
		Connect : ClntBurLogCtrlStepsInt_typ;
		Event : ClntBurLogCtrlStepsInt_typ;
		Lifecheck : ClntBurLogCtrlStepsInt_typ;
		HostByName : ClntBurLogCtrlStepsInt_typ;
	END_STRUCT;
	ClntBurLogSettings_typ : 	STRUCT 
		MeasurementType : BurLog_Measure_enum;
		NrOfLogEntries : UINT;
		MeasurementTime : TIME;
	END_STRUCT;
	ClntBurLogCtrlStepsInt_typ : 	STRUCT 
		Checked : BOOL;
		LastStepInternal : DINT;
	END_STRUCT;
	ClntBurLog_typ : 	STRUCT 
		StartLogging : BOOL;
		Title : STRING[40];
		Steps : ClntBurLogCtrlSteps_typ;
		Settings : ClntBurLogSettings_typ;
		Internal : ClntBurLogCtrlInt_typ;
		FubState : UINT;
		FubStateInc : UDINT;
	END_STRUCT;
END_TYPE

(*- - - Server logger types*)

TYPE
	ServBurLogCtrlInt_typ : 	STRUCT 
		LogEntries : UINT;
		ActTime : UDINT;
		StartLogging : BOOL;
	END_STRUCT;
	ServBurLogCtrlStepsInt_typ : 	STRUCT 
		Checked : BOOL;
		LastStepInternal : DINT; (*Internal Value of last tracked step*)
	END_STRUCT;
	ServBurLogCtrlSteps_typ : 	STRUCT 
		Connect : ServBurLogCtrlStepsInt_typ;
		EventSend : ServBurLogCtrlStepsInt_typ;
		EventRecv : ServBurLogCtrlStepsInt_typ;
		Lifecheck : ServBurLogCtrlStepsInt_typ;
	END_STRUCT;
	ServBurLogSettings_typ : 	STRUCT 
		MeasurementType : BurLog_Measure_enum;
		NrOfLogEntries : UINT; (*/* complete amount of logentries*/*)
		MeasurementTime : TIME; (*/* timespan for logging [ms] */*)
	END_STRUCT;
	ServBurLog_typ : 	STRUCT 
		StartLogging : BOOL;
		Title : STRING[40];
		Steps : ServBurLogCtrlSteps_typ;
		Settings : ServBurLogSettings_typ;
		Internal : ServBurLogCtrlInt_typ;
		FubState : UINT;
		FubStateInc : UDINT;
	END_STRUCT;
END_TYPE
