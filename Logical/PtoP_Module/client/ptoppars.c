/*****************************************************************************************************************/
/*                    C - File für UDP / Client Parser    ´                  */ 
/*---------------------------------------------------------------------------------------------------------------*/
/*            Taskname: ptppars                                    */
/*            Dateiname: ptppars.c                                   */
/*            Autor:  B&R                                        */
/*            Erstelldatum: July 2002                                  */
/*            Classtime: 10 - 30000 ms                                 */
/*---------------------------------------------------------------------------------------------------------------*/
/*  Funktion:                                                  */
/*        * Auslesen des Datenmoduls 'ptop.dat' zur Ermittlung der Server                  */
/*        * Ablegen der konfigurierten Werte in den zuvor allokierten Speicher               */
/*                                                         */
/*****************************************************************************************************************/

#ifdef _DEFAULT_INCLUDES
#include <AsDefault.h>
#endif

#include "..\\processor.h"
#include <bur\plc.h>
#include <bur\plctypes.h>
#include <dataobj.h>
#include <string.h>
#include <sys_lib.h>
#include "allgclnt.h"
#include "ptpcl.h"
#include "ptoppars.h"
#include "allgclnt.h"

/**************************************************************************************************************
* Suche nach Hostname
**************************************************************************************************************/

BOOL SearchForName(
USINT** pData,      /* --- Pointer auf DM-Pointer   */
USINT* pCmpStr      /* --- Pointer auf Vergleichsstring */
)
{
UINT uiCmpLen = 0;
USINT* pCmp = NULL;
DINT sdCmp = 0;
BOOL bRetVal  = FALSE;

  pCmp = *pData;
  uiCmpLen = brsstrlen((UDINT) pCmpStr);

  sdCmp = brsmemcmp((UDINT)pCmp, (UDINT) pCmpStr, uiCmpLen);
  
  if (sdCmp)
  {
    bRetVal = FALSE;
  }
  else
  {
    /* --- Pointer korrigieren */
    *pData += uiCmpLen;
    bRetVal = TRUE;
  }
  
  return bRetVal;
}


/**************************************************************************************************************
* Parsen des Datenmoduls
**************************************************************************************************************/

BOOL ParsenDataModul (
UDINT* pStartAdrPv,     /* Startadresse der Variablenverwaltung */
UDINT* pStartAdrServ,     /* Startadresse der Serververwaltung */
USINT* pStartAdrDM,     /* Startadresse des ptop Datenobjektes  */
UDINT udLenDM,
UINT* puiActVar)
{
PvList_Typ        *pVariablenVerwaltungStart, *pVariablenManagement;
Client_List_Typ   *pClientVerwaltungStart, *pClientManagement;
UDINT uiShiftLen = 0;
BOOL bHostName = FALSE;
BOOL bSearchHostName = FALSE;
BOOL bSearchModus = FALSE;
BOOL bSearchArgs = TRUE;
USINT arTempStr[6];
USINT *pParsDataHost;
USINT *pParsDataModus;
UDINT uiOffsetPara = 0;
BOOL bGetIp = FALSE;
UINT iFoundModus = MODE_NEW; // gefundener Modus
  
  AktServer = 0;
  *puiActVar = 0;
  uiCntOffset  = 0;
  StartParsen = FALSE;
  arTempStr[0] = 0;
  
  pParsDataHost = NULL;
  pParsDataModus = NULL;
  
  UNUSED(pParsDataHost);    // for the compiler
  UNUSED(pParsDataModus);   // for the compiler

  pVariablenVerwaltungStart = (PvList_Typ*) pStartAdrPv;
  pClientVerwaltungStart = (Client_List_Typ*) pStartAdrServ;
  pStartParsData = (USINT*) pStartAdrDM;
  pClientManagement = pClientVerwaltungStart + AktServer;

/*****************************************************************************************************************/
/*  Beginn der Auswertung des Datenmoduls, sofern Bedingungen erfüllt*/
  
  for (udCntDM = 0; udCntDM < udLenDM; udCntDM++)
  {
    uiCntOffset++;
    pParsData = pStartParsData + udCntDM;

    /* --- neue UST gefunden ---------------------------------------------- */
    if (*pParsData == PARSER_KEY)
    {
      pParsData++;  /* auf nächstes Feld stellen für Suche nach UST */
      ResultStrCmp = brsstrcmp((UDINT) pParsData, (UDINT) PARSER_KEY_UST);
      
      if (ResultStrCmp == 0)
      {
        uiCntOffset = uiCntOffset + brsstrlen((UDINT) PARSER_KEY_UST) + 1;
        pParsData = pStartParsData + uiCntOffset;
        pParsDataHost = pParsData;
        pParsDataModus = pParsData;
        bGetIp = FALSE;
        bSearchArgs = TRUE;

        /* Suchen nach Schlüsselwörtern */
        do
        {
          uiShiftLen = 0;
        
          /* --- Unterscheidung -> ist IP-Adr. konfiguriert oder Hostname vorhanden "mytest" */
          bSearchHostName = SearchForName(&pParsData, (USINT*) PARSER_KEY_HOST);
          /* --- Modus  */ 
          bSearchModus = SearchForName(&pParsData, (USINT*) PARSER_KEY_MODE);

          if ((bSearchHostName == TRUE && bSearchModus == FALSE) && (bGetIp == FALSE))
          {
            pParsDataHost = pParsData;
            brsstrcpy((UDINT) &pClientManagement->HostName, (UDINT) pParsData);
            uiOffsetPara = brsstrlen((UDINT) &pClientManagement->HostName);
            pClientManagement->IPAdresse = 0;
            pClientManagement->ConfByHost = 1;
            SaveUSTData = TRUE;
            bHostName = TRUE;
            pParsData+=1;
            brsmemcpy((UDINT) &Buffer[4], (UDINT) (pParsData + uiOffsetPara), 10);
            /* uiShiftLen+= sizeof(PARSER_KEY_HOST) + 10; */
            uiOffsetPara+= 10; /* Offset für UST Ustparameter (normal 14, aber Ip wurde nicht konfiguriert) */
            pParsData += uiOffsetPara;

            uiCntOffset = 0;
            uiShiftLen = pParsData - pStartParsData;
            
            bGetIp = TRUE;
          }
          /* --- Hostname wurde nicht konfiguriert -> alte Variante */ 
          else if ((bSearchHostName == FALSE) && (bSearchModus == FALSE) && (bGetIp == FALSE))
          {         
            brsmemcpy((UDINT) &Buffer[0], (UDINT) pParsData, 14); /* Umkopieren der Ustdaten */
            bHostName = FALSE;
            SaveUSTData = TRUE;
            for (cnt = 0; cnt <= 14; cnt++)
            {
              if (Buffer[cnt] == PARSER_KEY)
              {
                ResultStrCmp = brsstrcmp((UDINT) &Buffer[cnt + 1], (UDINT) PARSER_KEY_UST);
                if (ResultStrCmp == 0)
                {
                  SaveUSTData = FALSE;
                  break;
                }
              }
            }
            uiShiftLen = 14;
            bGetIp = TRUE;            
          }
          
          if (bSearchModus)
          { 
            pParsDataModus = pParsData;
            brsstrcpy((UDINT) &arTempStr, (UDINT) pParsData);
            uiShiftLen = brsstrlen((UDINT) &arTempStr);
            iFoundModus = (UINT) brsatoi((UDINT) &arTempStr);
            pParsData += 2; /* CR + LF*/
            /* uiShiftLen += 2; */
            uiShiftLen+= sizeof(PARSER_KEY_MODE);
          }
          
          /* --- Suche erfolglos -> Abbruch  */
          if ((bSearchModus == FALSE) && (bSearchHostName == FALSE))
          {
            bSearchArgs = FALSE;
          }
          
          uiCntOffset+= uiShiftLen; /* Zeigeroffset um ausgelesene Struktur versetzen */
        } while (bSearchArgs == TRUE);
                      
        
        /* --- wenn erfolgreich, dann Übertragen der Daten in die Client - Struktur */
        if (SaveUSTData == TRUE) 
        {
        /* auf Client x schreiben */
#if INTEL         
          if (bHostName == FALSE)
          {
            /* in Feldern 0-3 wird IP Adresse erwartet */
            brsmemcpy((UDINT) &pClientManagement->IPAdresse, (UDINT) &Buffer[0], 4);
            pClientManagement->ConfByHost = 0;
            pClientManagement->HostName[0] = 0;
          }
          
          pClientManagement->Timeout = Buffer[5];
          pClientManagement->Timeout = (pClientManagement->Timeout << 8) + Buffer[4];  /* Feld 4 + Feld 5   */

          pClientManagement->LifeCheck = STATE_LIFE_ERR;  /* Status nicht angemeldet !*/
          pClientManagement->Modus = iFoundModus;
          /* Abspeichern der Werte je UST -> falls 0 konfiguriert wurde */
          ReadUST.Hysterese =   Buffer[7];
          ReadUST.Hysterese = (ReadUST.Hysterese << 8) + Buffer[6];
          
          ReadUST.InitWert  = Buffer[9];
          ReadUST.InitWert  = (ReadUST.InitWert << 8) + Buffer[8];
          
          ReadUST.Remanenz  =   Buffer[11];
          ReadUST.Remanenz  = (ReadUST.Remanenz << 8) + Buffer[10]; 
          
          ReadUST.Ersatzwert  =   Buffer[13];
          ReadUST.Ersatzwert  = (ReadUST.Ersatzwert << 8) + Buffer[12];
          
#else
          if (bHostName == FALSE)
          {
            /* in Feldern 0-3 wird IP Adresse erwartet */
            brsmemcpy(&pClientManagement->IPAdresse, &Buffer[0], 4);
            pClientManagement->ConfByHost = 0;
            pClientManagement->HostName[0] = 0;
          }
          
          pClientManagement->Timeout = Buffer[4];
          pClientManagement->Timeout = (pClientManagement->Timeout << 8) + Buffer[5];  /* Feld 4 + Feld 5   */

          pClientManagement->LifeCheck = STATE_LIFE_ERR;  /* Status nicht angemeldet !*/
          pClientManagement->Modus = iFoundModus;
          /* Abspeichern der Werte je UST -> falls 0 konfiguriert wurde */
          ReadUST.Hysterese =   Buffer[6];
          ReadUST.Hysterese = (ReadUST.Hysterese << 8) + Buffer[7];
          
          ReadUST.InitWert  = Buffer[8];
          ReadUST.InitWert  = (ReadUST.InitWert << 8) + Buffer[9];
          
          ReadUST.Remanenz  =   Buffer[10];
          ReadUST.Remanenz  = (ReadUST.Remanenz << 8) + Buffer[11]; 
          
          ReadUST.Ersatzwert  =   Buffer[12];
          ReadUST.Ersatzwert  = (ReadUST.Ersatzwert << 8) + Buffer[13];

#endif          
          /* zum nächsten Client weiter */
          AktServer++;
          if (AktServer <= MAX_SERVER )
          {
            pClientManagement = pClientVerwaltungStart + AktServer;
            iFoundModus = MODE_NEW;
          }
          else
            break;
        } /* if (SaveUSTData == TRUE) */
        
        /* Aufbau: 2 Strings + 4 Word */
        VarStringCnt = 0;
        VariablenOffset = 0;
        PositionVariable = uiCntOffset;
        
        for (ZaehlerVariablen = uiCntOffset; ZaehlerVariablen < udLenDM; ZaehlerVariablen++)
        {
          pParsData = pStartParsData + ZaehlerVariablen;

          /* Stringlänge ermitteln*/
          GroeszeString = brsstrlen((UDINT) pParsData);
          /* der Schleifenzähler inkrementiert immer um 1 -> bei Strings muß nicht erhöht werden, da der Nullstring übergangen wird */
          /* erwartet wird: */
          /* String Server  */
          /* String Client  */
          /* 4 x 2 Byte für Hysterese, Initwert, Remanenz, Ersatzwert */
          /* Erweiterung: 2 Bytes für Zeitsynchronisierung */
          
          if ((GroeszeString > 0) && (VarStringCnt == 0))
          { 
            ZaehlerVariablen = ZaehlerVariablen + GroeszeString;
            brsmemset((UDINT) &ReadVar.NameServer, 0, sizeof(ReadVar.NameServer));
            brsmemcpy((UDINT) &ReadVar.NameServer[0], (UDINT) pParsData, GroeszeString);
            VarStringCnt++;
            VariablenOffset = VariablenOffset + GroeszeString + 1;
            DummyVar = 0;
          }
          else if((GroeszeString > 0) && (VarStringCnt == 1))
          {
            ZaehlerVariablen = ZaehlerVariablen + GroeszeString;
            brsmemset((UDINT) &ReadVar.NameClient, 0, sizeof(ReadVar.NameClient));
            brsmemcpy((UDINT) &ReadVar.NameClient[0], (UDINT) pParsData, GroeszeString);
            VarStringCnt++;
            VariablenOffset = VariablenOffset + GroeszeString + 1;
          }
          else if (VarStringCnt == 2)
          {
            ZaehlerVariablen = ZaehlerVariablen + 1;
            brsmemcpy((UDINT) &ReadVar.Hysterese, (UDINT) pParsData, 2);
            VarStringCnt++;
          }
          else if (VarStringCnt == 3)
          {
            ZaehlerVariablen = ZaehlerVariablen + 1;
            brsmemcpy((UDINT) &ReadVar.InitWert, (UDINT) pParsData, 2);
            VarStringCnt++;
          }
          else if (VarStringCnt == 4)
          {
            ZaehlerVariablen = ZaehlerVariablen + 1;
            brsmemcpy((UDINT) &ReadVar.Remanenz, (UDINT) pParsData, 2);
            VarStringCnt++;
          }
          else if (VarStringCnt == 5)
          {
            ZaehlerVariablen = ZaehlerVariablen + 1;
            brsmemcpy((UDINT) &ReadVar.Ersatzwert, (UDINT) pParsData, 2);
            VarStringCnt++;
          }
          else if (VarStringCnt == 6)
          {
            ZaehlerVariablen = ZaehlerVariablen + 1;
            brsmemcpy((UDINT) &ReadVar.SyncTime, (UDINT) pParsData, 2);
            VarStringCnt++;
          }

          /* ermittelte Werte werden in Speicher abgelegt*/
          if (VarStringCnt == 7)
          {
            VariablenOffset = VariablenOffset + 8;
            
            /* nach # suchen, um Schleife verlassen zu können und mit neuer Ust starten*/
            for (x = PositionVariable; x < (uiCntOffset + 2); x++)
            {
              pParsData = pStartParsData + x;
              if (*pParsData == PARSER_KEY)
              {
                pParsData++;
                ResultStrCmp = brsstrcmp((UDINT) pParsData, (UDINT) PARSER_KEY_UST);
                pParsData--;
                if (ResultStrCmp == 0)
                {
                  FlagAbbruch = TRUE;
                  break; /* Abbruch -> keine Auswertung! */
                } /* if (ResultStrCmp == 0) */
              }/* if (*pParsData == PARSER_KEY) */
            } /* for (x = (uiCntOffset - VariablenOffset); x < uiCntOffset; x++) */
            
            if (FlagAbbruch == TRUE)
            {
              break;
            }
                 
            VarStringCnt = 0;
            pVariablenManagement = pVariablenVerwaltungStart + *puiActVar;
            (*puiActVar)++;
            
            /* --- die ermittelten Werte werden auf den allokierten Speicher umkopiert*/
            brsmemcpy((UDINT) &pVariablenManagement->NameClient[0], (UDINT) &ReadVar.NameClient[0], PV_LENGTH_ARR);
            brsmemcpy((UDINT) &pVariablenManagement->NameServer[0], (UDINT) &ReadVar.NameServer[0], PV_LENGTH_ARR);
            pVariablenManagement->Hysterese   = ReadVar.Hysterese;
            pVariablenManagement->Init      = ReadVar.InitWert;
            pVariablenManagement->Remanent  = ReadVar.Remanenz;
            pVariablenManagement->Ersatzwert  = ReadVar.Ersatzwert;
            pVariablenManagement->SyncTime  = ReadVar.SyncTime;
            
            pVariablenManagement->Client = AktServer - 1;
            brsmemset((UDINT) &ReadVar, 0, sizeof(ReadVar));
            
            /* ---------- kann die nächste PV noch gelesen werden? ------------------------ */
            if ((*puiActVar) + 1 >= MAX_VARIABLEN)
            {
              /* nein -> Grenzwertüberschreitung -> Ende !*/
              return TRUE;  
            }
            
          } /* if (VarStringCnt == 6) */
          
          
          uiCntOffset = ZaehlerVariablen;
        } /*for (ZaehlerVariablen = uiCntOffset; ZaehlerVariablen < udLenDM; ZaehlerVariablen++)*/

        uiCntOffset = pParsData - pStartParsData;

        /* UST - Ende erreicht:                               */
        /* da Zähler inkrementiert wird bei neuem Durchgang um 1 dekrementiert-> Abbgleich! */
        if (FlagAbbruch == TRUE)
        {
          FlagAbbruch = FALSE;
          udCntDM = uiCntOffset - 1; 
        } /* if (FlagAbbruch == TRUE) */
      }/* if (ResultStrCmp == TRUE) */
    } /* if (*pParsData == PARSER_KEY) */   
  } /* for (udCntDM = 0; udCntDM < udLenDM; udCntDM++)*/

  return FALSE;
}

