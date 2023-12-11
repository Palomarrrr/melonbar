#pragma once 
#include "xcommon.h"
#include "modules.h"
#include <X11/X.h>
#include <X11/Xlib.h>

/*
typedef void (*ModuleFN)(Display*,Window*,FontContext*,GC*,void*); // Pointer to the function to call for this module
typedef struct _Module{
    unsigned char style; // Bitflag
    char module_name[20]; // Name of the module
    char *module_params[4]; // There should only be 4 params per module
    //void (*module_fn)(Display*,Window*,FontContext*,GC*,_Module*); // Pointer to the function to call for this module
    ModuleFN fn;
}Module;

typedef struct {
    int bar_x, bar_y;
    int bar_wid, bar_hgt;
    char *font_name;
    char *color_bar, *color_border, *color_accent, *color_font;
    int font_size;
    char *config_location;
    unsigned char n_modules;
    Module modules[12];
}UserConfig;
*/

extern UserConfig user_cfg;

//void ParseConfig();
void ParseOptions(int argc, char **argv);
void ReadConfigFile(char *file_name);
