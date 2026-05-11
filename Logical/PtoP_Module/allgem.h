/*****************************************************************************************************************/
/*										Header File für UDP	Client/Server		´							     */	
/*---------------------------------------------------------------------------------------------------------------*/
/*						Taskname: client bzw. server															 */
/*						Dateiname: allgem.h																		 */
/*						Autor:	B&R																				 */
/*						Erstelldatum: Juni 2007																	 */
/*						Classtime: 10 - 30000 ms																 */
/*---------------------------------------------------------------------------------------------------------------*/
/*	Funktion:																									 */
/*					- allgemeine Deklarationen													     		 	 */
/*					- Clientparametrierung												 					 	 */
/*					- Prototyping 															 					 */
/*																												 */
/*****************************************************************************************************************/
#ifndef ALLGEM_H
#define ALLGEM_H

#define PTOP_VERSION    6500

#define	 TRUE						1
#define	 FALSE					0

#ifndef NULL
#define NULL 						0L
#endif

#ifndef NULL_ADR
#define NULL_ADR				0L
#endif

#define LONG_PV 				0

#if LONG_PV
	
	#define  CRC_OPEN_TELEGRAM	104	
#else
	
	#define  CRC_OPEN_TELEGRAM	74	
#endif

#if INTEL
	#define	 UDP_BUF_SEND_SIZE		250000
	#define	 UDP_BUF_RECV_SIZE		250000
#else
	#define	 UDP_BUF_SEND_SIZE		50000
	#define	 UDP_BUF_RECV_SIZE		50000
#endif

#define  MAX_RETRY_CALL_FUB_CONN	70		/* Zeitspanne bei Conn höher, da bei nicht - Erfolg dieses Schrittes Anmeldevorgang komplett abgebrochen wird! */
#define	 MAX_RETRY_CALL_FUB			50		/* max. Anzahl der Funktionsaufrufe UDP - Fubs	*/

#define  MAX_UDP_FRAME_LEN			(1500 - 40)	/* max. Nutzdaten in einem gesendeten UDP Telegramm: Email 22.06.2007 11:42 von Matthias Melitzer*/
												/* Ergänzung: nicht max. Nutzdaten, sondern komplette Telegrammlänge; deshalb "- 40 Bytes" */


/* --- Lifecheck Zustände ---------------------------------------------------------------- */
#define STATE_LIFE_OK				0		/* Lifecheck ist ok							*/
#define STATE_LIFE_CLIENT_MISSING	1		/* keine Verbindung mehr zu Server			*/
#define STATE_LIFE_CONN				2		/* Anmeldung erfolgt zur zeit zum Server 	*/
#define STATE_LIFE_ERR				3		/* Anmeldung erfolgt zur zeit zum Server 	*/


/* --- Telegramme ----------------------------------------------- */
#define TELEGRAMM_IDENT_CONN		"CONN"
#define TELEGRAMM_IDENT_CONX		"CONX"
#define TELEGRAMM_IDENT_OPEN		"OPEN"
#define TELEGRAMM_IDENT_OPEX		"OPEX"
#define TELEGRAMM_IDENT_LINK		"LINK"
#define TELEGRAMM_IDENT_LINX		"LINX"
#define TELEGRAMM_IDENT_ACKN		"ACKN"
#define TELEGRAMM_IDENT_EVNT		"EVNT"
#define TELEGRAMM_IDENT_EVNX		"EVNX"
#define TELEGRAMM_IDENT_LIFE		"LIFE"

/* --- Betriebsarten Modus --------------------------------------- */
#define MODE_OLD        0
#define MODE_NEW        1

/* --- Datenobjektnamen ------------------------------------------- */
#define CONF_DAT_OBJ_SERV       "confserv"
#define CONF_DAT_OBJ_CLIENT     "confclnt"

#endif

