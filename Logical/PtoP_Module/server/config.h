/*****************************************************************************************************************/
/*										Header File Config-File					´							     */	
/*---------------------------------------------------------------------------------------------------------------*/
/*						Taskname: server + client																 */
/*						Dateiname: config.h																		 */
/*						Autor:	B&R																				 */
/*						Erstelldatum: März 2007																	 */
/*						Classtime: 10 - 30000 ms																 */
/*---------------------------------------------------------------------------------------------------------------*/
/*	Funktion:																									 */
/*					- Auslesen von Konfigurationsinformationen aus Datenmodul "ptopconf"			     		 */
/*																												 */
/*****************************************************************************************************************/

#ifndef CONFIG_H
#define CONFIG_H

BOOL GetConfData(void* pPtopConf, UINT uiLen, UINT uiId);

#endif
