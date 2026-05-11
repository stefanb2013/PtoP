/*****************************************************************************************************************/
/*										Header File für UDP	Server				´							     */	
/*---------------------------------------------------------------------------------------------------------------*/
/*						Taskname: server																		 */
/*						Dateiname: allgem.h																		 */
/*						Autor:	B&R																				 */
/*						Erstelldatum: Mai 2002																	 */
/*						Classtime: 10 - 30000 ms																 */
/*---------------------------------------------------------------------------------------------------------------*/
/*	Funktion:																									 */
/*					- allgem. Konstanten															     		 */
/*					- Serverparametrierung													 					 */
/*					- Arbeitsstrukturen																			 */
/*																												 */
/*****************************************************************************************************************/
#ifndef ALLGSERV_H_
#define ALLGSERV_H_

#include "..\\allgem.h"

#define  DATA_OBJ_MEM_NAME_SERVER	        "ptopserv"
#define  LOGGER_NAME_SERV                 "_PtoPServ"

#define  TIMEOUT_LIFE_RETRY					  	3		/* Anzahl der Versuche nach Timeout 	*/

/*****************************************************************************************************************/
/* 		Parameter für Telegrammhandling																			 */
//#define	 MAX_ANZAHL_CLIENTS		  100		
//#define	 MAX_CLIENT_VARIABLEN		1000		/* max. Anzahl der Variablen, welche auf einem Server verwaltet werden können */
#define	 MAX_RETRY_TELEGRAM		  1 		  /* Anzahl der Wiederholungen senden Telegramm */
#define  MAX_PV_LOOP_CNT			  50		  /* wieviele Schleifendurchläufe (PV mit allen Clients!) sind je Taskzyklus erlaubt */

#endif
