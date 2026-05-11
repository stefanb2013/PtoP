
#include <bur/plctypes.h>

#ifdef _DEFAULT_INCLUDES
	#include <AsDefault.h>
#endif


/*****************************************************************************************************************/
/*                      Makros                                                                                   */
/*****************************************************************************************************************/

#define ABS(N) ((N<0)?(-N):(N))

UDINT GetAbsoluteValue(UDINT NewValue, UDINT OldValue);

void _INIT ProgramInit(void)
{

}

void _CYCLIC ProgramCyclic(void)
{
  ResultValue = ABS(FirstValue-LastValue);
  ResultValue1 = GetAbsoluteValue(FirstValue, LastValue);
}

void _EXIT ProgramExit(void)
{

}

UDINT GetAbsoluteValue(UDINT NewValue, UDINT OldValue)
{
  UDINT ReturnValue = 0;
  
  if (NewValue == OldValue)
  {
    ReturnValue = 0;
  }
  else if (NewValue > OldValue)
  {
    ReturnValue = NewValue - OldValue;
  }
  else if (OldValue > NewValue)
  {
    ReturnValue = OldValue - NewValue;
  }
  
  return ReturnValue;
}
