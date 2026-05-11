(*****************************************************************
  Created with BrIecConverter V1.00
  File: Global_pipe.typ
  Date: 2018.07.12 10:28
*****************************************************************)

TYPE
	PipeManagement_typ : 	STRUCT 
		DataAvailable : UINT; (*daten sind vorhanden*)
		ClientIndex : UINT; (*für welchen client*)
		PVIndex : UINT; (*welche Variable in der Liste*)
		Laenge : UINT; (*Variablenlänge - sizeof()*)
		Wert : UDINT; (*Variablenwert*)
	END_STRUCT;
	PipeLock_typ : 	STRUCT 
		Lock : BOOL; (*konnte Auftrag in Pipe geschrieben werden*)
		PVIndex : UINT; (*bei welcher PV konnte Auftrag nicht mehr in Pipe geschrieben werden*)
	END_STRUCT;
	PipeMonitor_typ : 	STRUCT 
		Lock : BOOL; (*Monitormode enabled*)
		PipeIndex : UINT; (*Speicherplatz innerhalb der Pipe*)
		PipeNr : UINT; (*Pipe zu Client*)
	END_STRUCT;
END_TYPE
