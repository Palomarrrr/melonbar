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
int R_OFFSET = 0;

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
    int visual_len = msglen + (msglen * (user_cfg.font_size/2)); // msglen + (msglen * fontsize/2)

    // Print the string
    XftTextExtentsUtf8(dpy, fctx->font, (XftChar8 *)msg, msglen, &fctx->ext);
    XftDrawStringUtf8(fctx->draw, &fctx->fontColor, fctx->font, L_OFFSET, (user_cfg.bar_hgt / 2) + 3, (XftChar8 *)msg, msglen);

    XDrawLine(dpy, *win, *gc, L_OFFSET + visual_len + user_cfg.font_size, 3, L_OFFSET + visual_len + user_cfg.font_size, ((user_cfg.bar_hgt/2) + 3) * 2 - 8);

    L_OFFSET += visual_len + (user_cfg.font_size * 2);
}

// Get the current local time
// TODO - ADD DIFFERENT FORMATS
//  EX. Y-M-D, D-M-Y, 24 hour time, etc
void DisplayTime(Display *dpy, Window *win, FontContext *fctx, GC *gc, unsigned char format_flag){
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

    PrintToBar(dpy, win, fctx, gc, msg, 0);

    free(msg);
}

void DisplayMem(Display *dpy, Window *win, FontContext *fctx, GC *gc, char units, unsigned char get_swap){
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

    PrintToBar(dpy, win, fctx, gc, msg, 0);

    free(msg);
}

void DisplayBattery(Display *dpy, Window *win, FontContext *fctx, GC *gc){
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

    PrintToBar(dpy, win, fctx, gc, msg, 0);

    free(stat);
    free(msg); // Clean up
}

void DisplayCpu(Display *dpy, Window *win, FontContext *fctx, GC *gc){
    static unsigned char err_status = 0;
    FILE *fp;
    static int last_total_jiffies = 0, last_work_jiffies = 0; // Jiffies from the previous run
    int cpu_usr, cpu_nice, cpu_sys, cpu_idle, cpu_iowait, cpu_irq, cpu_sirq; // cpustat fields
    int total_jiffies, work_jiffies; // Current jiffies
    int total_period, work_period; // The difference in jiffies over the period
    float cpu_usage = 0; // The actual cpu usage
    char *msg = malloc(sizeof(char) * 30);

    fp = fopen("/proc/stat", "r");

    if(fp == NULL){
        if(!err_status){
            THROW_ERR("CPUUsage", "Failed to read from /proc/stat\nYou may not have permission to read from this file");
            err_status = 1;
        }
        fclose(fp);
        return;
    }

    fscanf(fp, "cpu %d %d %d %d %d %d %d", &cpu_usr, &cpu_nice, &cpu_sys, &cpu_idle, &cpu_iowait, &cpu_irq, &cpu_sirq); // Read the file
    fclose(fp);

    // Some math to get things how we want them
    total_jiffies = cpu_usr + cpu_nice + cpu_sys + cpu_idle + cpu_iowait + cpu_irq + cpu_sirq;
    work_jiffies = cpu_usr + cpu_nice + cpu_sys;

    total_period = total_jiffies - last_total_jiffies;
    work_period = work_jiffies - last_work_jiffies;

    // Assign the current jiffies to the last jiffies for next run
    last_total_jiffies = total_jiffies;
    last_work_jiffies = work_jiffies;

    cpu_usage = ((float)work_period/(float)total_period) * 100.0;

    snprintf(msg, 30, "CPU:%4.1f%%", cpu_usage); // Trying to format this properly... Please work C

    PrintToBar(dpy, win, fctx, gc, msg, 0);

    free(msg);
}

void DisplayUser(Display *dpy, Window *win, FontContext *fctx, GC *gc){
    static int has_run = 0;
    static char *users_name = NULL;
    static char *host_name = NULL;
    char *msg = malloc(sizeof(char) * 50);

    if(has_run <= 3){ // SHIT DOESN'T GET AUTO FREED LIKE IT SHOULD BE. JUST CACHE IT
        users_name = getenv("USER");
        host_name = getenv("HOSTNAME"); // THIS SHIT RETURNS NULL FOR SOME FUCKING REASON AND I CANT FIGURE OUT WHY
        has_run++;
    }

    snprintf(msg, 50, "%s@%s", users_name, host_name);

    PrintToBar(dpy, win, fctx, gc, msg, 0);

    free(msg);
}

void DisplayEnvVar(Display *dpy, Window *win, FontContext *fctx, GC *gc, char *var_name, char *format){
    char *var_str = getenv(var_name);
    char *msg = malloc(sizeof(char) * 50);

    snprintf(msg, 50, format, var_str);
    PrintToBar(dpy, win, fctx, gc, msg, 0);
    
    free(msg);
}

void DisplayKernel(Display *dpy, Window *win, FontContext *fctx, GC *gc, unsigned char format){
    static int has_run = 0;
    char *msg = malloc(sizeof(char)*50);
    static struct utsname system_name = {0};

    if(!has_run){ // Cache the shit as to not alloc more memory
        uname(&system_name);
    }

    snprintf(msg, 50, "KRN: %s" , system_name.release);
    PrintToBar(dpy, win, fctx, gc, msg, 0);

    free(msg);
}

void DisplayShellCMD(Display *dpy, Window *win, FontContext *fctx, GC *gc, char *command){ // This is displaying weird characters at the end of text... Figure out why
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

    PrintToBar(dpy, win, fctx, gc, msg, 0);

    free(msg);
}
