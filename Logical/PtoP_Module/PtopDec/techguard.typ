
TYPE
	enumTGuard : 
		(
		sTG_Wait := 0, (*Wait State*)
		sTG_ReadDevice, (*Read available B&R Technology Guards*)
		sTG_RegisterLicense, (*Register license*)
		sTG_CheckLicense, (*Check license*)
		sTG_DeregisterLicense, (*Deregister license*)
		sTG_ReadOperatingTime, (*Reads operating time*)
		sTG_C_ReadOperatingTime, (*Reads customer operating time counter*)
		sTG_C_StartOperatingTime, (*Starts customer operating time counter*)
		sTG_C_StopOperatingTime, (*Stops customer operating time counter*)
		sTG_ReadUserData, (*Read user data*)
		sTG_WriteUserData, (*Write user data*)
		sTG_ReadBuRDongleData, (*Read B&R dongle context data*)
		sTG_WriteLicenseData, (*Write license data to B&R dongle*)
		sTG_ReadBuRDongleLicense, (*Read B&R dongle licenses*)
		sTG_Error (*Here some error Handling has to be implemented*)
		);
	Techguard_typ : 	STRUCT 
		Step : enumTGuard;
		dongleCnt : UDINT;
		dongleInfos : ARRAY[0..7]OF dongleInfo_t;
		boxMask : UINT;
		serNo : UDINT;
		operatingTime : DTStructure;
		cfgUpdateSeconds : UDINT;
		userData : ARRAY[0..241]OF USINT;
		contextData : ARRAY[0..8191]OF USINT;
		licenseData : ARRAY[0..8191]OF USINT;
		Licenses : ARRAY[0..99]OF licenseInfo_t;
		guardGetDongles_0 : guardGetDongles;
		guardRegisterLicense_0 : guardRegisterLicense;
		guardCheckLicense_0 : guardCheckLicense;
		guardDeregisterLicense_0 : guardDeregisterLicense;
		guardReadOperatingTime_0 : guardReadOperatingTime;
		guardStartCustomCounter_0 : guardStartCustomOpTimeCounter;
		guardStopCustomCounter_0 : guardStopCustomOpTimeCounter;
		guardReadData_0 : guardReadData;
		guardWriteData_0 : guardWriteData;
		guardGetContext_0 : guardGetContext;
		guardGetLicenses_0 : guardGetLicenses;
		guardUpdateLicenses_0 : guardUpdateLicenses;
	END_STRUCT;
END_TYPE
