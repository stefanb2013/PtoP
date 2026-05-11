/*****************************************************************************************************************/
/*                    C - File für UDP Server       ´                    */ 
/*---------------------------------------------------------------------------------------------------------------*/
/*            Taskname: server                                     */
/*            Dateiname: pipe.c                                    */
/*            Autor:  B&R                                        */
/*            Erstelldatum: Juli 2003                                  */
/*            Classtime: 10 - 30000 ms                                 */
/*---------------------------------------------------------------------------------------------------------------*/
/*  Funktion:                                                  */
/*          - Funktion zum Allokieren des Pipe - Speichers                           */
/*          - Funktionen zum Lesen u. Speichern in die Pipe                        */
/*                                                         */
/*****************************************************************************************************************/

#include <bur/plc.h>
#include <bur/plctypes.h>
#include <sys_lib.h>
#include "pipe.h"
#include "allgserv.h"

plcbit  EnableBit, WriteBit, ReadBit;
UDINT   Dummy;
UINT  AufrufCnt;
UINT  Sucess;

/************************************* allocate Memory for the storage Pipe **********************************************/
/* enters a set of Values into the pipe                                          */
UDINT SetPipeMemory ( /* Rückgabewert: Startadresse der Pipe*/
UINT  PipeSize,     /* Größe der zu allokierenden Pipe */
UINT* pAllocState)    /* Resultat der Allokierung */
{
UINT  StatusTmpAlloc = 0; /* Returnvalue */
UDINT   *pTmpStart;     /* Start - Adress of allocated memory */
UDINT pRetVal = NULL;   /* Return Value of function */

  if (pAllocState == NULL)
  {
    pRetVal = NULL;
    return pRetVal;
  }

  StatusTmpAlloc = TMP_alloc(PipeSize,(void**)&pTmpStart);
  *pAllocState = StatusTmpAlloc;
  
  if (StatusTmpAlloc == 0)
  {
    brsmemset((UDINT) pTmpStart, 0, PipeSize);
    pRetVal = (UDINT) pTmpStart;
  }
  else
  {
    pRetVal = NULL;
  }
  
  return pRetVal;
}


/************************************* write Data to memory **************************************************************/

UINT  WriteDataToPipe (
PipeManagement_typ Uebergabe,     /* zu übergebende Werte */
UDINT pPipeStartAdress,       /* Start der Pipe   */
UDINT pPipeStopAdress,        /* Ende der Pipe    */
PipeManagement_typ** ppWritePipe,   /* Schreibzeiger    */
PipeManagement_typ* pReadPipe)    /* Lesezeiger       */
{
  
  if ((pPipeStartAdress == NULL) ||
    (pPipeStopAdress == NULL) ||
    (*ppWritePipe == NULL) ||
    (pReadPipe == NULL))
  {
    return 1;
  } 

  if ((*ppWritePipe) >= (PipeManagement_typ*)pPipeStopAdress)
  {
    if ((PipeManagement_typ*)pPipeStartAdress != pReadPipe)
    {
      brsmemcpy((UDINT) *ppWritePipe, (UDINT) &Uebergabe, sizeof(PipeManagement_typ));
      (*ppWritePipe)->DataAvailable = TRUE;
      *ppWritePipe = (PipeManagement_typ*)pPipeStartAdress;
      return 0;   
    }
    else
    {
      return 1;
    }
  }
  else
  {
    if (((*ppWritePipe) + 1) != pReadPipe)
    {
      brsmemcpy((UDINT) *ppWritePipe, (UDINT) &Uebergabe, sizeof(PipeManagement_typ));
      (*ppWritePipe)->DataAvailable = TRUE;
      (*ppWritePipe)++;
      return 0;
    }
    else
    {
      return 1;
    }
  }
  
}


/************************************* read Data to from pipe **********************************************/

UINT ReadDataFromPipe(
UDINT pPipeStopAdress,            /* Ende der Pipe    */
UDINT pPipeStartAdress,           /* Start der Pipe   */
PipeManagement_typ **ppReadPipe,      /* Lesezeiger       */
PipeManagement_typ *pWritePipe,       /* Schreibzeiger    */
PipeManagement_typ *pReturnTyp)       /* Rückgabewerte    */
{
  
  if ((pPipeStopAdress != NULL) && (pPipeStartAdress != NULL) && ((*ppReadPipe) != NULL) && (pWritePipe != NULL) && (pReturnTyp != 0))
  {
    /* --- read pointer ist not equal write pointer*/
    if (*ppReadPipe != pWritePipe) 
    {
      brsmemcpy((UDINT) pReturnTyp, (UDINT) *ppReadPipe, sizeof(PipeManagement_typ));
  
      (*ppReadPipe)->DataAvailable = FALSE;
      (*ppReadPipe)++;
      
      if (*ppReadPipe > (PipeManagement_typ*) pPipeStopAdress) 
      {
        *ppReadPipe = (PipeManagement_typ*) pPipeStartAdress;
      }   
      return 0;
    }
    else
    {
      return 1;
    }
  } /* if ((PipeStopAdress != 0) && (PipeStartAdress != 0) && ((*ppReadPipe) != NULL) && (pWritePipe != NULL) && (pReturnTyp != 0)) */
  else
  {
    return 2;
  }
}
