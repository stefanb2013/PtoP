/*****************************************************************************************************************/
/*                    C - File für Stephandler Funktion(en)                                   ´                  */ 
/*---------------------------------------------------------------------------------------------------------------*/
/*            Taskname: client bzw. server                                                                       */
/*            Dateiname: stephandler.c                                                                           */
/*            Autor:  B&R                                                                                        */
/*            Erstelldatum: November 2020                                                                        */
/*            Classtime: 10 - 30000 ms                                                                           */
/*---------------------------------------------------------------------------------------------------------------*/
/*  Funktion:                                                                                                    */
/*                                                                                                               */
/*****************************************************************************************************************/

#ifdef _DEFAULT_INCLUDES
#include <AsDefault.h>
#endif

#include "../stephandler.h"

/* Behandelt Schrittketten (Protokollierung, Timeout usw.) */
signed long BrbStepHandler(PBrbStepHandling_type* pStepHandling)
{
	// Muss der Schritt protokolliert werden?
	if(pStepHandling->Intern.bLogOnNextCycle == 1)
	{
		pStepHandling->Intern.bLogOnNextCycle = 0;
		// Ist Protokollierung gestoppt?
		if(pStepHandling->Log.bStop == 0)
		{
			// Zykluszähler merken
			pStepHandling->Log.Steps[0].nCycleCount = pStepHandling->Intern.nCycleCount;
			// Einträge um eins nach unten schieben
			brsmemmove((UDINT) &pStepHandling->Log.Steps[1], (UDINT) &pStepHandling->Log.Steps[0], sizeof(PBrbStepHandlingStep_type)*(nBRB_STEPLOG_STEPS_MAX_CPY));
			// Aktuellen Eintrag reinkopieren
			pStepHandling->Log.Steps[0].nStepNr = pStepHandling->Intern.nStepNrOld;
			brsstrcpy((UDINT) &pStepHandling->Log.Steps[0].sStepText, (UDINT) &pStepHandling->Current.sStepText);			
		}
		// Zykluszähler wieder auf 0 setzen
		pStepHandling->Intern.nCycleCount = 0;
	}
	// Hat sich der Schritt geändert?
	if(pStepHandling->Current.nStepNr != pStepHandling->Intern.nStepNrOld)
	{
		// Schritt beim nächsten Durchlauf protokollieren, weil dann erst der richtige Schritttext gesetzt ist
		pStepHandling->Intern.bLogOnNextCycle = 1;
	}
	// Zykluszähler erhöhen
	pStepHandling->Intern.nCycleCount++;
	// Aktuellen Schritt übernehmen
	pStepHandling->Intern.nStepNrOld = pStepHandling->Current.nStepNr;
	// Kommando zum Löschen der Protokollierung
	if(pStepHandling->Log.bClear == 1)
	{
		pStepHandling->Log.bClear = 0;
		// Den gesamten Aufzeichnungspuffer löschen
		brsmemset((UDINT) &pStepHandling->Log.Steps[0], 0, sizeof(pStepHandling->Log.Steps));
		pStepHandling->Intern.bLogOnNextCycle = 1;
	}
	// StepTimeout
	TON(&pStepHandling->Intern.fbTimeout);
	if(pStepHandling->Intern.fbTimeout.Q == 1)
	{
		// Timeout abgelaufen
		pStepHandling->Intern.fbTimeout.IN = 0;
		pStepHandling->Current.bTimeoutElapsed = 1;
	}	
	return 0;
}
