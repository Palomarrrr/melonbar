#include "xcommon.h"

unsigned long convertColorString(char *color){
    char *colorOut = malloc(sizeof(char) * strnlen(color, 8) + 2);
    colorOut[0] = '0';
    colorOut[1] = 'x';
    for(int i = 1; i  <= strnlen(color, 8); i++)
        colorOut[i + 1] = color[i];
    unsigned long r_int = strtoul(colorOut, NULL, 0);
    free(colorOut);
    return r_int;
}
