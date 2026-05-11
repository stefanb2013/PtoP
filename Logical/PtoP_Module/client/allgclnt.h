/*****************************************************************************************************************/
/*										Header File für UDP	Client				´							     */	
/*---------------------------------------------------------------------------------------------------------------*/
/*						Taskname: client																		 */
/*						Dateiname: allgclnt.h																	 */
/*						Autor:	B&R																				 */
/*						Erstelldatum: Mai 2002																	 */
/*						Classtime: 10 - 30000 ms																 */
/*---------------------------------------------------------------------------------------------------------------*/
/*	Funktion:																									 */
/*					- allgemeine Deklarationen													     		 	 */
/*					- Clientparametrierung												 					 	 */
/*					- Prototyping 															 					 */
/*																												 */
/*****************************************************************************************************************/
#ifndef ALLGCLNT_H_
#define ALLGCLNT_H_

#ifdef _DEFAULT_INCLUDES
  #include <AsDefault.h>
#endif

#include "../allgem.h"
#include "asarcfg.h"

/*****************************************************************************************************************/
/* allgem. Parametrierungen */

#if INTEL
	#define  MAX_VARIABLEN_LOOP				200 	/* Anzahl der Variablen, welche pro Zyklus bearbeitet werden dürfen !*/
#else

	#define  MAX_VARIABLEN_LOOP		 	  100
#endif

#define  DATA_OBJ_MEM_NAME_CLIENT			"ptopclnt"
#define  LOGGER_NAME_CLNT	          	"_PtoPClnt"


#define  TIMEOUT_RETRY					  	3		/* Anzahl der Versuche nach Timeout 	*/
#define  TIMEOUT_RETRY_CONN				  1		/* Anzahl der Versuche bei Connection (nur ein Telegramm bei Anmeldung) */
#define  TIMEOUT_LIFE_RETRY					3		/* Anzahl der Versuche bei Lifecheck*/	
#define  GRENZWERT_NO					 	    65535	/* Grenzwerte werden bei 65535 nicht auf Variable geschrieben				*/
#define  LOGBOOK_ENTRIES_DM					5		/* max. Einträge von Fehlerhaften Variablen in das Logbuch bei auslesen Datenmodul*/	
#define  LOGBOOK_ENTRIES_CONN				5		/* max. Einträge von Fehlerhaften Variablen in das Logbuch während Anmeldephase*/	
#define  TIMEOUT_HOST_CHECK_IP			60000	/* Zeit, welche ablaufen muss, bis Ipadresse erneut angefordert wird */

/*****************************************************************************************************************/
/* Prototyping																									 */
UINT CRCcheck (UDINT* TelegrammStartAdresse, UINT Telegrammlaenge );
void GetPointerOfPV(UDINT* StartAdresse, UINT AnzahlVariablen);
plcbit ParsenDataModul (UDINT* StartAdressePV, UDINT* StartAdresseServer, USINT* StartAdresseDM, UDINT LaengeDM, UINT* puiActVar);
void Vorbelegung (UDINT* StartAdressePV, UDINT* StartAdresseServer);
void ErsatzwerteSetzen(UDINT *StartAdresse, UINT Server);
void ParserLogbuch (UDINT *StartAdresse );
void VerifyVariables (UDINT *StartAdresse, UINT Logbuch, UDINT *ErrorAdresse );
void SetEntryErrorList (UDINT *StartAdresse, USINT MainStep, USINT Step, UINT Status, UINT PVIndex, UDINT IPAdress, UINT Portnumber, UDINT SysTimeTick, DATE_AND_TIME ActTime);
void FilterErrorList(UDINT *StartAdresse, UDINT *AdresseAnzeige);
void LogbuchEintrag(UDINT* StartAdresse, UINT ClientIndex, USINT LogbuchCnt, USINT TypErr);
UDINT TimerFunction (UDINT *AdrTickCount);
UDINT SwapUdintForIntel(UDINT value);
UINT SwapUintForIntel(UINT value);
UDINT GetOwnIpAddress();
void CopyClientConnectRequestToSendBuffer(UDINT* pSendBuf, ClientConnectRequest_typ *pClientConnect);
void CopyReceiveBufferToClientConnectResponse(UDINT* pRecvBuf);
void CopyPVOpenRequestToSendBuffer(UDINT* pSendBuf, PVOpenRequest_typ *pPVOpenRequest);
void CopyReceiveBufferToPVOpenResponse(UDINT* pRecvBuf);
void CopyClientLinkToSendBuffer(UDINT* pSendBuf);
void CopyReceiveBufferToNAK(UDINT* pRecvBuf);
void CopyReceiveBufferToPVEvent(UDINT* pRecvBuf);
void CopyPVEventAcknowledgeToSendBuffer(UDINT* pRecvBuf);
void CopyLifeCheckRequestToSendBuffer();
void CopyReceiveBufferToLifeCheckResponse();
UDINT GetServerByIp(UDINT IpAddr, UINT* pServerIndex);
UDINT GetServerByIpStr(UDINT* pIpAddr, UINT* pServerIndex);
void SetPvState(unsigned long  pPvAdr, unsigned short State);
void SetServerState(UDINT pPv, UINT uiServerState);
UINT GetPvStateFromServerState(UINT uiServerState);
void GetClock(UDINT *pudActClock);

/*****************************************************************************************************************/
/* Expressions                                                                                                   */

#define UNUSED(expr) do { (void)(expr); } while (0)

#endif
