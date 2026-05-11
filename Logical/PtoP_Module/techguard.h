/*****************************************************************************************************************/
/*										H - File für ptop					´							     	 */	
/*---------------------------------------------------------------------------------------------------------------*/
/*						Taskname: server + client																 */
/*						Dateiname: techguard.h																	 */
/*						Autor:	B&R						      														 */
/*						Erstelldatum: Dezember 2018 														 */
/*						Classtime: 10 - 30000 ms																 */
/*---------------------------------------------------------------------------------------------------------------*/
/*	Funktion:																								*/
/*					- 								     		 	                    */
/*					- 							 					                      */
/*																												  */
/*****************************************************************************************************************/
#ifndef TECHGUARD_H_
#define TECHGUARD_H_

#ifdef _DEFAULT_INCLUDES
#include <AsDefault.h>
#endif

void GetTechGuardianInfo(Techguard_typ* pTechGuard, USINT* pLicensed);
UINT UnregisterLicense(Techguard_typ* pTechGuard);

#endif
