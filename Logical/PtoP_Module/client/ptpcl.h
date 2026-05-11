/*****************************************************************************************************************/
/*										Header File für UDP	Client				´							     */	
/*---------------------------------------------------------------------------------------------------------------*/
/*						Taskname: client																		 */
/*						Dateiname: ptpcl.h																		 */
/*						Autor:	B&R																				 */
/*						Erstelldatum: Mai 2002																	 */
/*						Classtime: 10 - 30000 ms																 */
/*---------------------------------------------------------------------------------------------------------------*/
/*	Funktion:																									 */
/*					- Speicherverwaltungsstrukturen													     		 */
/*					- Monitorfunktionen														 					 */
/*					- Konstanten f. Schrittschaltwerke 															 */
/*					- Fehler u. Statuskonstanten														 		 */
/*																												 */
/*****************************************************************************************************************/
#ifndef PTPCL_H_
#define PTPCL_H_


#include "..\\client\\allgclnt.h" 

/* --- Schrittschaltwerk - Konstanten --------------------------------------------------------- */

#define  UDP_SCHRITT_INIT                 0   /* Schrittschaltwerk Initialisieurng  */
#define	 UDP_SCHRITT_OPEN					        1		/* öffnen Port zu UDP Treiber 		 		*/
#define  UDP_SCHRITT_OPTIONS_1_SET			  2		/* setzen von diversen Optionen	1			*/
#define  UDP_SCHRITT_OPTIONS_1				    3		/* setzen von diversen Optionen	1			*/
#define  UDP_SCHRITT_OPTIONS_2_SET			  4		/* setzen von diversen Optionen	2			*/
#define  UDP_SCHRITT_OPTIONS_2				    5		/* setzen von diversen Optionen	2			*/
#define  UDP_SCHRITT_RECEIVE_SET			    10		/* Empfangen von Telegrammen initialisieren	*/
#define	 UDP_SCHRITT_RECEIVE				      11		/* Empfangen von Telegrammen         		*/
#define	 UDP_SCHRITT_SEND_CONN_SET			  13		/* Senden von Verbindungstelegrammen CONN		*/
#define	 UDP_SCHRITT_SEND_CONN				    14		/* Warten bis Senden erfolgreich CONN	 	*/
#define  UDP_SCHRITT_RECEIVE_CONN_SET		  15		/* Empfangen der CONN Antworttelegramme einrichten */
#define	 UDP_SCHRITT_RECEIVE_CONN			    16		/* Warten bis Empfangen erfolgreich CONN 	*/
#define	 UDP_SCHRITT_SEND_OPEN_SET			  17		/* Einrichten u. Senden von PV - Open Telegrammen  		*/
#define	 UDP_SCHRITT_SEND_OPEN				    18		/* Warten bis Senden von PV - Open Telegrammen  erfolgreich		*/
#define  UPD_SCHRITT_RECEIVE_OPEN_SET		  19		/* Einrichten des Empfangs von PV - Open Responser Telegrammen	*/
#define  UPD_SCHRITT_RECEIVE_OPEN			    20		/* Warten bis Empfang  von PV - Open Telegrammen	*/
#define  UDP_SCHRITT_CLIENT_LINK_SET		  21		/* LINK - Telegramm einrichten */
#define  UDP_SCHRITT_CLIENT_LINK			    22		/* Warten bis Senden des Link Telegrammes erfolgreich	*/
#define  UDP_SCHRITT_CLIENT_RECEIVE_SET	  23		/* Einrichten des Empfangs des Link - Response Telegrammes*/
#define  UDP_SCHRITT_CLIENT_RECEIVE			  24		/* Warten bis Empfangen des Link Telegrammes erfolgreich */	
#define	 UDP_SCHRITT_REPEAT_LAST			    25		/* Wiederholen von beliebigen Telegrammen 	*/
#define  UDP_SCHRITT_ERROR					      26		/* Fehlerschritt, Anmeldung gescheitert, Schrittschaltwerk inaktiv */
#define	 UDP_SCHRITT_NEUANMELDUNG			    27		/* komplette Liste wurde abgearbeitet */
#define	 UDP_SCHRITT_NEUANMELDUNG_SET		  28		/* Vorinitialisieren für die Abarbeitung der kompletten Serverliste zur Anmeldung*/
#define	 UDP_SCHRITT_CHECK_VARIABLES		  29		/* nach einzelnen Variablen, welche Fehlerhaft sind, wird gesucht */
#define	 UDP_SCHRITT_CHECK_VARIABLES_SET	30		/* nach einzelnen Variablen, welche Fehlerhaft sind, wird gesucht */

#define	 GET_HOST_INIT						        0		/* Init für Schrittschaltwerküberwachung		*/
#define	 GET_HOST							            1		/* abchecken, ob Ip vorhanden		*/
#define	 GET_HOST_FUB						          2		/* weitere Aufrufe der Funktion		*/

/*****************************************************************************************************************/

#define	 EVENT_SCHRITT_INIT					      0		/* Init für Schrittschaltwerküberwachung  */
#define	 EVENT_SCHRITT_SET					      1		/* Konfiguration des Empfangs von Eventtelegrammen */
#define	 EVENT_SCHRITT						        2		/* Eventauswertung */
#define	 EVENT_SCHRITT_RESPONSE			 	    3		/* Eventauswertung Wiederholung */
#define	 EVENT_SCHRITT_INACTIVE			 	  100	  /* Eventauswertung Inactive Step */

#define	 LIFE_CHECK_INIT					        0		/* Init für Schrittschaltwerküberwachung	    */
#define	 LIFE_CHECK_SEND					        1		/* Senden des Lifechecks	    */
#define	 LIFE_CHECK_SEND_OK					      2		/* Senden ok?					*/	
#define	 LIFE_CHECK_INACTIVE				    100	/* Lifecheck Inactive Step	    */

/* --- Status von Open	------------------------------------------------------------------- */
#define  STATUS_SERV_OK						        0	/* Variable gefunden u. ok						*/
#define	 STATUS_SERV_NO_VAR					      1	/* Variable nicht auf Server gefunden!			*/
#define  STATUS_SERV_NO_OPEN				      2	/* Server kann nicht konf. werden				*/
#define  STATUS_SERV_NO_CLIENT_SPACE		  3	/* kein Platz f. Client eintrag					*/
#define  STATUS_SERV_NO_PV_SPACE			    4	/* kein Platz f. PV eintrag						*/
#define	 STATUS_SERV_VAR_ILL_LENGTH			  5	/* PV hat ungültige	Länge 						*/

// #define  HOST_NAME_LEN						33	/* 32 Zeichen 									*/
//#define  IP_ADR_LEN							18  /* 17 Zeichen f. Ip Adr. + Abschluss			*/ 

#define  CLIENT_TASK          "client"

/******************************************************************************************************************/
/*											Datenverwaltung														  */
/******************************************************************************************************************/

/******************************************************************************************************************/

#endif
