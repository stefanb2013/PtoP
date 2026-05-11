
FUNCTION_BLOCK PtopInit
	VAR_INPUT
		enable : BOOL;
	END_VAR
	VAR_OUTPUT
		status : UINT;
	END_VAR
END_FUNCTION_BLOCK

FUNCTION_BLOCK PtopGetStateOfServer
	VAR_INPUT
		ServerId : UINT;
		PvStatus : UINT;
	END_VAR
	VAR_OUTPUT
		ServerStatus : UINT;
	END_VAR
END_FUNCTION_BLOCK

FUNCTION_BLOCK PtopRebuildConn
	VAR_INPUT
		enable : BOOL;
		ServerId : UINT;
	END_VAR
END_FUNCTION_BLOCK

FUNCTION_BLOCK PtopLifeCheckServer
	VAR_INPUT
		ServerId : UINT;
	END_VAR
	VAR_OUTPUT
		Lebensuebeberwachung : USINT;
	END_VAR
END_FUNCTION_BLOCK

FUNCTION_BLOCK PtopLifeAllServer
	VAR_INPUT
		enable : BOOL;
		Lebensueberwachung : UDINT;
		Feldgroesse : UINT;
	END_VAR
END_FUNCTION_BLOCK

FUNCTION_BLOCK PtopAllRebuildConn
	VAR_INPUT
		enable : BOOL;
	END_VAR
END_FUNCTION_BLOCK

FUNCTION_BLOCK PtopServGetClients
	VAR_INPUT
		enable : BOOL;
		BuildFile : BOOL;
		szDeviceName : STRING[32];
	END_VAR
	VAR_OUTPUT
		status : UINT;
		sInfos : GetClients_Info_typ;
	END_VAR
	VAR
		sIntern : GetClients_Intern_typ;
	END_VAR
END_FUNCTION_BLOCK
