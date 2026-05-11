/*****************************************************************************************************************/
/*										Header File für UDP	Client											     */
/*---------------------------------------------------------------------------------------------------------------*/
/*						Taskname: client																		 */
/*						Dateiname: errclnt.h																	 */
/*						Autor:	B&R																				 */
/*						Erstelldatum: Jänner 2003																 */
/*						Classtime: 10 - 30000 ms																 */
/*---------------------------------------------------------------------------------------------------------------*/
/*	Funktion:																									 */
/*					- Konstanten für Fehlerauswertung												     		 */
/*					- Fehlerstruktur														 					 */
/*																												 */
/*****************************************************************************************************************/
#ifndef ERRCLNT_H_
#define ERRCLNT_H_

#define  ERROR_SCHRITT_OPEN					1		/* Fehler tritt bei Anmeldung der Variablen auf */
#define  ERROR_SCHRITT_EVENT				2		/* Fehler tritt bei Eventauswertung auf 		*/
#define  ERROR_SCHRITT_LIFE					3		/* Fehler tritt bei Lifecheck auf				*/
#define  ERROR_SCHRITT_GET_PV				4		/* PV nicht auf Client vorhanden				*/
#define  ERROR_SCHRITT_LIFE_RECV			5		/* Empfangen von Lifechcheck Telegrammen		*/
#define  ERROR_SCHRITT_GET_IP				6		/* Ermittlung von Ip-Adressen					*/

/*****************************************************************************************************************/
/*									Fehlernummern Variablenverwaltung											 */
/*****************************************************************************************************************/
#define	 ERR_NO							  	0
#define	 ERR_BCC							  1
#define  ERR_TIMEOUT						2
#define  ERR_ILLEGAL_LENGTH 		3
#define  ERR_OPEN_FAILED				4
#define  ERR_VALUE_TO_LONG			5
#define  ERR_VALUE_NOTX_SERVER 	6
#define	 ERR_OPEN_SEND_FAILED		7
#define  ERR_ILLEGAL_PV_INDEX		8
#define	 ERR_NULL_POINTER_CLNT	9

#define  ERR_NO_CLIENT_SPACE		21			/* bei Anmeldung wurde kein Platz für Client gefunden */
#define  ERR_NO_PV_SPACE				22			/* bei Anmeldung wurde kein Platz für neuen PV gefunden */

/*****************************************************************************************************************/
/* Statustexte																									 */
#define  ERR_NO_TXT						      "ok"
#define  ERR_BCC_TXT						    "Checksumme"
#define  ERR_TIMEOUT_TXT				    "Timeout"
#define  ERR_ILLEGAL_LENGTH_TXT		  "PV Laenge ill."
#define  ERR_OPEN_FAILED_TXT			  "Open gescheitert"
#define  ERR_VALUE_TO_LONG_TXT			"PV zu lang"
#define  ERR_VALUE_NOTX_SERVER_TXT	"PV nicht vorhanden"
#define  ERR_OPEN_SEND_FAILED_TXT	  "keine Antwort"
#define  ERR_ILLEGAL_PV_INDEX_TXT		"ungültiger PV Idx"

#define  ERR_NO_CLIENT_SPACE_TXT		"kein Client Memory"
#define  ERR_NO_PV_SPACE_TXT			  "kein PV Memory"
#define  ERR_NO_PV_UNDEF_TXT        "undefined"

/*****************************************************************************************************************/
/* Fehlernummern Clientverwaltung Anmeldephase */
#define  ERR_FAILED_CONN_SEND				50			/* Fehler beim Senden des CONN - Telegrammes (IP - vorhanden?)	*/
#define  ERR_FAILED_CONN_RECEIVE			51			/* Fehler beim Empfangen des CONN - Telegrammes (Timeout)*/
#define  ERR_FAILED_LINK_SEND				52			/* Fehler beim Senden des LINK - Telegrammes (IP - vorhanden?)	*/
#define  ERR_FAILED_LINK_RECEIVE			53			/* Fehler beim Empfangen des LINK - Telegrammes (Timeout)*/

#define	 ERR_FAILED_SERVER_RESP			60			/* keine Antwort von Server bei Lifecheck */	

/******************************************************************************************************************/
#define  FUB_OK								0			/* Funktionsblockaufruf ok*/
#define  FUB_NO_OPEN					    	2			/* Server Status ist nicht auf OPEN	*/
#define  FUB_BUSY							0xFFFF		    /* FUB braucht noch einen Aufruf */

#define  SERVER_RDY							0			/* Server ist zur Anmeldung bereit */
#define  SERVER_NRDY						   1			/* Server ist nicht zur Anmeldung bereit */
#define  SERVER_LNK_ERROR					3			/* Fehler beim Linken */
#define  SERVER_CONN_FAILURE				6

/*****************************************************************************************************************/
/* Vorbelegung der Statusvariable bei Anmeldung an Server 														 */
#define  SERVER_CONN_DEFAULT				0xFF			/* Vorbelegung	*/
#define  SERVER_CONN_RUNTIME				0
#define  SERVER_CONN_REGISTRATION			1			/* Anmeldung zur Zeit aktiv 							*/
#define  SERVER_CONN_START					2			/* OPEN Telegramm - Timeout -> Anmeldung gescheitert	*/
#define  SERVER_CONN_OK						3			/* Anmeldung erfolgreich								*/	

/******************************************************************************************************************/
/* 								Status PV Anmeldung - OPEN														  */
/******************************************************************************************************************/
#define	 STAT_DEFAULT					0			/* Status DEFAULT -> nicht belegt			*/
#define	 STAT_PV_NOT_AVAILABLE		1			/* PV am Server nicht vorhanden				*/
#define	 STAT_TIMEOUT					2			/* Timeout bei Kommunikation mit Server 	*/
#define	 STAT_BCC						3			/* Checksummenfehler 						*/
#define	 STAT_PV_ILL_LENGTH			4			/* Variable hat nicht die erwartete Länge 	*/
#define	 STAT_PV_INDEX					5			/* PV Index ist 0, anstatt > 0 				*/
#define  STAT_OK							6			/* PV am Server vorhanden und angemeldet 	*/
#define  STAT_PV_CL_NOT_AVAILABLE	7			/* PV am Client nicht vorhanden 			*/
#define  STAT_SERV_NOT_OK				8			/* Fehler am Server 						*/
#define  STAT_VAL_IS_NULL				9			/* Nullpointer bzw.  0 - Länge der Variable	*/

/******************************************************************************************************************/
/*								 UDP Funktionsblock Fehlernummern 												  */
/******************************************************************************************************************/
#define UDP_ERR_BUSY						0xFFFF		/* siehe UDP Hilfe B&R						*/	
#define ETH_ERR_FBETH_TCPMANTIO		14893		/* Ethernet Bibliothek Manager (früher TcpIpMgr) Timeout (200ms).*/	

/******************************************************************************************************************/
/*											Fehlerliste														  	  */
/******************************************************************************************************************/
//in client.typ

#endif
