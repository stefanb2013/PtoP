
TYPE
	PBrbStepHandlingIntern_type : 	STRUCT 
		nStepNrOld : DINT;
		nCycleCount : UDINT;
		bLogOnNextCycle : BOOL;
		fbTimeout : TON;
	END_STRUCT;
	PBrbStepHandlingStep_type : 	STRUCT 
		nStepNr : DINT; (*Schrittnummer*)
		sStepText : STRING[nBRB_STEP_TEXT_CHAR_MAX]; (*Schritttext*)
		nCycleCount : UDINT; (*Zyklen*)
	END_STRUCT;
	PBrbStepHandlingLog_type : 	STRUCT 
		bClear : BOOL; (*Kommando zum Löschen der Protokollierung*)
		bStop : BOOL; (*Kommando zum Stoppen der Protokollierung*)
		Steps : ARRAY[0..nBRB_STEPLOG_STEPS_MAX]OF PBrbStepHandlingStep_type; (*Protokollierung*)
	END_STRUCT;
	PBrbStepHandlingCurrent_type : 	STRUCT 
		nStepNr : DINT; (*Aktuelle Schrittnummer*)
		sStepText : STRING[nBRB_STEP_TEXT_CHAR_MAX]; (*Aktueller Schritttext*)
		bTimeoutElapsed : BOOL; (*1=Timeout abgelaufen*)
		nTimeoutContinueStep : DINT; (*Schrittnummer nach Timeout*)
	END_STRUCT;
	PBrbStepHandling_type : 	STRUCT 
		Current : PBrbStepHandlingCurrent_type;
		Log : PBrbStepHandlingLog_type;
		Intern : PBrbStepHandlingIntern_type;
	END_STRUCT;
	PtopStepMain_type : 	STRUCT 
		StepText : STRING[nBRB_STEP_TEXT_CHAR_MAX]; (*Name of step *)
		StepNr : UINT; (*Step Nr: switch / Case*)
		InitDone : BOOL; (*Init Successfull*)
	END_STRUCT;
	PtopStepProt_type : 	STRUCT 
		Step : PtopStepMain_type;
		StepHandling : PBrbStepHandling_type;
	END_STRUCT;
END_TYPE
