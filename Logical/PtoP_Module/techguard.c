/*****************************************************************************************************************/
/*                    C - File für ptop                              */
/*---------------------------------------------------------------------------------------------------------------*/
/*            Taskname: server & client                           */
/*            Dateiname: Techguard.c                              */
/*            Autor:  B&R                                         */
/*            Erstelldatum: Dezember 2018                         */
/*            Classtime: 10 - 30000 ms                            */
/*---------------------------------------------------------------------------------------------------------------*/
/*  Funktion:                                                     */
/*          - Techguard Lizenz auslesen                           */
/*                                                                */
/*****************************************************************************************************************/

#ifdef _DEFAULT_INCLUDES
#include <AsDefault.h>
#endif

#include <bur/plc.h>
#include <bur/plctypes.h>
#include "../allgem.h"
#include "../techguard.h"

#define LICENSE_FIRM_CODE        101652
#define LICENSE_PRODUCT_CODE     349
#define LICENSE_ORDER_NUMBER     "1TGPTOP.00-01"
#define LICENSE_ORDER_DESC       "Peer-to-Peer NG runtime license"


// Technology guardian info ermitteln und Lizenz anmelden
void GetTechGuardianInfo(Techguard_typ* pTechGuard, USINT* pLicensed)
{
  
  if ((pTechGuard == NULL) || (pLicensed == NULL))
  {
    return;
  }
  
	/*
	Lizenzabfrage
##################################################
	Description of pTechGuard->Step:

	0........Wait State
	
	1........Read available B&R Technology Guards

	10.......Register license
	11.......Check license
	12.......Deregister license
	
	20.......Reads operating time
	
	30.......Reads customer operating time counter
	31.......Starts customer operating time counter
	32.......Stops customer operating time counter

	40.......Read user data
	41.......Write user data

	50.......Read B&R dongle context data
	51.......Write license data to B&R dongle
	
	
	100......Here some error Handling has to be implemented */


  switch (pTechGuard->Step)
  {
    case sTG_Wait:	/* Wait State */			
      break;		
		
    case sTG_ReadDevice:	/* Read available B&R Technology Guards */
      pTechGuard->guardGetDongles_0.enable         = TRUE;
      pTechGuard->guardGetDongles_0.pDongleInfos   = &pTechGuard->dongleInfos[0];
      pTechGuard->guardGetDongles_0.dongleInfoSize = sizeof(pTechGuard->dongleInfos);
      guardGetDongles(&pTechGuard->guardGetDongles_0);		/* Call the function block */
		
      //VisBkNotLicensed=0;
      *pLicensed = 0;
			
      if (pTechGuard->guardGetDongles_0.status == ERR_OK) /* guardGetDongles successful */
      {
        pTechGuard->Step        = sTG_RegisterLicense;
        pTechGuard->dongleCnt   = pTechGuard->guardGetDongles_0.neededSize;
        if (pTechGuard->dongleCnt > 0) 
        {
          pTechGuard->boxMask = pTechGuard->dongleInfos[0].boxMask;
          pTechGuard->serNo   = pTechGuard->dongleInfos[0].serNo;
        }
      }
      else if (pTechGuard->guardGetDongles_0.status == ERR_FUB_BUSY)
      {
        /* busy */
      }
      else
      {
        pTechGuard->Step      = sTG_Error;		/* go to error step */
        pTechGuard->dongleCnt = 0;
        brsmemset((UDINT) pTechGuard->dongleInfos, 0, sizeof(pTechGuard->dongleInfos));
      }
      break;
		
    case sTG_RegisterLicense:	/* Register license */
      
      pTechGuard->guardRegisterLicense_0.enable       = TRUE;
      pTechGuard->guardRegisterLicense_0.firmCode     = LICENSE_FIRM_CODE;
      pTechGuard->guardRegisterLicense_0.productCode  = LICENSE_PRODUCT_CODE;      
      brsstrcpy((UDINT) pTechGuard->guardRegisterLicense_0.orderNumber, (UDINT) LICENSE_ORDER_NUMBER);
      brsstrcpy((UDINT) pTechGuard->guardRegisterLicense_0.description, (UDINT) LICENSE_ORDER_DESC);
      pTechGuard->guardRegisterLicense_0.registerType = guardREGISTER_ALWAYS;
      pTechGuard->guardRegisterLicense_0.reaction     = guardLIC_REACT_LOGBOOK | guardLIC_REACT_BLINK_CPU_LED;
      guardRegisterLicense(&pTechGuard->guardRegisterLicense_0);			/* Call the function block */
		
      if (pTechGuard->guardRegisterLicense_0.status == ERR_OK)
      {
        pTechGuard->Step = sTG_CheckLicense;
				
      }				
      else if (pTechGuard->guardRegisterLicense_0.status == ERR_FUB_BUSY)
      {
        /* busy */
      }
      else
      {
        pTechGuard->Step = sTG_Error;
        *pLicensed = 0;
        //VisBkNotLicensed=1;
      }
      break;
		
    case sTG_CheckLicense:	/* Check license */
      pTechGuard->guardCheckLicense_0.enable = TRUE;
      pTechGuard->guardCheckLicense_0.ident  = pTechGuard->guardRegisterLicense_0.ident;
      guardCheckLicense(&pTechGuard->guardCheckLicense_0);				/* Call the function block */
			
      if (pTechGuard->guardCheckLicense_0.status == ERR_OK)
      {
        pTechGuard->Step = sTG_ReadOperatingTime;
        *pLicensed = 1;
        //VisBkNotLicensed=0;
      }
      else
      {
        pTechGuard->Step = sTG_Error;
        //VisBkNotLicensed=1;
      }
      break;
				
    case sTG_DeregisterLicense:	/* Deregister license */
      pTechGuard->guardDeregisterLicense_0.enable = TRUE;
      pTechGuard->guardDeregisterLicense_0.ident  = pTechGuard->guardRegisterLicense_0.ident;
      guardDeregisterLicense(&pTechGuard->guardDeregisterLicense_0);			/* Call the function block */
		
      if (pTechGuard->guardDeregisterLicense_0.status == ERR_OK)
      {
        pTechGuard->Step = sTG_Wait;
      }
      else if (pTechGuard->guardDeregisterLicense_0.status == ERR_FUB_BUSY)
      {
        /* busy */
      }
      else
      {
        pTechGuard->Step = sTG_Error;
      }
      break;
		
    case sTG_ReadOperatingTime:	/* Reads operating time */
      pTechGuard->guardReadOperatingTime_0.enable      = TRUE;
      pTechGuard->guardReadOperatingTime_0.boxMask     = pTechGuard->boxMask;
      pTechGuard->guardReadOperatingTime_0.serNo       = pTechGuard->serNo;
      pTechGuard->guardReadOperatingTime_0.counterType = guardGENERAL_OP_TIME_COUNTER;
      guardReadOperatingTime(&pTechGuard->guardReadOperatingTime_0);		/* Call the function block */
		
      if (pTechGuard->guardReadOperatingTime_0.status == ERR_OK)
      {
        pTechGuard->Step = sTG_ReadBuRDongleLicense;
			
        /* convert to readable Format */
        DT_TO_DTStructure(pTechGuard->guardReadOperatingTime_0.operatingTime, (UDINT) &pTechGuard->operatingTime);
        pTechGuard->operatingTime.year  = pTechGuard->operatingTime.year  - 1970;		/* correction to date 01.01.1970 */
        pTechGuard->operatingTime.month = pTechGuard->operatingTime.month - 1;		/* correction to date 01.01.1970 */
        pTechGuard->operatingTime.day   = pTechGuard->operatingTime.day   - 1;		/* correction to date 01.01.1970 */
      }
      else if (pTechGuard->guardReadOperatingTime_0.status == ERR_FUB_BUSY)
      {
        /* busy */
      }
      else /* go to error step */
      {
        pTechGuard->Step = sTG_Error;
      }
      break;
		
    case sTG_C_ReadOperatingTime:	/* Reads customer operating time */
      pTechGuard->guardReadOperatingTime_0.enable      = TRUE;
      pTechGuard->guardReadOperatingTime_0.boxMask     = pTechGuard->boxMask;
      pTechGuard->guardReadOperatingTime_0.serNo       = pTechGuard->serNo;
      pTechGuard->guardReadOperatingTime_0.counterType = guardCUSTOMER_OP_TIME_COUNTER;
      guardReadOperatingTime(&pTechGuard->guardReadOperatingTime_0);			/* Call the function block */
		
      if (pTechGuard->guardReadOperatingTime_0.status == ERR_OK)
      {
        pTechGuard->Step = sTG_Wait;
        /* convert to readable Format */
        DT_TO_DTStructure(pTechGuard->guardReadOperatingTime_0.operatingTime, (UDINT) &pTechGuard->operatingTime);
        pTechGuard->operatingTime.year  = pTechGuard->operatingTime.year - 1970;		/* correction to date 01.01.1970 */
        pTechGuard->operatingTime.month = pTechGuard->operatingTime.month - 1;		/* correction to date 01.01.1970 */
        pTechGuard->operatingTime.day   = pTechGuard->operatingTime.day   - 1;		/* correction to date 01.01.1970 */
      }	
      else if (pTechGuard->guardReadOperatingTime_0.status == ERR_FUB_BUSY)
      {
        /* busy */
      }
      else /* go to error step */
      {
        pTechGuard->Step = sTG_Error;
      }
      break;
		
    case sTG_C_StartOperatingTime:	/* Starts customer operating time counter */
      pTechGuard->guardStartCustomCounter_0.enable           = TRUE;
      pTechGuard->guardStartCustomCounter_0.boxMask          = pTechGuard->boxMask;
      pTechGuard->guardStartCustomCounter_0.serNo            = pTechGuard->serNo;
      pTechGuard->guardStartCustomCounter_0.counterType      = guardCUSTOMER_OP_TIME_COUNTER;
      pTechGuard->guardStartCustomCounter_0.cfgUpdateSeconds = pTechGuard->cfgUpdateSeconds;
      guardStartCustomOpTimeCounter(&pTechGuard->guardStartCustomCounter_0);		/* Call the function block */
		
      if (pTechGuard->guardStartCustomCounter_0.status == ERR_OK)
      {
        pTechGuard->Step = sTG_Wait;
      }
      else if ( pTechGuard->guardStartCustomCounter_0.status == ERR_FUB_BUSY)
      {
        /* busy */
      }
      else
      {
        pTechGuard->Step = sTG_Error;
      }
      break;
		
    case sTG_C_StopOperatingTime:	/* Stops customer operating time counter*/
      pTechGuard->guardStopCustomCounter_0.enable = TRUE;
      pTechGuard->guardStopCustomCounter_0.ident  = pTechGuard->guardStartCustomCounter_0.ident;
      guardStopCustomOpTimeCounter(&pTechGuard->guardStopCustomCounter_0);		/* Call the function block */
		
      if (pTechGuard->guardStopCustomCounter_0.status == ERR_OK)
      {
        pTechGuard->Step = sTG_Wait;
      }
      else if (pTechGuard->guardStopCustomCounter_0.status == ERR_FUB_BUSY)
      {
        /* busy */
      }
      else
      {
        pTechGuard->Step = sTG_Error;
      }
      break;
		
    case sTG_ReadUserData:	/* Read user data */
      pTechGuard->guardReadData_0.enable      = TRUE;
      pTechGuard->guardReadData_0.boxMask     = pTechGuard->boxMask;
      pTechGuard->guardReadData_0.serNo       = pTechGuard->serNo;
      guardReadData(&pTechGuard->guardReadData_0);				/* Call the function block */
		
      if (pTechGuard->guardReadData_0.status == ERR_OK)
      {
        pTechGuard->Step = sTG_Wait;
        brsstrcpy((UDINT) pTechGuard->userData, (UDINT) pTechGuard->guardReadData_0.userData);
      }
      else if ( pTechGuard->guardReadData_0.status == ERR_FUB_BUSY )
      {
        /* busy */
      }
      else
      {
        pTechGuard->Step = sTG_Error;
      }
      break;
		
    case sTG_WriteUserData:	/*Write user data */
      pTechGuard->guardWriteData_0.enable      = TRUE;
      pTechGuard->guardWriteData_0.boxMask     = pTechGuard->boxMask;
      pTechGuard->guardWriteData_0.serNo       = pTechGuard->serNo;
      brsstrcpy((UDINT) pTechGuard->guardWriteData_0.userData, (UDINT) pTechGuard->userData);
      pTechGuard->guardWriteData_0.userDataLen = brsstrlen((UDINT) pTechGuard->userData);
      guardWriteData(&pTechGuard->guardWriteData_0);				/* Call the function block */

      if (pTechGuard->guardWriteData_0.status == ERR_OK)
      {
        pTechGuard->Step = sTG_Wait;
      }
      else if (pTechGuard->guardWriteData_0.status == ERR_FUB_BUSY)
      {
        /* busy */
      }
      else
      {
        pTechGuard->Step = sTG_Error;
      }
      break;
		
    case sTG_ReadBuRDongleData:	/* Read B&R dongle context data */
      pTechGuard->guardGetContext_0.enable              = TRUE;
      pTechGuard->guardGetContext_0.boxMask             = pTechGuard->boxMask;
      pTechGuard->guardGetContext_0.serNo               = pTechGuard->serNo;
      pTechGuard->guardGetContext_0.firmCode            = guardBR_FIRMCODE;
      pTechGuard->guardGetContext_0.maxContextDataCount = sizeof(pTechGuard->contextData);
      pTechGuard->guardGetContext_0.pContextData        = (UDINT) pTechGuard->contextData;
      guardGetContext(&pTechGuard->guardGetContext_0);
		
      if (pTechGuard->guardGetContext_0.status == ERR_OK)
      {
        pTechGuard->Step = sTG_Wait;
      }
      else if (pTechGuard->guardGetContext_0.status == ERR_FUB_BUSY)
      {
        /* busy */
      }
      else
      {
        pTechGuard->Step = sTG_Error;
      }
      break;
		
    case sTG_WriteLicenseData:	/* Write license data TO B&R dongle */
      pTechGuard->guardUpdateLicenses_0.enable           = TRUE;
      pTechGuard->guardUpdateLicenses_0.pLicenseData     = (UDINT) pTechGuard->licenseData;
      pTechGuard->guardUpdateLicenses_0.licenseDataCount = brsstrlen((UDINT) pTechGuard->licenseData);
      guardUpdateLicenses(&pTechGuard->guardUpdateLicenses_0);
		
      if (pTechGuard->guardUpdateLicenses_0.status == ERR_OK)
      {
        pTechGuard->Step = sTG_Wait;
      }
      else if (pTechGuard->guardUpdateLicenses_0.status == ERR_FUB_BUSY)
      {
        /* busy */
      }
      else
      {
        pTechGuard->Step = sTG_Error;
      }
      break;
		
		
		
		
		
    case sTG_ReadBuRDongleLicense:	/* Read B&R dongle licenses */
			
      pTechGuard->guardGetLicenses_0.enable          = TRUE;
      pTechGuard->guardGetLicenses_0.maxNumLicenses		=99;
      pTechGuard->guardGetLicenses_0.pLicenses			= (UDINT) pTechGuard->Licenses;
      guardGetLicenses(&pTechGuard->guardGetLicenses_0);
					
		
      if (pTechGuard->guardGetLicenses_0.status == ERR_OK)
      {
        pTechGuard->Step = sTG_Wait;
      }
      else if (pTechGuard->guardGetLicenses_0.status == ERR_FUB_BUSY)
      {
        /* busy */
      }
      else
      {
        pTechGuard->Step = sTG_Error;
      }
      break;
		
			
    case sTG_Error:  /* Here some error handling has to be implemented */
      
      pTechGuard->Step = sTG_ReadBuRDongleLicense;
      break;
  }

}


// wird nur im EXIT benutzt -> einfaches abmelden der Lizenz, da ansonsten der Lizenzcounter nur erhöht wird.
UINT UnregisterLicense(Techguard_typ* pTechGuard)
{
  UINT Status = ERR_FUB_BUSY;
  
  if (pTechGuard == NULL)
  {
    return 1; // keine Adresse vorhanden, Ende
  }
  
  if (pTechGuard->guardRegisterLicense_0.ident == 0)
  {
    return 2; // kein gültiger Ident vorhanden, Ende
  }
  
  pTechGuard->guardDeregisterLicense_0.enable = TRUE;
  pTechGuard->guardDeregisterLicense_0.ident  = pTechGuard->guardRegisterLicense_0.ident;
  guardDeregisterLicense(&pTechGuard->guardDeregisterLicense_0);			/* Call the function block */
	     
  if (pTechGuard->guardDeregisterLicense_0.status != ERR_FUB_BUSY)
  {
    Status = pTechGuard->guardDeregisterLicense_0.status;
  }
  
  return Status;
}
