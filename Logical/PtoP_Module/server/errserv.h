/*****************************************************************************************************************
*										Header File für UDP	Server
*---------------------------------------------------------------------------------------------------------------
*						Taskname: server							
*						Dateiname: errserv.h						
*						Autor:	B&R								
*						Erstelldatum: Jänner 2003					
*						Classtime: 10 - 30000 ms					
*--------------------------------------------------------------------------------------------------------------- *
*	Funktion:														
*					- Konstanten für Fehlerauswertung				
*					- Fehlerstruktur									
*																	
*****************************************************************************************************************/
#ifndef ERRSERV_H_
#define ERRSERV_H_

//#define ERROR_LIST_LENGTH				30			/* Größe der Fehlerliste		*/

/*****************************************************************************************************************/
#define ERROR_SCHRITT_OPEN				1			/* Fehler tritt bei Anmeldung der Variablen auf */
#define ERROR_SCHRITT_EVENT				2			/* Fehler tritt bei Eventauswertung auf 		*/
#define ERROR_SCHRITT_LIFE				3			/* Fehler tritt bei Lifecheck auf				*/

#define ERROR_REC_OPEN					  11			/* Fehler bei Empfang OPEN - Telegramm	*/
#define ERROR_REC_CONN					  12			/* Fehler bei Empfang CONN - Telegramm	*/
#define ERROR_REC_LINK					  13			/* Fehler bei Empfang LINK - Telegramm	*/

/*****************************************************************************************************************/
/*									Fehlernummern Variablenverwaltung											 */
/*****************************************************************************************************************/
#define  ERR_NO								    0			/* Telegramm ok */
#define  ERR_CRC_FAILURE					1			/* Fehler bei CRC */
#define	 ERR_VAR_EXISTS					  2			/* Variable existiert bereits */ 
#define  ERR_LINK_FAILURE					3			/* Fehler bei Linken des Clients -> kein Speicher für Variable,...*/
#define  ERR_VAR_ILL_LENGTH				4			/* Fehler: Variablenlänge */
#define	 ERR_VAR_NEXISTS					5			/* Variable existiert auf Server nicht */ 
#define  ERR_CONN_FAILURE					6

#define  ERR_STAT_NO_OPEN				  11			/* Fehler: Server ist nicht auf OPEN*/
#define  ERR_STAT_TICK						12			/* Systemtick konnte nicht ausgelesen werden */

#define  ERR_NO_CLIENT_SPACE			21			/* bei Anmeldung wurde kein Platz für Client gefunden */
#define  ERR_NO_PV_SPACE					22			/* bei Anmeldung wurde kein Platz für neuen PV gefunden */

/* Fehlernummern Clientverwaltung Anmeldephase */
#define  ERR_FAILED_CONN_SEND			50			/* Fehler beim Senden des CONN - Telegrammes (IP - vorhanden?)	*/
#define  ERR_FAILED_CONN_RECEIVE	51			/* Fehler beim Empfangen des CONN - Telegrammes (Timeout)*/
#define  ERR_FAILED_LINK_SEND			52			/* Fehler beim Senden des CONN - Telegrammes (IP - vorhanden?)	*/
#define  ERR_FAILED_LINK_RECEIVE	53			/* Fehler beim Empfangen des CONN - Telegrammes (Timeout)*/

#define	 ERR_FAILED_SERVER_RESP		60			/* keine Antwort von Server bei Lifecheck */	

/******************************************************************************************************************/
/*								 UDP Funktionsblock Fehlernummern 						 */
/******************************************************************************************************************/
#define UDP_ERR_BUSY						  65535		/* siehe UDP Hilfe B&R			*/	



#endif
