/*****************************************************************************************************************/
/*										Header File für UDP	Server				´							     */	
/*---------------------------------------------------------------------------------------------------------------*/
/*						Taskname: server																		 */
/*						Dateiname: pipe.h																		 */
/*						Autor:	B&R																				 */
/*						Erstelldatum: Juli 2003																	 */
/*						Classtime: 10 - 30000 ms																 */
/*---------------------------------------------------------------------------------------------------------------*/
/*	Funktion:																									 */
/*					- Prototyping																	     		 */
/*					- Datenverwaltung														 					 */
/*					- Konstanten																				 */
/*																												 */
/*****************************************************************************************************************/
#ifndef PIPE_H_
#define PIPE_H_

#ifdef _DEFAULT_INCLUDES
#include <AsDefault.h>
#endif

#define PIPE_SIZE		200			/* Daten je Pipe	*/
#define PIPE_CNT		10			/* Anzahl der Pipes */


UDINT SetPipeMemory (UINT PipeSize, UINT* pAllocState);
UINT  WriteDataToPipe (PipeManagement_typ Uebergabe, UDINT pPipeStartAdress, UDINT pPipeStopAdress, PipeManagement_typ** ppWritePipe, PipeManagement_typ* pReadPipe);
UINT  ReadDataFromPipe (UDINT pPipeStopAdress, UDINT pPipeStartAdress, PipeManagement_typ **ppReadPipe, PipeManagement_typ *pWritePipe, PipeManagement_typ *pReturnTyp);

#endif
