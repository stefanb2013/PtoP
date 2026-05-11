(*****************************************************************
  Created with BrIecConverter V1.00
  File: Global_telegram.typ
  Date: 2018.07.12 10:28
*****************************************************************)

TYPE
	PVOpenRequest_typ : 	STRUCT  (*Anforderung für einfache Anmeldung: 1 Variable *)
		Kennung : ARRAY[0..3]OF USINT; (*Telegrammkennung*)
		ClientIndex : UINT; (*Client Index*)
		Variablenname : ARRAY[0..PV_LENGTH_ARR]OF USINT; (*Variablen name*)
		PVIndex : UINT; (*Variablen index*)
		Laenge : UINT; (*Variablen länge*)
		Hysterese : UDINT; (*Hysterese*)
		SyncTime : UINT; (*Sycnhronisierungszeit*)
		Reserve : ARRAY[0..1]OF UDINT; (*Reserve*)
		BCC : UINT; (*Checksum*)
	END_STRUCT;
	PVOpenResponse_typ : 	STRUCT  (* Antwort für einfache Anmeldung: 1 Variable *)
		Kennung : ARRAY[0..3]OF USINT; (*Telegrammkennung*)
		Status : UINT; (*Änderung: 12.01.03: -> wenn Variable nicht existiert, muss trotzdem ein Response erfolgen!*)
		ClientIndex : UINT; (*Client Index*)
		PVIndex : UINT; (*Variablen index*)
		PVIdent : UDINT; (*Adresse der PV*)
		Laenge : UINT; (*Variablen länge*)
		Wert : UDINT; (*Variablen wert*)
		BCC : UINT; (*Checksumme*)
	END_STRUCT;
	PVOpenRequestX_typ : 	STRUCT  (* Anfrage für geblockte Anmeldung: n Variablen *)
		ClientIndex : UINT; (*Clientnummer, wird auf Server abgelegt und v. Client zur richtigen Zuordnung benötigt*)
		Variablenname : ARRAY[0..PV_LENGTH_ARR]OF USINT; (*PV String	*)
		PVIndex : UINT; (*Verwaltung der PV am Client ->Zuordnung der PV am Client bei jedem Event*)
		Laenge : UINT; (*Länge der Variable				*)
		Hysterese : UDINT; (*Hysterese für Eventauswertung	*)
		SyncTime : UINT; (*Zeitsynchronisation in Sekunden	*)
	END_STRUCT;
	PVOpenResponseX_typ : 	STRUCT  (* Antwort für geblockte Anmeldung: n Variablen *)
		Status : UINT;
		PVIndex : UINT;
		PVIdent : UDINT;
		Laenge : UINT;
		Wert : UDINT;
	END_STRUCT;
	ClientConnectRequest_typ : 	STRUCT  (*Alle Variablen wurden gesendet*)
		Kennung : ARRAY[0..3]OF USINT;
		ClientIndex : UINT; (*änderung:Server speichert ClientIndex ab -> wird für Verwaltung benütigt*)
		Timeout : UINT;
		BCC : UINT;
	END_STRUCT;
	ClientConnectResponse_typ : 	STRUCT 
		Kennung : ARRAY[0..3]OF USINT;
		ClientIndex : UINT;
		Status : UINT;
		BCC : UINT;
	END_STRUCT;
	PVEvent_typ : 	STRUCT 
		Kennung : ARRAY[0..3]OF USINT;
		PVIndex : UINT;
		Laenge : UINT;
		Wert : UDINT;
		EventCnt : UINT;
		BCC : UINT;
	END_STRUCT;
	PVEventAcknowledge_typ : 	STRUCT 
		Kennung : ARRAY[0..3]OF USINT;
		Status : UINT;
		PVIndex : UINT;
		ClientIndex : UINT;
		EventCnt : UINT;
		BCC : UINT;
	END_STRUCT;
	PVEventX_typ : 	STRUCT 
		PVIndex : UINT;
		Laenge : UINT;
		Wert : UDINT;
	END_STRUCT;
	PVEventAcknowledgeX_typ : 	STRUCT 
		Status : UINT;
		PVIndex : UINT;
	END_STRUCT;
	LifeCheckRequest_typ : 	STRUCT 
		Kennung : ARRAY[0..3]OF USINT;
		Reserve1 : UINT;
		BCC : UINT;
	END_STRUCT;
	LifeCheckResponse_typ : 	STRUCT 
		Kennung : ARRAY[0..3]OF USINT;
		ClientStatus : UINT; (*Erweiterung 12.01.02*)
		BCC : UINT;
	END_STRUCT;
	AckResponse_typ : 	STRUCT 
		Kennung : ARRAY[0..3]OF USINT;
		Status : UINT;
		Reserve2 : UINT;
		BCC : UINT;
	END_STRUCT;
	ClientLink_typ : 	STRUCT 
		Kennung : ARRAY[0..3]OF USINT;
		ClientIndex : UINT;
		BCC : UINT;
	END_STRUCT;
END_TYPE
