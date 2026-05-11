(********************************************************************
 * COPYRIGHT -- Bernecker + Rainer
 ********************************************************************
 * Library: PtopLib
 * File: PtopLib.typ
 * Author: B&Rt
 * Created: February 26, 2014
 ********************************************************************
 * Data types of library PtopLib
 ********************************************************************)

TYPE
	FubCaller_typ : 	STRUCT 
		FileCreate_Fub : FileCreate;
		FileDelete_Fub : FileDelete;
		FileWrite_Fub : FileWrite;
		FileClose_Fub : FileClose;
		DtGetTime_Fub : DTGetTime;
	END_STRUCT;
	Err_Info_typ : 	STRUCT 
		status : UINT;
		step : UINT;
		bErr : BOOL;
	END_STRUCT;
	GetClients_Info_typ : 	STRUCT 
		szFileName : STRING[32];
		sErr : Err_Info_typ;
	END_STRUCT;
	Server_Data_typ : 	STRUCT 
		udAdrServer : UDINT; (*Start - Adresse der Server Verwaltung*)
		iServerLen : UINT;
		udAdrPvListe : UDINT;
		iPvListeLen : UINT;
	END_STRUCT;
	GetClients_Intern_typ : 	STRUCT 
		iStep : INT; (*Schrittvariable für Ablauf*)
		sFubCaller : FubCaller_typ; (*Funktionsblöcke*)
		sServerInfo : Server_Data_typ; (*Infos über Server*)
		iWriteFilePos : UINT; (*Startposition für das schreiben des Files (Offset)*)
		iBufferPos : UINT;
		iGetBufLen : UINT; (*Länge der zu schreibenden Daten*)
		iActClient : UINT;
		iActPv : UINT;
	END_STRUCT;
END_TYPE
