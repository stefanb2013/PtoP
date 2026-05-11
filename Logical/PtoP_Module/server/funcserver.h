/*****************************************************************************************************************/
/*										Header File für Allgemeine Funktionen								     */
/*---------------------------------------------------------------------------------------------------------------*/
/*						Taskname: server																		 */
/*						Dateiname: funcserver.h																	 */
/*						Autor:	B&R																				 */
/*						Erstelldatum: Juni 2007																 	 */
/*						Classtime: 10 - 30000 ms																 */
/*---------------------------------------------------------------------------------------------------------------*/
/*	Funktion:																									 */
/*																												 */
/*****************************************************************************************************************/

#ifndef FUNCSERVER_H
#define FUNCSERVER_H

#include "..\\processor.h"
#include <bur\plc.h>
#include <bur\plctypes.h>
#include "ptpserv.h"
#include "allgserv.h"

void ConvertIpAdrListToAscii(UDINT* pClientListStartAdr);
UINT ConvertIpAdrToAscii(UDINT* pClientAdr);
void ConvertIpAdrToAsciiX(UDINT IpAdr, UDINT* parIpAdr);
void ConvertAsciiToIpAdr(UDINT* pIpAdrSourceTest, UDINT* parIpAdrTest);

#endif
