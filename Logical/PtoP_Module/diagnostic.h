/*****************************************************************************************************************/
/*										H - File für ptop					´							     	 */	
/*---------------------------------------------------------------------------------------------------------------*/
/*						Taskname: server + client																 */
/*						Dateiname: diagnostic.h																	 */
/*						Autor:	B&R																				 */
/*						Erstelldatum: April 2005																 */
/*						Classtime: 10 - 30000 ms																 */
/*---------------------------------------------------------------------------------------------------------------*/
/*	Funktion:																								*/
/*					- 								     		 	                    */
/*					- 							 					                      */
/*																												  */
/*****************************************************************************************************************/
#ifndef DIAGNOSTIC_H_
#define DIAGNOSTIC_H_

#ifdef _DEFAULT_INCLUDES
#include <AsDefault.h>
#endif

/* --- Basiseinstellungen -------------------------------------------------------------- */
#define BUFFER_TEMP_LEN				  80
#define STEP_CHAR_LEN				    25
#define MAX_PV_DIAGNOSE				  10

#define STATE_OK					      0
#define STATE_NULL_POINTER			1
#define STATE_WRONG_BUFFER_LEN 	2
#define STATE_DISABLED				  3
#define STATE_NO_VALUE				  4
#define STATE_NO_LOGGING			  5
#define STATE_NO_CHANGE				  6

/*  --- Schrittkettenbeschreibung ---------------------------------------------------------*/
#define	 UDP_SCHRITT_OPEN_TXT					        "Ports öffnen          "
#define  UDP_SCHRITT_RECEIVE_SET_TXT			    "Set Recv. Telegr.     "
#define	 UPP_SCHRITT_RECEIVE_TXT				      "Recv. Telegr.         "
#define  UDP_SCHRITT_GET_CONN_TXT				      "Suche nächsten Client "
#define	 UDP_SCHRITT_SEND_CONN_SET_TXT		    "Set Send Conn         "	/* Senden von Verbindungstelegrammen CONN		*/
#define	 UDP_SCHRITT_SEND_CONN_TXT				    "Send Conn             "	/* Warten bis Senden erfolgreich CONN	 	*/
#define  UDP_SCHRITT_RECEIVE_CONN_SET_TXT	    "Set Recv. Conn        "	/* Empfangen der CONN Antworttelegramme einrichten */
#define	 UDP_SCHRITT_RECEIVE_CONN_TXT			    "Recv. Conn            "	/* Warten bis Empfangen erfolgreich CONN 	*/
#define	 UDP_SCHRITT_SEND_OPEN_SET_TXT		    "Set Send Open         "	/* Einrichten u. Senden von PV - Open Telegrammen  		*/
#define	 UDP_SCHRITT_SEND_OPEN_TXT				    "Send Open             "	/* Warten bis Senden von PV - Open Telegrammen  erfolgreich		*/
#define  UPD_SCHRITT_RECEIVE_OPEN_SET_TXT	    "Set Recv. Open        "	/* Einrichten des Empfangs von PV - Open Responser Telegrammen	*/
#define  UPD_SCHRITT_RECEIVE_OPEN_TXT			    "Recv. Open            "	/* Warten bis Empfang  von PV - Open Telegrammen	*/
#define  UDP_SCHRITT_CLIENT_LINK_SET_TXT	    "Set Send Link         "	/* LINK - Telegramm einrichten */
#define  UDP_SCHRITT_CLIENT_LINK_TXT			    "Send Link             "	/* Warten bis Senden des Link Telegrammes erfolgreich	*/
#define  UDP_SCHRITT_CLIENT_RECEIVE_SET_TXT		"Set Recv. Link        "	/* Einrichten des Empfangs des Link - Response Telegrammes*/
#define  UDP_SCHRITT_CLIENT_RECEIVE_TXT		    "Recv. Link            "	/* Warten bis Empfangen des Link Telegrammes erfolgreich */	
#define	 UDP_SCHRITT_REPEAT_LAST_TXT			    "Repeat Tel            "	/* Wiederholen von beliebigen Telegrammen 	*/
#define  UDP_SCHRITT_ERROR_TXT					      "Error Step            "	/* Fehlerschritt, Anmeldung gescheitert, Schrittschaltwerk inaktiv */
#define	 UDP_SCHRITT_NEUANMELDUNG_TXT			    "Liste ok, Neu         "	/* komplette Liste wurde abgearbeitet */
#define	 UDP_SCHRITT_NEUANMELDUNG_SET_TXT	    "Set Neu               "	/* Vorinitialisieren für die Abarbeitung der kompletten Serverliste*/
#define	 UDP_SCHRITT_CHECK_VARIABLES_TXT	    "Check wrong PV        "	/* nach einzelnen Variablen, welche Fehlerhaft sind, wird gesucht */
#define	 UDP_SCHRITT_CHECK_VARIABLES_SET_TXT	"Set Check wrong PV    "	/* nach einzelnen Variablen, welche Fehlerhaft sind, wird gesucht */

#define	 EVENT_SCHRITT_SET_TXT				 	      "Set Event Recv.       "	/* Konfiguration des Empfangs von Eventtelegrammen */
#define	 EVENT_SCHRITT_TXT					 	        "Event Recv.           "	/* Eventauswertung */
#define	 EVENT_SCHRITT_RESPONSE_TXT		 	 	    "Event Response        "	/* Eventauswertung Wiederholung */

#define	 LIFE_CHECK_SEND_TXT				 	        "Life Send             "	/* Senden des Lifechecks	    */
#define	 LIFE_CHECK_SEND_OK_TXT				 	      "Life Send ok          "	/* Senden ok?					*/	
	


/* ******************************** Bur Logger AsEvent Functions ****************************************** */
DINT CyclicBurLoggerCreate(Logger_typ* pLogger, char* pLoggerName);
DINT WriteLoggerMessage(Logger_typ* pLogger, UINT ErrNumber, STRING* pMessage, DINT AdditionalStatus, USINT Severity);

#endif
