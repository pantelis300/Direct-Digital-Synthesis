#ifndef HARMONIC_SIGN_GEN_H_INCLUDED
#define HARMONIC_SIGN_GEN_H_INCLUDED


typedef char bool;
typedef unsigned short      UINT16;
typedef signed short	    INT16;
typedef unsigned long       UINT32;
typedef long                INT32;

/*
    This function updates the state of the module given the binary inputs of the
    system in1, in2, in3, in4 and reset. The binary inputs in1, in2, in3, in4
    determine the output frequency and

    INPUTS:
    UINT16 time         - Current system time in ms.
    in1, in2, in3, in4  - Binary inputs for frequency calculation
    reset               - Module Reset. If in IDLE state and reset = 0, the output of the
                          module ramps up to the maximum amplitude.
                          If in STEADY state and reset = 1, the output of the module
                          ramps down to the minimum amplitude.
*/
void FGEN_InputChange(UINT16 time, bool in1, bool in2, bool in3, bool in4, bool reset);

/*
    This function returns the current output value of the module.

    INPUT:
        UINT16 time     - Current system time in ms.
    OUTPUT:
        UINT16          - Current system output.
*/
INT16 FGEN_GetOutput(UINT16 time);

/*
    This function performs initialization of the system.
    INPUT:
        UINT16 time     - Current system time in ms.
*/
void  FGEN_Init(UINT16 time);

extern UINT16 amplitude;
extern UINT16 module_frequency ;

#endif // HARMONIC_SIGN_GEN_H_INCLUDED
