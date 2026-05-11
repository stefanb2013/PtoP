(*- - - ein Eintrag von der ptop Liste*)

TYPE
	Time_typ : 	STRUCT 
		Intervall : UINT; (*Refresh Intervall fuer Aktualisierung*)
		Old : UDINT; (*letzter Wert bei Fub Aufruf			*)
	END_STRUCT;
	Client_typ : 	STRUCT 
		iClntIndex : INT; (*Nummer des Clients -> Speicheroffset*)
		uiClntPvIndex : UINT; (*VerwaltungsIndex der Variable auf Client*)
	END_STRUCT;
	PVListEntry_typ : 	STRUCT 
		pPv : UDINT; (*Adresse der Variable im Server 	*)
		Name : ARRAY[0..PV_LENGTH_ARR]OF USINT; (*Name der PV						*)
		PvLaenge : UINT; (*Laenge der Variable im Server 	*)
		Hysterese : UDINT; (*Hysterese der Variable im Server*)
		PvWert : UDINT; (*Letzter Wert der Variable im Server*)
		PvWertCnt : UINT; (*Aenderungszaehler fuer Daten - Debuging	*)
		Tick : Time_typ; (*Zeitverwaltung bei Intervall - Refresh*)
		Client : ARRAY[0..MAX_ANZAHL_CLIENTS_ARR]OF Client_typ;
	END_STRUCT;
END_TYPE

(*- - - Strukturen Client Liste*)

TYPE
	ClientListeEntry_typ : 	STRUCT 
		IPAdresse : UDINT; (*IPAdresse des Client Targets*)
		IpAdr : ARRAY[0..IP_ADR_LEN_ARR]OF USINT; (*Ip-Adresse d. Servers als String*)
		LinkFlag : UINT; (*Zustand, in dem sich der Anmelde / Auswertungsprozeß befindet*)
		Timeout : UINT; (*Timeout [100 ms]*)
		ClientIndex : UINT; (*ClientIndex auf Client !*)
		LastTimeStamp : UDINT; (*letzte Aktualisierung *)
		Modus : UINT; (*Daten einfach ("OPEN") oder geblockt ("OPEX") versenden, wird über Anmeldung ermittelt*)
	END_STRUCT;
END_TYPE
