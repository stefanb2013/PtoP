/*****************************************************************************************************************/
/*										Header File für PtoP Library			´							     */	
/*---------------------------------------------------------------------------------------------------------------*/
/*						Dateiname: ptoplibinc.h																	 */
/*						Autor:	B&R																				 */
/*						Erstelldatum: August 2005																 */
/*						Classtime: 10 - 30000 ms																 */
/*---------------------------------------------------------------------------------------------------------------*/
/*	Funktion:																									 */
/*																												 */
/*****************************************************************************************************************/
#ifndef PTOPLIBINC_H
#define PTOPLIBINC_H

#include <sys_lib.h>
#include <fileio.h>
#include <astime.h>
#include "ptoplib.h"

#define FALSE	0
#define TRUE	1

/* --- Initialisierung --------------------------------------- */
#define STATE_PV_LIFE_NOT_FOUND  		1
#define STATE_PV_RESTART_NOT_FOUND  	2
#define STATE_PV_PVADR_NOT_FOUND		3
#define STATE_PV_CLIENTADR_NOT_FOUND	4


/* --- Scbrittschaltwerk ------------------------------------ */
#define SERV_CLIENTS_IDLE                   0
#define SERV_CLIENTS_GET_PV_ADR_1           1
#define SERV_CLIENTS_GET_PV_ADR_2           3
#define SERV_CLIENTS_ORGANIZE_FILE          10
#define SERV_CLIENTS_CREATE_FILE            11
#define SERV_CLIENTS_CREATE_CL_OVERVIEW     12
#define SERV_CLIENTS_WRITE_CL_OVERVIEW      13
#define SERV_CLIENTS_CREATE_PV_LIST         14
#define SERV_CLIENTS_WRITE_PV_LIST          15
#define SERV_CLIENTS_GEN_CLIENT_LIST        20
#define SERV_CLIENTS_CLOSE_FILE             30
#define SERV_CLIENTS_ERR                    50


typedef struct
{	
	UDINT udLifeCheckAdr;
	UDINT udLifeCheckLen;	
	UDINT udRestartClientAdr;
	UDINT udRestartClientLen;
	UINT  uiMaxServer;
	UDINT udPvAdr;
	UDINT udClientAdr;
}PvAdmin_typ;


#endif
