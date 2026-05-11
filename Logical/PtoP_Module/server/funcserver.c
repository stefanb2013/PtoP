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

#include "funcserver.h"

/* ------------------------------------------------------------------------------------------------------------- */
/* --- komplette Server - Liste durchsuchen und Ip numerisch in Ip - String wandeln              --- */
/* ------------------------------------------------------------------------------------------------------------- */

void ConvertIpAdrListToAscii(
UDINT* pClientListStartAdr)   /* Startadresse der Serverliste */
{
UINT iClient = 0;
ClientListeEntry_typ* pActClient = NULL;

  for (iClient = 0; iClient < MAX_ANZAHL_CLIENTS; iClient++)
  {
    pActClient = (ClientListeEntry_typ*) pClientListStartAdr + iClient;
    
    /* --- Converts a network address to dotted decimal notation */
    //inet_ntoa_b(pActClient->IPAdresse,  (UDINT) &pActClient->IpAdr);
    ethInetNtoa(pActClient->IPAdresse,  (UDINT) &pActClient->IpAdr);
  } 
}


/* ------------------------------------------------------------------------------------------------------------- */
/* --- einen Server - Eintrag bearbeiten: Ip numerisch in Ip - String wandeln                  --- */
/* ------------------------------------------------------------------------------------------------------------- */

UINT ConvertIpAdrToAscii(
UDINT* pClientAdr)
{
  ClientListeEntry_typ* pActClient = NULL;
  UINT uiLenInet;
  
  uiLenInet = 0;
  
  if (pClientAdr == NULL)
  {
    return uiLenInet;
  }

  pActClient = (ClientListeEntry_typ*) pClientAdr;
  
  /* --- Converts a network address to dotted decimal notation */
  if (pActClient->IPAdresse != 0)
  {
    //uiLenInet = inet_ntoa_b(pActClient->IPAdresse, (UDINT) &pActClient->IpAdr);
    if (ethInetNtoa(pActClient->IPAdresse, (UDINT) &pActClient->IpAdr) == 0)
    {
      uiLenInet = brsstrlen((UDINT) pActClient->IpAdr);
    }
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
  //inet_ntoa_b(IpAdrTarget,  (UDINT) parIpAdr);
  ethInetNtoa(IpAdrTarget, (UDINT) parIpAdr);
}


/* ------------------------------------------------------------------------------------------------------------- */
/*  einen Server - Eintrag bearbeiten: Ip String in Ip - String numerisch wandeln     */
/* ------------------------------------------------------------------------------------------------------------- */

void ConvertAsciiToIpAdr(
UDINT* pIpAdrSourceTest, 
UDINT* parIpAdrTest)
{
  //*pIpAdrSourceTest = inet_addr ((UDINT) parIpAdrTest);
  ethInetAton((UDINT) parIpAdrTest, (UDINT) pIpAdrSourceTest);  
}
