/*****************************************************************************************************************/
/*										Header File für UDP	Server				´							     */	
/*---------------------------------------------------------------------------------------------------------------*/
/*						Taskname: server																		 */
/*						Dateiname: ptpserv.c																	 */
/*						Autor:	B&R																				 */
/*						Erstelldatum: Mai 2002																	 */
/*						Classtime: 10 - 30000 ms																 */
/*---------------------------------------------------------------------------------------------------------------*/
/*	Funktion:																					 */
/*					- Konstanten für Schrittschaltwerke											 */
/*					- Konstanten für Zustände, Fehler										 	 */
/*					- Anlegen der Variablen, Eventauswertung, Lifecheck						 */
/*					- Fehler u. Statuskonstanten													 */
/*					- PV u. Clientliste - Strukturen													 */
/*																								 */
/*****************************************************************************************************************/
#ifndef PTPSERV_H_
#define PTPSERV_H_

#include "allgserv.h"

/*****************************************************************************************************************/
/*		Konstanten für Schrittschaltwerk Anmeldung																 */
#define  UDP_SCHRITT_INIT               0
#define	 UDP_SCHRITT_OPEN				        1		/* öffnen Port zu UDP Treiber 		 		*/
#define  UDP_SCHRITT_OPTIONS_1_SET	    2		/* setzen von diversen Optionen	1			*/
#define  UDP_SCHRITT_OPTIONS_1		      3		/* setzen von diversen Optionen	1			*/
#define  UDP_SCHRITT_OPTIONS_2_SET	    4		/* setzen von diversen Optionen	2			*/
#define  UDP_SCHRITT_OPTIONS_2		      5		/* setzen von diversen Optionen	2			*/
#define  UDP_SCHRITT_RECEIVE_SET	      10		/* Empfangen von Telegrammen initialisieren	*/
#define	 UDP_SCHRITT_RECEIVE			      11		/* Empfangen von Telegrammen         		*/
#define	 UDP_SCHRITT_SEND_CONN_SET	    12		/* Senden von Verbindungstelegrammen 		*/
#define	 UDP_SCHRITT_SEND_CONN		      13
#define	 UDP_SCHRITT_SEND_OPEN		      14		/* Senden von PV - Open Telegrammen  		*/
#define	 UDP_SCHRITT_REPEAT_LAST	      15		/* Wiederholen von beliebigen Telegrammen 	*/
#define  UDP_SCHRITT_SEND_LINK_ACK	    16

#define  NO_CLIENT_ENTRY				        0
#define  TIMER_INTERVAL_FKT			        100
#define  TIMEOUT_RETRY					        3
#define  TIMEOUT_DELETE_CLIENT		      300000 /* 5 Minuten */

/*****************************************************************************************************************/
/*		Konstanten für Schrittschaltwerk Eventauswertung		*/
#define	 UDP_EVENT_SEND_INIT		      0												 
#define	 UDP_EVENT_CHECK_PIPE		      1
#define  UDP_EVENT_SEND_SET			      2
#define  UDP_EVENT_SEND				        3
#define  UDP_EVENT_RECEIVE_INIT	      10
#define	 UDP_EVENT_RECEIVE_SET	      11
#define	 UDP_EVENT_RECEIVE			      12
#define  UDP_EVENT_CHECK_TIME		      13
#define  UDP_EVENT_IDLE				        20

/*****************************************************************************************************************/
/*		Konstanten für Schrittschaltwerk Lebensüberwachung														 */
#define  LIFE_CHECK_IDLE					    0
#define	 LIFE_CHECK_RESPONSE			    1
#define	 LIFE_CHECK_RESPONSE_WAIT	    2

/*****************************************************************************************************************/
/*									 Status von Client */
#define	 STATUS_UNLINK			          0
#define  STATUS_OPEN				          1
#define  STATUS_EVENT				          2

/* --- Status von Open	------------------------------------------------------------------- */
#define  STATUS_SERV_OK					      0	/* Variable gefunden u. ok						*/
#define	 STATUS_SERV_NO_VAR				    1	/* Variable nicht auf Server gefunden!			*/
#define  STATUS_SERV_NO_OPEN			    2	/* Server kann nicht konf. werden				*/
#define  STATUS_SERV_NO_CLIENT_SPACE	3	/* kein Platz f. Client eintrag					*/
#define  STATUS_SERV_NO_PV_SPACE		  4	/* kein Platz f. PV eintrag						*/
#define	 STATUS_SERV_VAR_ILL_LENGTH		5	/* PV hat ungültige	Länge 						*/

/* --- Klartexte für Open -------------------------------------------------------------------------------------- */
#define  STATUS_OK_TXT				        "ok"						            /* Variable gefunden u. ok						*/
#define	 STATUS_NO_VAR_TXT		        "PV nicht gefunden"			    /* Variable nicht auf Server gefunden!			*/
#define  STATUS_NO_OPEN_TXT		        "Server nicht konfiguriert"	/* Server kann nicht konf. werden				*/


/*					Status von ClientConnectRequest -> bei CONN 												 */
#define  STATUS_CONNECT_REQUEST_INIT			  0		/* ist nicht belegt 									 */
#define  STATUS_CONNECT_REQUEST_ENTRY			  1		/* Neueintrag in Clientliste							 */
#define  STATUS_CONNECT_REQUEST_RECONNECT		2	  /* Löschen des Clients aus Clientliste und Neueintrag	 */

#define LIFE_OK_TXT					          "ok"
#define LIFE_CLIENT_MISSING_TXT		    "Server nicht gefunden"
#define LIFE_CONN_TXT				          "Anmeldung läuft"	

#define FUB_OK					              0
#define FUB_BUSY				              65535

//#define IP_ADR_LEN			19

#define  SERVER_TASK          "server"

#endif
