#include "config.h"
#include "xcommon.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MODULE_PARAMETER_STATE 0b00001000
#define MODULE_PARAMETER_END_STATE 0b00010000
#define MODULE_STRING_STATE 0b00000100
#define MODULE_NAME_STATE 0b00000010
#define MODULE_MODULE_END_STATE 0b00000001
#define MODULE_END_OF_READ_STATE 0b10000000
#define MODULE_START_READ_STATE 0b01000000

UserConfig user_cfg = {
    .bar_x = -1,
    .bar_y = 0,
    .bar_wid = 1918,
    .bar_hgt = 25,
    .color_font = NULL,
    .color_bar = NULL,
    .color_border = NULL,
    .color_accent = NULL,
    .font_name = NULL,
    .config_location = NULL,
};

int cli_opts[4] = {INT_MIN,INT_MIN,INT_MIN,INT_MIN}; // Holder for command line flags

inline void ParseOptions(int argc, char **argv){
    for(int i = 1; i < argc && argv[i][0] == '-'; i++){
        switch(argv[i][1]){
            case 'x':
                sscanf(argv[i], "-x=%d", &cli_opts[0]);
                break;
            case 'y':
                sscanf(argv[i], "-y=%d", &cli_opts[1]);
                break;
            case 'W':
                sscanf(argv[i], "-W=%d", &cli_opts[2]);
                break;
            case 'H':
                sscanf(argv[i], "-H=%d", &cli_opts[3]);
                break;
            case 'c':
                free(user_cfg.config_location);
                user_cfg.config_location = malloc(sizeof(char) * 200);
                sscanf(argv[i], "-c=%199s", user_cfg.config_location);
                break;
            case 'h':
                printf("Melonbar - A simple, janky, status bar\n");
                printf("Options:\n");
                printf("\t-x=<x-offset>\n\t\tOverride the x offset of the window set in the config file\n");
                printf("\t-y=<y-offset>\n\t\tOverride the y offset of the window set in the config file\n");
                printf("\t-W=<bar-width>\n\t\tOverride the bar width set in the config file\n");
                printf("\t-H=<bar-height>\n\t\tOverride the bar height set in the config file\n");
                printf("\t-c=/path/to/config\n\t\tLook for a config file in a different location than default($HOME/.config/melonbar/melonbar.conf\n");
                printf("\t-h\n\t\tPrint this message\n");
                exit(1);
                break;
            default:
                printf("\nCommand unrecognized...\nuse the '-h' flag to get help\n");
                exit(1);
                break;
        }
    }
}

inline void ReadConfigFile(char *file_name){ // WIP
    FILE *fp;
    fp = fopen(file_name, "r");

    if(fp == NULL){
        THROW_ERR("ReadConfigFile", "Failed to open the config file.\nMake sure that the file exists in your $HOME/.config/melonbar directory\nOr you can pass -c=<file name> to read in a different location\n" );
        exit(1);
    }

    char *curr_line = malloc(sizeof(char) * 550);
    unsigned char module_count = 0; // How many modules have we currently registered

    while(fgets(curr_line, 500, fp)){

        if(curr_line[0] == '\n' || curr_line[0] == '#'){
            continue;
        }

        char *curr_field_label = malloc(sizeof(char) * 20);
        char *curr_field_data = malloc(sizeof(char) * 480); // I know... This is not good

        sscanf(curr_line, "%s = {%s}", curr_field_label, curr_field_data);

        if(!strncmp(curr_field_label, "BASIC", 20)){ // Do shit to parse whatever in here
            sscanf(curr_field_data, "%d,%d,%d,%d}", &user_cfg.bar_x, &user_cfg.bar_y, &user_cfg.bar_wid, &user_cfg.bar_hgt);
            for(int cli = 0; cli < 4; cli++) // This is really janky... but it works for now
                if(cli_opts[cli] != INT_MIN) // If the option equals INT_MIN, then it wasn't changed
                    switch(cli){
                        case 0:
                            user_cfg.bar_x = cli_opts[0];
                            break;
                        case 1:
                            user_cfg.bar_y = cli_opts[1];
                            break;
                        case 2:
                            user_cfg.bar_wid = cli_opts[2];
                            break;
                        case 3:
                            user_cfg.bar_hgt = cli_opts[3];
                    }
                
        }else if(!strncmp(curr_field_label, "COLORS", 20)){
            user_cfg.color_bar = malloc(sizeof(char) * strnlen(curr_field_data, 10));
            user_cfg.color_border = malloc(sizeof(char) * strnlen(curr_field_data, 10));
            user_cfg.color_accent = malloc(sizeof(char) * strnlen(curr_field_data, 10));
            user_cfg.color_font = malloc(sizeof(char) * strnlen(curr_field_data, 10));
            sscanf(curr_field_data, "%7s,%7s,%7s,%7[^}]s", user_cfg.color_bar, user_cfg.color_border, user_cfg.color_accent, user_cfg.color_font);

        }else if(!strncmp(curr_field_label, "FONT", 20)){
            user_cfg.font_name = malloc(sizeof(char) * strnlen(curr_field_data, 100));
            sscanf(curr_field_data, "%[^}]s", user_cfg.font_name); 
            sscanf(user_cfg.font_name, "%*[^:]:size=%d", &user_cfg.font_size);

        }else if(!strncmp(curr_field_label, "MODULES", 20)){ // I hate this
            if(curr_field_data[0] == ' '){
                THROW_ERR("ReadConfigFile", "Found MODULES header with no body\nMake sure your data is all on the same line");
                break;
            }
            unsigned char module_read_state = 0;
            unsigned char parameter_count = 0;
            unsigned short i = 0; // Iterator for arrays

            for(int j = 0; j < strnlen(curr_line, 500); j++){ // Read the whole damn line because sscanf doesn't want to deal with spaces
                // Format should be:
                // MODULES = { MODULENAME[param1=0,param2=1,],MODULENAME[],... }
                // Capture module name then add params in

                switch(curr_line[j]){ // Check for special characters 
                    case ' ': // Skip spaces
                        if(!(module_read_state & MODULE_STRING_STATE)){ // Unless we are recording a string
                            continue;
                        }
                        user_cfg.modules[module_count].module_params[parameter_count][i] = curr_line[j];
                        i++;
                        break;
                    case '\t': // Skip tabs
                        if(!(module_read_state & MODULE_STRING_STATE)){ // Unless we are recording a string
                            continue;
                        }
                        user_cfg.modules[module_count].module_params[parameter_count][i] = curr_line[j];
                        i++;
                        break;
                    case '{': // When we encounter the = sign thats when we start
                        if(!(module_read_state &= MODULE_START_READ_STATE)){
                            module_read_state = 0;
                            module_read_state |= MODULE_NAME_STATE;
                            module_read_state |= MODULE_START_READ_STATE;
                        }
                        break;
                    case '[': // When we have reached the parameter list
                        if(!(module_read_state & MODULE_PARAMETER_STATE)){ // If we are not already reading parameters, start reading them
                            i = 0; // Set the array iterator to 0
                            module_read_state &= ~MODULE_NAME_STATE; // Stop looking for module name
                            module_read_state |= MODULE_PARAMETER_STATE; // Start looking for parameters
                            if(!(curr_line[j+1] == ']')){
                                user_cfg.modules[module_count].module_params[0] = calloc(40, sizeof(char));
                            }
                            parameter_count = 0;
                        }
                        break;
                    case ']': // We have reached the end of the parameter list
                        if(module_read_state & MODULE_PARAMETER_STATE){
                            module_read_state &= ~MODULE_PARAMETER_STATE;
                            i = 0;
                        }
                        break;
                    case ',':
                        if(module_read_state & MODULE_PARAMETER_STATE){ // If we are reading parameters
                            if(parameter_count < 4){
                                parameter_count++;
                                user_cfg.modules[module_count].module_params[parameter_count] = calloc(40, sizeof(char));
                                i = 0;
                            }else{
                                THROW_ERR("ReadConfigFile", "One of your modules has too many parameters\n");
                                exit(1);
                            }
                        }else{ // If we are not reading parameters
                            module_read_state |= MODULE_NAME_STATE;
                            module_count++; // Increment the module counter
                        }
                        break;
                    case '`':
                        if(module_read_state & MODULE_STRING_STATE){ // If we have reached the end of the command
                            module_read_state &= ~MODULE_STRING_STATE;
                        }else{ // else start reading a command
                            module_read_state |= MODULE_STRING_STATE;
                        }
                        break;
                    case '}': // If we have reached the end of the list
                        if(module_read_state & MODULE_START_READ_STATE)
                            module_read_state |= MODULE_END_OF_READ_STATE;
                        break;
                    default:
                        if(module_read_state & MODULE_NAME_STATE && i < 20){
                            user_cfg.modules[module_count].module_name[i] = curr_line[j];
                            i++;
                        }else if(module_read_state & MODULE_PARAMETER_STATE && i < 40){
                            user_cfg.modules[module_count].module_params[parameter_count][i] = curr_line[j];
                            i++;
                        }
                }

                if(module_count > 20 || module_read_state & MODULE_END_OF_READ_STATE){ // If over limit for commands or end of list
                    user_cfg.n_modules = module_count + 1;
                    break; // Just break out
                }

            }
        }else{
            THROW_ERR("ReadConfigFile", "There was an invalid field found in your config file\n");
        }

        free(curr_line);
        free(curr_field_label);
        free(curr_field_data);

        curr_line = malloc(sizeof(char) * 500);
    }
    free(curr_line);
}
