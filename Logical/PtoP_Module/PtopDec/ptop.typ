(********************************************************************
 * COPYRIGHT -- Bernecker+Rainer
 ********************************************************************
 * Package: ptop
 * File: ptop.typ
 * Author: B&R
 * Created: July 13, 2018
 ********************************************************************
 * Data types of package ptop
 ********************************************************************)

TYPE
	PvList_Typ : 	STRUCT 
		NameClient : ARRAY[0..PV_LENGTH_ARR]OF USINT; (*Name der Variable auf Client*)
		NameServer : ARRAY[0..PV_LENGTH_ARR]OF USINT; (*Name der Variable auf Server*)
		Hysterese : UDINT; (*Hysterese*)
		Init : UDINT; (* Startwert der Variable								*)
		Remanent : UINT; (* Remanent ?											*)
		Ersatzwert : UDINT; (* Ersatzwert 											*)
		pPv : UDINT; (* Zeiger der Variable auf Client 						*)
		PvLaenge : UINT; (* Länge der Variable auf Client						*)
		DataType : UINT; (* Datentyp der Variable auf Client						*)
		pPvServer : UDINT; (* Identifizierung der Variable auf Server 				*)
		SyncTime : UINT; (* Synchronisierungsinterval auf Server 				*)
		Client : UINT; (* ClientIndex -> Offset auf dem sich die Clientdaten befinden: 0 - (Max-1)	*)
		Status : UINT; (* aktueller Status der Variable, siehe Konstanten							*)
		StatusTxt : ARRAY[0..19]OF USINT; (* aktueller Status, Text								*)
		Anmeldung : UINT; (* Stand der Anmeldung									*)
		StatusServ : UINT; (* Status der Anmeldung vom Server*)
		LetzterWert : UDINT; (* Daten (Wert) von Server für diese Variable			*)
		LetzterWertCnt : UINT; (* Änderungszähler für Daten		*)
		LetzterWertServ : UINT; (*von welchem Server kam der letzte Wert: Index*)
		LetzterWertServIp : STRING[IP_ADR_LEN]; (*von welchem Server kam der letzte Wert: Ip Adress*)
	END_STRUCT;
	Client_List_Typ : 	STRUCT 
		Timeout : UINT; (*Timeout der Variable									*)
		IPAdresse : UDINT; (*IP - Adresse d. Servers								*)
		HostName : ARRAY[0..HOST_NAME_LEN_ARR]OF USINT; (*Hostname des Servers									*)
		IpAdr : ARRAY[0..IP_ADR_LEN_ARR]OF USINT; (*Ip-Adresse d. Servers als String 					*)
		ConfByHost : UINT; (*Konfiguration aus ptop-Dat als Hostname (1) oder Ip (0)	*)
		IpHostTimeout : UDINT; (*letzter Zeitpunkt, in dem der Hostname ?berpr?ft wurde*)
		ClientIndex : UINT; (*auf welchem Index wird Client auf Server verwaltet 	*)
		LifeCheck : UINT; (*lebt Parametrierte Station?, ist LifeCheck auf 1 so wurde von Station nach(!) dem Anmeldevorgang kein Telegramm mehr Empfangen 		*)
		LifeCheckTime : UDINT; (* letzter Zeitpunkt, bei dem ein Antworttelegramm eingetroffen ist *)
		LifeCheckTrial : UDINT; (*wie oft wurde Telegramm an Server gesendet 				*)
		Modus : UINT; (*Daten einfach ("OPEN") oder geblockt ("OPEX") versenden, wird ?ber Anmeldung ermittelt*)
	END_STRUCT;
END_TYPE
