(********************************************************************
 * COPYRIGHT -- Bernecker + Rainer
 ********************************************************************
 * Program: getClFile
 * File: getClFile.st
 * Author: B&R
 * Created: March 03, 2014
 ********************************************************************
 * Implementation of program getClFile
 ********************************************************************)

PROGRAM _INIT


  PtopServGetClients_0.szDeviceName := 'FileDevice';
  bCmdBuild := FALSE;
  
END_PROGRAM


PROGRAM _CYCLIC


(* TODO : Add your code here *)
  
  IF bCmdBuild = TRUE THEN
    PtopServGetClients_0.enable := 1;
    PtopServGetClients_0.BuildFile := bCmdBuild;
    
    PtopServGetClients_0();
  
    IF PtopServGetClients_0.status <> ERR_FUB_BUSY THEN
      bCmdBuild := FALSE;
    END_IF
  END_IF
  
      
      
END_PROGRAM
