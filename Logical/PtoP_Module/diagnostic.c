/*****************************************************************************************************************/
/*                    C - File für ptop                              */
/*---------------------------------------------------------------------------------------------------------------*/
/*            Taskname: server                                     */
/*            Dateiname: diagnostic.c                                  */
/*            Autor:  B&R                                        */
/*            Erstelldatum: April 2005                                 */
/*            Classtime: 10 - 30000 ms                                 */
/*---------------------------------------------------------------------------------------------------------------*/
/*  Funktion:                                                  */
/*          - Mitloggen von Telegrammdaten + Filter                              */
/*          - Schrittverfolgung                                      */
/*                                                         */
/*****************************************************************************************************************/

#ifdef _DEFAULT_INCLUDES
#include <AsDefault.h>
#endif


#include <astime.h>
#include <bur/plc.h>
#include <bur/plctypes.h>
#include <sys_lib.h>
#include "../diagnostic.h"
#include "../client/allgclnt.h"
#include "../client/ptpcl.h"



DINT CyclicBurLoggerCreate(Logger_typ* pLogger, char* pLoggerName)
{
  DINT iStatus;
  
  iStatus = ERR_FUB_BUSY;
  pLogger->Diag.CreateLogger++;
  
  switch(pLogger->Steps.Create)
  {
    case stLogger_Create_Idle:
      {
        brsstrcpy((UDINT) &pLogger->Fubs.ArEventLogGetIdent_Fub.Name, (UDINT) pLoggerName);
        pLogger->Fubs.ArEventLogGetIdent_Fub.Execute = TRUE;
        
        pLogger->Steps.Create = stLogger_Create_Ident;
        break;
      }
    
    case stLogger_Create_Ident:
      {      
        ArEventLogGetIdent(&pLogger->Fubs.ArEventLogGetIdent_Fub);
        
        if (pLogger->Fubs.ArEventLogGetIdent_Fub.Done == TRUE)
        {
          pLogger->Ident = pLogger->Fubs.ArEventLogGetIdent_Fub.Ident;
          
          pLogger->Fubs.ArEventLogGetIdent_Fub.Execute = FALSE;
          ArEventLogGetIdent(&pLogger->Fubs.ArEventLogGetIdent_Fub);
          
          // Logger exists allready -> done!
          pLogger->Steps.Create = stLogger_Create_Finished;
        }
        else if (pLogger->Fubs.ArEventLogGetIdent_Fub.Error == TRUE)
        {
          pLogger->Fubs.ArEventLogGetIdent_Fub.Execute = FALSE;
          ArEventLogGetIdent(&pLogger->Fubs.ArEventLogGetIdent_Fub);
          
          // Logger does not exist allready -> create it!
          brsstrcpy((UDINT) &pLogger->Fubs.ArEventLogCreate_Fub.Name, (UDINT) pLoggerName);
          pLogger->Fubs.ArEventLogCreate_Fub.Persistence = arEVENTLOG_PERSISTENCE_PERSIST;
          pLogger->Fubs.ArEventLogCreate_Fub.Size = 100000; //[Byte]
          pLogger->Fubs.ArEventLogCreate_Fub.Info = 0;
          pLogger->Fubs.ArEventLogCreate_Fub.Execute = TRUE;
          
          pLogger->Steps.Create = stLogger_Create_Create;
        
        }
        break;
      }
    
    case stLogger_Create_Create:
      {
        ArEventLogCreate(&pLogger->Fubs.ArEventLogCreate_Fub);
        if (pLogger->Fubs.ArEventLogCreate_Fub.Busy == FALSE)
        {
          if (pLogger->Fubs.ArEventLogCreate_Fub.Done == TRUE)
          {
            
            pLogger->Ident = pLogger->Fubs.ArEventLogCreate_Fub.Ident;
            
            pLogger->Fubs.ArEventLogCreate_Fub.Execute = FALSE;
            ArEventLogCreate(&pLogger->Fubs.ArEventLogCreate_Fub);
            
            pLogger->Steps.Create = stLogger_Create_Finished;
            
          }
          else if (pLogger->Fubs.ArEventLogCreate_Fub.Error == TRUE)
          {
            pLogger->Fubs.ArEventLogCreate_Fub.Execute = FALSE;
            ArEventLogCreate(&pLogger->Fubs.ArEventLogCreate_Fub);
            
            pLogger->Ident = 0;    
            pLogger->LastError = pLogger->Fubs.ArEventLogCreate_Fub.StatusID;
            pLogger->Steps.Create = stLogger_Create_Err;
          }
        }
        
        break;
      }
    
    case stLogger_Create_Finished:
      {
        pLogger->IsActive = TRUE;
        pLogger->Steps.Create = stLogger_Create_Idle;
        iStatus = ERR_OK;
        break;
      }
    
    case stLogger_Create_Err:
      {
        pLogger->IsActive = FALSE;
        pLogger->Steps.Create = stLogger_Create_Idle;
        iStatus = pLogger->LastError;
        break;
      }
  }
  
  return iStatus;
} 


DINT WriteLoggerMessage(Logger_typ* pLogger, UINT ErrNumber, STRING* pMessage, DINT AdditionalStatus, USINT Severity)
{
  DINT iRetVal;
  UDINT iStrLen;
  USINT HelpString1[255];
  USINT HelpString2[20];
  
  iStrLen = 0;
  HelpString1[0] = 0;
  if ((pLogger->IsActive == FALSE) || (pLogger->Ident == 0))
  {
    return 0;
  }
  
  if (AdditionalStatus != 0x7FFFFFFF)
  {
    brsstrcpy((UDINT) &HelpString1, (UDINT) pMessage);
    brsitoa(AdditionalStatus, (UDINT) &HelpString2);    
    brsstrcat((UDINT) &HelpString1, (UDINT) " (");
    brsstrcat((UDINT) &HelpString1, (UDINT) &HelpString2);
    brsstrcat((UDINT) &HelpString1, (UDINT) ")");
    
    pLogger->Fubs.ArEventLogWrite_Fub.AddData = (UDINT) &HelpString1;
    iStrLen = brsstrlen((UDINT) &HelpString1);
  }
  else
  {
    pLogger->Fubs.ArEventLogWrite_Fub.AddData = (UDINT) pMessage;
    iStrLen = brsstrlen((UDINT) pMessage);
    
  }
  
  pLogger->Fubs.ArEventLogWrite_Fub.AddDataSize = iStrLen + 1;
  pLogger->Fubs.ArEventLogWrite_Fub.Execute = TRUE;
  pLogger->Fubs.ArEventLogWrite_Fub.Ident = pLogger->Ident;
  pLogger->Fubs.ArEventLogWrite_Fub.TimeStamp = 0;
  pLogger->Fubs.ArEventLogWrite_Fub.AddDataFormat = arEVENTLOG_ADDFORMAT_TEXT;  
  pLogger->Fubs.ArEventLogWrite_Fub.EventID = ArEventLogMakeEventID(Severity, 1, ErrNumber);
  
  // set message
  ArEventLogWrite(&pLogger->Fubs.ArEventLogWrite_Fub);
  iRetVal = pLogger->Fubs.ArEventLogWrite_Fub.StatusID;
  
  // and reset FUB
  pLogger->Fubs.ArEventLogWrite_Fub.Execute = FALSE;
  ArEventLogWrite(&pLogger->Fubs.ArEventLogWrite_Fub);
  
  // debug
  pLogger->Diag.CreateEntry++;
  
  return iRetVal;
}

