#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include "harmonic_sign_gen.h"

#define FALSE                   0
#define TRUE                    1
#define MAX_OUTPUT              20000U

#define INIT_PHASE              16384
#define TWO_MSB_UINT16 			49152 	// 0xC000
#define QUARTER_CICLE_SHIFTS	14		// 16bits - 2bits
#define ADDRESS_SHIFTS			10
#define LUT_ADDRESS_BITS		15360	// 0x3C00
#define PHASE_BITS				16383	// 16'b0011 1111 1111 1111
#define STEP_SCALED				1024	// 2^16 / 16
#define FREQUENCY_SCALING       66
#define RAMP_STEPS              132000
#define MAX_AMPLITUDE           19
#define AMPLITUDE_RAMP_RATE_INV 6947    // RAMP_STEPS / MAX_AMPLITUDE

typedef enum {IDLE, RAMP_UP, RAMP_DOWN, STEADY} state_t;

const UINT16 lut[] =
{
     100,
     200,
     297,
     392,
     483,
     569,
     650,
     724,
     792,
     851,
     903,
     946,
     980,
    1004,
    1019,
    1024
};

UINT16      amplitude           = 0;
state_t     state               = IDLE;
UINT16      module_frequency    = 2;
UINT32     ramp_timer          = 0;
bool        module_reset        = 1;

// Forward Declaration
UINT16 get_amplitude(UINT32 ramp_timer);


/*
    This function implements the transitions of the state machine.
    The four possible states are: IDLE, RAMP_UP, RAMP_DOWN and STEADY.

    In the IDLE state, the output amplitude is 0. The internal counter is
    set to 0. The module remains in this state indefinitely,
    until the reset signal is deactivated. In this case the module
    transitions to the RAMP_UP stage.

    In the RAMP_UP stage the amplitude increases linearly in time
    2s, until it reaches the maximum amplitude and then transitions to
    the STEADY state.

    In the RAMP_DOWN state the amplitude decreases linearly in 2s,
    until it reaches 0 and then transitions to the IDLE state.

    In the STEADY state the amplitude is set to the constant MAX_AMPLITUDE.
    The module remains in this state indefinitely until the reset signal
    is activated (set to FALSE). In this case, it transitions to the
    RAMP_DOWN phase.
*/
void state_machine(UINT16 time)
{
    switch(state)
    {
        case IDLE:
            amplitude = 0;
            if (module_reset == 0)
            {
                state = RAMP_UP;
                ramp_timer = 0;
            }
            break;

        case RAMP_UP:
            ramp_timer++;
            if (ramp_timer < RAMP_STEPS)
                amplitude = get_amplitude(ramp_timer);
            else
                state = STEADY;
            break;

        case RAMP_DOWN:
            ramp_timer++;
            if (ramp_timer < RAMP_STEPS)
                amplitude = get_amplitude(ramp_timer);
            else
                state = IDLE;
            break;

        case STEADY:
            amplitude = MAX_AMPLITUDE;
            if (module_reset == 1)
            {
                state = RAMP_DOWN;
                ramp_timer = 0;
            }
            break;
    }
}

/*
    This utility function calculates the amplitude on a given time, depending on
    the module state. In the RAMP_UP state, the amplitude linearly increases
    from 0 to MAX_AMPLITUDE. In the RAMP_DOWN state, the amplitude linearly
    decreases from MAX_AMPLITUDE to 0. On a STEADY state the amplitude
    remains MAX_AMPLITUDE.

    INPUT:
        UINT32 ramp_timer       - Internal timer for increasing/decreasing
                                  amplitude linearly in 2s.
    OUTPUT:
        UINT16                  - Current ramp up/down amplitude
*/
UINT16 get_amplitude(UINT32 ramp_timer)
{
    if (state == RAMP_UP)
        return ramp_timer / AMPLITUDE_RAMP_RATE_INV;
    else if(state == RAMP_DOWN)
        return MAX_AMPLITUDE - (ramp_timer / AMPLITUDE_RAMP_RATE_INV);
    else
        return MAX_AMPLITUDE;
}

/*
    This utility function sets the global variable module_frequency
    based on the binary inputs of the module
*/
UINT16 set_frequency(bool in1, bool in2, bool in3, bool in4)
{
	module_frequency = (UINT16)(2 + in1 + 3*in2 + 5*in3 + 7*in4);
	return module_frequency;
}

/*
    This utility function returns the computed phase on any
    given time, based on the frequency and the initial phase.


    INPUT:
        UINT16 time         - Current system time in ms.
        UINT16 frequency    - Current frequency in range [2, 18] Hz
        UINT16 init_phase   - Initial sine phase. 0rad -> 0, 2pi rad -> 2^16 - 1
    OUTPUT:
        UINT16              - Sine phase
*/
UINT16 phase(UINT16 time, UINT16 frequency, UINT16 init_phase)
{
	return (frequency * time * FREQUENCY_SCALING + init_phase);
}

/*
    This utility function extract the 2 MSBs of a given phase,
    which represent the quarter of the sine wave.
    The possible values returned are 0, 1, 2 and 3 corresponding
    to the first, second, third and forth quarter respectively.
*/
bool extract_quarter(UINT16 phase)
{
	UINT16 result;
	result = phase & TWO_MSB_UINT16;
	result >>= QUARTER_CICLE_SHIFTS;
	return (bool)result;
}


/*
    This utility function extracts the bits 13, 12, 11 and 10
    which represent the address for the LUT of size 16 = 2^4.
*/
bool extract_lut_address(UINT16 phase)
{
	UINT16 result;
	result = phase & LUT_ADDRESS_BITS;
	result >>= ADDRESS_SHIFTS;
	return (bool)result;
}

/*
    This function implements the linear interpolation calculation given
    the coordinates of two points a and b, and the x value corresponding
    to the phase of the sine phase mapped into the first quarter.
    The distance between two points in the x-axis is always the same
    and equal to STEP_SCALED. For better accuracy performing integer
    arithmetic the inverse slope is computed and the used in the equation.
    y - y_a = ( (y_b - y_a) / (x_b - x_a) ) * (x - x_a)
*/
UINT16 linear_interpolation_uint(UINT16 x_a, UINT16 y_a, UINT16 y_b, UINT16 x)
{
	UINT32 y, slope_inv;

	slope_inv = STEP_SCALED / (y_b - y_a);
	assert(slope_inv != 0);
	y = (x - x_a) / slope_inv + y_a;
	return (UINT16)y;
}


/*
    This function computes the sine of a given phase in the first
    quarter.

    INPUT:
        UINT16 phase     - Sine phase
    OUTPUT:
        UINT16           - Sine output mapped in the first quarter
*/
UINT16 sine_first_quarter(UINT16 phase)
{
	bool addr;
	UINT16 x_a, y_a, y_b, x, y;
	assert( (phase & TWO_MSB_UINT16) == 0);			// or phase < 16383

	addr = extract_lut_address(phase);

	if (addr == 0)
	{
		y_a = 0;
	}
	else
	{
		y_a = lut[addr - 1];
	}
	x = phase & PHASE_BITS;
	x_a = addr * STEP_SCALED;
	y_b = lut[(UINT16)addr];

	y = linear_interpolation_uint(x_a, y_a, y_b, x);
	return y;
}

/*
    This function returns the sine given a phase in the
    second quarter using the identity sine(pi - theta) = sine(theta)
    The value of pi corresponds to the half maximum value of UINT16.
*/
UINT16 sine_second_quorter(UINT16 phase)
{
    return sine_first_quarter(PHASE_BITS - phase);
}

/*
    This function returns the sine given a phase on any of
    the four quarters.

    INPUT:
        UINT16 phase     - Sine phase
    OUTPUT:
        INT16               Sine output
*/
INT16 sine_out(UINT16 phase)
{
	bool quarter = extract_quarter(phase);
	phase &= PHASE_BITS;

	switch(quarter)
	{
		case 0:
            return sine_first_quarter(phase);
        case 1:
            return sine_second_quorter(phase);
        case 2:
            return -sine_first_quarter(phase);
        case 3:
            return -sine_second_quorter(phase);
	}
	return 0;
}


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
void FGEN_InputChange(UINT16 time, bool in1, bool in2, bool in3, bool in4, bool reset)
{
    module_reset = reset;
    state_machine(time);
    set_frequency(in1, in2, in3, in4);
}

/*
    This function returns the current output value of the module.

    INPUT:
        UINT16 time     - Current system time in ms.
    OUTPUT:
        UINT16          - Current system output.
*/
INT16 FGEN_GetOutput(UINT16 time)
{
    // Update the state
    state_machine(time);
    return sine_out(phase(time, module_frequency, INIT_PHASE)) * amplitude;
}

/*
    This function performs initialization of the system.
    INPUT:
        UINT16 time     - Current system time in ms.
*/
void  FGEN_Init(UINT16 time)
{
    ramp_timer   = time;
    module_reset = 0;
    state = IDLE;
    set_frequency(0, 0, 0, 0);
    state_machine(time);
}
