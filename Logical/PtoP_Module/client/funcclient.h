/*****************************************************************************************************************/
/*										Header File für Allgemeine Funktionen								     */
/*---------------------------------------------------------------------------------------------------------------*/
/*						Taskname: functions																		 */
/*						Dateiname: functions.h																	 */
/*						Autor:	B&R																				 */
/*						Erstelldatum: Juni 2007																 	 */
/*						Classtime: 10 - 30000 ms																 */
/*---------------------------------------------------------------------------------------------------------------*/
/*	Funktion:																									 */
/*					- Konstanten für Fehlerauswertung												     		 */
/*					- Fehlerstruktur														 					 */
/*																												 */
/*****************************************************************************************************************/

#ifndef FUNCCLIENT_H
#define FUNCCLIENT_H

#include "..\\processor.h"
#include <bur\plc.h>
#include <bur\plctypes.h>
//#include <ethsock.h>
#include "ptpcl.h"
#include "allgclnt.h"

void ConvertIpAdrListToAscii(UDINT* pClientListStartAdr);
UINT ConvertIpAdrToAscii(UDINT* pClientAdr);
void ConvertIpAdrToAsciiX(UDINT IpAdr, UDINT* parIpAdr);

#endif
