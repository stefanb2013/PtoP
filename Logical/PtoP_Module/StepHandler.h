/*****************************************************************************************************************/
/*										H - File für ptop					´							     	 */	
/*---------------------------------------------------------------------------------------------------------------*/
/*						Taskname: server + client																 */
/*						Dateiname: stephandler.h																	 */
/*						Autor:	B&R																				 */
/*						Erstelldatum: November 2020																 */
/*						Classtime: 10 - 30000 ms																 */
/*---------------------------------------------------------------------------------------------------------------*/
/*	Funktion:																									 */
/*					- step handler from brblib													     		 	 */
/*					- protocolls the step navigation										 					 */
/*																												 */
/*****************************************************************************************************************/

#ifndef STEPHANDLER_H_
#define STEPHANDLER_H_

#ifdef _DEFAULT_INCLUDES
#include <AsDefault.h>
#endif

/* Prototyping*/
signed long BrbStepHandler(PBrbStepHandling_type* pStepHandling);

#endif