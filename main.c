#include <stdio.h>
#include "harmonic_sign_gen.h"

#define PHASE_BITS				16383	// 16'b0011 1111 1111 1111

int main(void)
{
    FILE *fp;
    fp = fopen("C:/Users/Pantelli/Documents/MATLAB/MEX/test", "w+");

    printf("long long: %d\n", sizeof(long long));
    printf("long %d\n", sizeof(long));
    printf("unsigned long: %d", sizeof(unsigned long));

    FGEN_Init(0);

    printf("Module Frequency: %d\n", module_frequency);
    for (long i = 0; i < PHASE_BITS * 32; i++)
    {
        fprintf(fp, "%d\n", FGEN_GetOutput((UINT16) i));
    }

    FGEN_InputChange(0, 1, 0,0,0,1);

    printf("Module Frequency: %d\n", module_frequency);
    for (long i = 0; i < PHASE_BITS * 16; i++)
    {
        fprintf(fp, "%d\n", FGEN_GetOutput((UINT16) i));
    }

    FGEN_InputChange(0, 1, 0,1,0,0);
    printf("Module Frequency: %d\n", module_frequency);
    for (long i = 0; i < PHASE_BITS * 16; i++)
    {
        fprintf(fp, "%d\n", FGEN_GetOutput((UINT16) i));
    }

    FGEN_InputChange(0, 1, 1,1,1,0);
    printf("Module Frequency: %d\n", module_frequency);
    for (long i = 0; i < PHASE_BITS * 16; i++)
    {
        fprintf(fp, "%d\n", FGEN_GetOutput((UINT16) i));
    }

    FGEN_InputChange(0, 1, 1,1,1,0);
    printf("Module Frequency: %d\n", module_frequency);
    for (long i = 0; i < PHASE_BITS * 16; i++)
    {
        fprintf(fp, "%d\n", FGEN_GetOutput((UINT16) i));
    }

    FGEN_InputChange(0, 1, 1,1,0,1);
    printf("Module Frequency: %d\n", module_frequency);
    for (long i = 0; i < PHASE_BITS * 16; i++)
    {
        fprintf(fp, "%d\n", FGEN_GetOutput((UINT16) i));
    }

    FGEN_InputChange(0, 1, 1,1,0,1);
    printf("Module Frequency: %d\n", module_frequency);
    for (long i = 0; i < PHASE_BITS * 16; i++)
    {
        fprintf(fp, "%d\n", FGEN_GetOutput((UINT16) i));
    }

    FGEN_InputChange(0, 1, 1,0,0,0);
    printf("Module Frequency: %d\n", module_frequency);
    for (long i = 0; i < PHASE_BITS * 16; i++)
    {
        fprintf(fp, "%d\n", FGEN_GetOutput((UINT16) i));
    }

    fclose(fp);

	return 0;
}
