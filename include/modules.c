#include "modules.h"
#include "config.h"
#include "xcommon.h"
#include <X11/X.h>
#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*************************************
 *  Some built-in modules to keep
 *  basic functions from slowing
 *  things down
*************************************/

int L_OFFSET = 0;
int R_OFFSET = 0; // TO BE IMPLEMENTED

/*
char *GetValueAsPercentBar(double curr_val, double max_val, double min_val, double steps, int *strlen){
    char *return_str;
    if(steps > 1){ // Can't have a step value more than 1
        fprintf(stderr, "ERROR: YOU CANT HAVE A STEP GREATER THAN 1");
        return "ERR";
    }

    double n_steps = 1 / steps; // How many steps will the output have
    *strlen = (int)n_steps + 2; // The final string length
    double scaled_val = (curr_val - min_val) / (max_val - min_val); // Scale between 1-0
    double steps_taken = (scaled_val / steps); // Figure out how many steps the value has gone up

    return_str = malloc(sizeof(char) * *strlen); // Allocate the string some memory
    return_str[0] = '<', return_str[*strlen - 1] = '>'; // Put the bookends on

    for(int i = 1; i < *strlen - 1; i++){ // Go through the string and find where to put the marker
        if(i == (int)steps_taken) return_str[i] = '|';
        else if(i < (int)steps_taken) return_str[i] = '='; // Things behind the marker should be this
        else return_str[i] = '-'; // Things ahead of the marker should be this
    }

    return return_str;
}
*/

void PrintToBar(Display *dpy, Window *win, FontContext *fctx, GC *gc, char *msg, unsigned char style){
   // Get the string length 
    int msglen = strnlen(msg, 255);

    // Print the string
    XftTextExtentsUtf8(dpy, fctx->font, (XftChar8 *)msg, msglen, &fctx->ext);

    if(style & STYLE_R_ALIGN){
        XftDrawStringUtf8(fctx->draw, &fctx->fontColor, fctx->font, R_OFFSET - fctx->ext.width, (user_cfg.bar_hgt + fctx->ext.y) / 2, (XftChar8 *)msg, msglen);
        XDrawLine(dpy, *win, *gc, R_OFFSET - fctx->ext.width - user_cfg.font_size, 3, R_OFFSET - fctx->ext.width - user_cfg.font_size, ((user_cfg.bar_hgt/2) + 3) * 2 - 8);
        R_OFFSET -= fctx->ext.width + (user_cfg.font_size * 2); // Set the offset for next module
    }else{ // Else just assume left align ig?
        XftDrawStringUtf8(fctx->draw, &fctx->fontColor, fctx->font, L_OFFSET, (user_cfg.bar_hgt + fctx->ext.y) / 2, (XftChar8 *)msg, msglen);
        XDrawLine(dpy, *win, *gc, L_OFFSET + fctx->ext.width + user_cfg.font_size, 3, L_OFFSET + fctx->ext.width + user_cfg.font_size, ((user_cfg.bar_hgt/2) + 3) * 2 - 8);
        L_OFFSET += fctx->ext.width + (user_cfg.font_size * 2); // Set the offset for next module
    }
}

// Get the current local time
// TODO - Add different formats ex. y-m-d, d-m-y, 24 hour time, etc
void DisplayTime(Display *dpy, Window *win, FontContext *fctx, GC *gc, unsigned char format_flag, unsigned char style){
    char *msg= malloc(sizeof(char) * 32);
    time_t t = time(NULL);
    struct tm *local_time= localtime(&t);

    // Figure out whether it is AM or PM
    char *ampm = NULL;
    if(local_time->tm_hour >= 12){
        ampm = "PM";
        if(local_time->tm_hour >= 13){
            local_time->tm_hour -= 12;
        }
    }else ampm = "AM";

    // Create the return string
    snprintf(msg, 32, "%02d-%02d-%04d | %02d:%02d:%02d %s", local_time->tm_mon+1, local_time->tm_mday, local_time->tm_year+1900, local_time->tm_hour, local_time->tm_min, local_time->tm_sec, ampm);

    PrintToBar(dpy, win, fctx, gc, msg, style);

    free(msg);
}

// Display the current memory usage
// I don't this is working correctly either... Need to look into it
// TODO - Add a preprocessor switch for Linux and other OSs so that this can be used on BSDs
void DisplayMem(Display *dpy, Window *win, FontContext *fctx, GC *gc, char units, unsigned char get_swap, unsigned char style){
    static unsigned char err_status = 0;
    struct sysinfo info;
    sysinfo(&info);
    char *msg = malloc(sizeof(char) * 50);
    int max_mem = 0, used_mem = 0;
    int divisor = 1000000000; // To get bytes to gigabytes

    switch(units){ // Hey! Actually utilizing case fall through makes me feel smart ok >:(
        case 'B':
            divisor /= 1000;
        case 'K':
            divisor /= 1000;
        case 'M':
            divisor /= 1000;
        case 'G':
            break;
        default:
            if(!(err_status & 0b00000001) && !get_swap){
                THROW_ERR("RAMUsage","An invalid memory unit was given to the module\nValid memory units include 'G', 'M', 'K', and 'B'");
                err_status |= 0b00000001;
            }else if(!(err_status & 0b00000010) && get_swap){
                THROW_ERR("SwapUsage","An invalid memory unit was given to the module\nValid memory units include 'G', 'M', 'K', and 'B'");
                err_status |= 0b00000010;
            }
    }
    
    // These are in bytes so keep that in mind when dealing with this
    if(!get_swap){
        max_mem = info.totalram / divisor;
        used_mem = (info.totalram - info.freeram) / divisor;
        snprintf(msg, 50, "MEM:%d%cb/%d%cb", used_mem, units, max_mem, units);
    }else{
        max_mem = info.totalswap / divisor;
        used_mem = (info.totalswap - info.freeswap) / divisor;
        snprintf(msg, 50, "SWP:%d%cb/%d%cb", used_mem, units, max_mem, units);
    }

    PrintToBar(dpy, win, fctx, gc, msg, style);

    free(msg);
}

// Display battery percentage
void DisplayBattery(Display *dpy, Window *win, FontContext *fctx, GC *gc, unsigned char style){
    static unsigned char err_status = 0; // This should act as a basic error flag
    FILE *fp;

    char *msg = malloc(sizeof(char) * 30);
    char *stat = malloc(sizeof(char) * 13);
    int bat_percent = 0;

    // Get the battery percentage
    fp = fopen("/sys/class/power_supply/BAT0/capacity", "r");

    if(fp == NULL){ // If this file doesn't exist, the user might be on a desktop
        if(!err_status){ // Only print this once
            THROW_ERR("BatteryStatus", "There was an error opening /sys/class/power_supply/BAT0/capacity\nIf you're on a desktop, please disable this module");
            err_status = 1;
        }
        snprintf(msg, 50, "BAT: N/A");
    }else{
        fscanf(fp, "%d", &bat_percent);
        fclose(fp);
    }

    // Get the battery status
    if(!err_status)
        fp = fopen("/sys/class/power_supply/BAT0/status", "r");

    if(fp == NULL){ // If this file doesn't exist, the user might be on a desktop
        if(!err_status){ // Only print this once
            THROW_ERR("BatteryStatus", "There was an error opening /sys/class/power_supply/BAT0/status\nIf you're on a desktop, please disable this module");
            err_status = 1;
        }
        snprintf(msg, 50, "BAT: N/A");
    }else{
        fscanf(fp, "%s", stat);
        fclose(fp);
        snprintf(msg, 50, "BAT:%d%% (%s)", bat_percent, stat);
    }

    PrintToBar(dpy, win, fctx, gc, msg, style);

    free(stat);
    free(msg); // Clean up
}

// Display CPU usage
// I don't think this works properly... Need to look deeper into it
void DisplayCpu(Display *dpy, Window *win, FontContext *fctx, GC *gc, unsigned char style){
    static unsigned char err_status = 0;
    FILE *fp;
    int n_cores; // number of cpu cores
    float curr_jobs = 0; // The actual cpu usage
    char *msg = malloc(sizeof(char) * 30);

    fp = fopen("/proc/cpuinfo", "r");

    if(fp == NULL){
        if(!err_status){
            THROW_ERR("CPUUsage", "Failed to read from /proc/cpuinfo\nYou may not have permission to read from this file");
            err_status = 1;
        }
        fclose(fp);
        return;
    }
    
    for(int i = 0; i < 10; i++){ // Skip to line 11
        fscanf(fp, "%*[^\n]\n");
        if(i == 9){
            fscanf(fp, "%*[^:]: %d", &n_cores);
        }
    }
    
    fclose(fp);

    fp = fopen("/proc/loadavg", "r");

    if(fp == NULL){
        if(!err_status){
            THROW_ERR("CPUUsage", "Failed to read from /proc/loadavg\nYou may not have permission to read from this file");
            err_status = 1;
        }
        fclose(fp);
        return;
    }
    
    fscanf(fp, "%f\n", &curr_jobs);
    fclose(fp);

    snprintf(msg, 29, "CPU: %4.1f%%", (curr_jobs * 100.0) / (float)n_cores); // Calculate the percentage and put it in the buffer
    
    PrintToBar(dpy, win, fctx, gc, msg, style);

    free(msg);
}

// Display the current user@hostname
void DisplayUser(Display *dpy, Window *win, FontContext *fctx, GC *gc, unsigned char style){
    static int has_run = 0;
    static char *users_name = NULL;
    static char *host_name = NULL;
    char *msg = malloc(sizeof(char) * 50);

    if(!has_run){  // This doesn't really need to be ran more than once
        users_name = getenv("USER");
        host_name = getenv("HOSTNAME"); // THIS SHIT RETURNS NULL FOR SOME FUCKING REASON AND I CANT FIGURE OUT WHY AAAAAAAAAAAAAAAAAAAAAA
        has_run = 1;
    }

    snprintf(msg, 50, "%s@%s", users_name, host_name);

    PrintToBar(dpy, win, fctx, gc, msg, style);

    free(msg);
}

// Display an environment variable
void DisplayEnvVar(Display *dpy, Window *win, FontContext *fctx, GC *gc, char *var_name, char *format, unsigned char style){
    char *var_str = getenv(var_name); // Thinking about caching this so it doesn't have to constantly run
    char *msg = malloc(sizeof(char) * 50);

    snprintf(msg, 50, format, var_str);
    PrintToBar(dpy, win, fctx, gc, msg, style);
    
    free(msg);
}

// Display info about the current kernel version
// TODO - Implement the format flag to allow the user to display whatever they want
void DisplayKernel(Display *dpy, Window *win, FontContext *fctx, GC *gc, unsigned char format, unsigned char style){
    static int has_run = 0;
    char *msg = malloc(sizeof(char)*50);
    static struct utsname system_name = {0};

    if(!has_run){ // This doesn't need to run more than once
        uname(&system_name);
        has_run = 1;
    }

    snprintf(msg, 50, "KRN: %s" , system_name.release);
    PrintToBar(dpy, win, fctx, gc, msg, style);

    free(msg);
}

void DisplayShellCMD(Display *dpy, Window *win, FontContext *fctx, GC *gc, char *command, unsigned char style){ 
    static unsigned char err_status = 0;
    FILE *fp;
    char *msg = malloc(sizeof(char) * 50);
    fp = popen(command, "r");

    if(fp == NULL){
        if(!err_status){
            THROW_ERR("DisplayShellCMD", "There was a problem running the provided command.\nMake sure to check your formatting");
            err_status = 1;
        }
        free(msg);
        return;
    }

    fgets(msg, 49, fp); // Grab the output of the command
    pclose(fp);

    msg[strnlen(msg,50) - 1] = '\0';

    PrintToBar(dpy, win, fctx, gc, msg, style);

    free(msg);
}
