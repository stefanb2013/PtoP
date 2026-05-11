/*****************************************************************************************************************/
/*                    C - File für allgemeine Funktionen    ´                  */ 
/*---------------------------------------------------------------------------------------------------------------*/
/*            Taskname: client bzw. server                               */
/*            Dateiname: functions.c                                   */
/*            Autor:  B&R                                        */
/*            Erstelldatum: Juni 2007                                  */
/*            Classtime: 10 - 30000 ms                                 */
/*---------------------------------------------------------------------------------------------------------------*/
/*  Funktion:                                                  */
/*                                                         */
/*****************************************************************************************************************/

#ifdef _DEFAULT_INCLUDES
#include <AsDefault.h>
#endif

#include "funcclient.h"

/* ------------------------------------------------------------------------------------------------------------- */
/* --- komplette Server - Liste durchsuchen und Ip numerisch in Ip - String wandeln              --- */
/* ------------------------------------------------------------------------------------------------------------- */

void ConvertIpAdrListToAscii(
UDINT* pClientListStartAdr)   /* Startadresse der Serverliste */
{
UINT iServ = 0;
Client_List_Typ* pActServer = NULL;

  for (iServ = 0; iServ < MAX_SERVER; iServ++)
  {
    pActServer = (Client_List_Typ*) pClientListStartAdr + iServ;
    
    /* --- Converts a network address to dotted decimal notation */
    ethInetNtoa(pActServer->IPAdresse, (UDINT) &pActServer->IpAdr);
    //inet_ntoa_b(pActServer->IPAdresse,  (UDINT) &pActServer->IpAdr);
  } 
}


/* ------------------------------------------------------------------------------------------------------------- */
/* --- einen Server - Eintrag bearbeiten: Ip numerisch in Ip - String wandeln                  --- */
/* ------------------------------------------------------------------------------------------------------------- */

UINT ConvertIpAdrToAscii(
UDINT* pClientAdr)
{
Client_List_Typ* pActServer = NULL;
UINT uiLenInet;

  uiLenInet = 0;
  if (pClientAdr == NULL)
  {
    return uiLenInet;
  }
  
  pActServer = (Client_List_Typ*) pClientAdr;
  
  /* --- Converts a network address to dotted decimal notation */
  if (pActServer->IPAdresse != 0)
  {
    /* --- Converts a network address to dotted decimal notation */
    ethInetNtoa(pActServer->IPAdresse, (UDINT) &pActServer->IpAdr);
    
  }
  
  return uiLenInet;
}


/* ------------------------------------------------------------------------------------------------------------- */
/* --- einen Server - Eintrag bearbeiten: Ip numerisch in Ip - String wandeln                  --- */
/* ------------------------------------------------------------------------------------------------------------- */

void ConvertIpAdrToAsciiX(
UDINT IpAdr,      /* Ip - Adresse (numerisch ) */
UDINT* parIpAdr)    /* Ip - Adresse (String)  */
{

UDINT IpAdrTarget = 0;
UINT i, j = 0;
USINT  arIpCntScr[4];
USINT  arIpCntTrg[4];

  brsmemcpy((UDINT) &arIpCntScr, (UDINT) &IpAdr, 4);
  
  for (i = 0, j = 3; i < 4; i++, j--)
  {
    arIpCntTrg[i] = arIpCntScr[j];
  }
  
  brsmemcpy((UDINT) &IpAdrTarget, (UDINT) &arIpCntTrg, 4); 

  /* --- Converts a network address to dotted decimal notation */
  ethInetNtoa(IpAdrTarget, (UDINT) parIpAdr);
  //inet_ntoa_b(IpAdrTarget,  (UDINT) parIpAdr);
  
}
