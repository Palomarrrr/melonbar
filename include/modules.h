#pragma once
#include "xcommon.h"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>

#define STYLE_L_ALIGN 0b00000001
#define STYLE_R_ALIGN 0b00000010
#define STYLE_M_ALIGN 0b00000100
// These will be filled out later ig?

extern int L_OFFSET, R_OFFSET;

void DisplayTime(Display *dpy, Window *win, FontContext *fctx, GC *gc, unsigned char format_flag, unsigned char style);
void DisplayMem(Display *dpy, Window *win, FontContext *fctx, GC *gc, char units, unsigned char get_swap, unsigned char style);
void DisplayBattery(Display *dpy, Window *win, FontContext *fctx, GC *gc, unsigned char style);
void DisplayCpu(Display *dpy, Window *win, FontContext *fctx, GC *gc, unsigned char style);
void DisplayUser(Display *dpy, Window *win, FontContext *fctx, GC *gc, unsigned char style);
void DisplayEnvVar(Display *dpy, Window *win, FontContext *fctx, GC *gc, char *var_name, char *format, unsigned char style);
void DisplayKernel(Display *dpy, Window *win, FontContext *fctx, GC *gc, unsigned char format, unsigned char style);
void DisplayShellCMD(Display *dpy, Window *win, FontContext *fctx, GC *gc, char *command, unsigned char style);
//char *GetValueAsPercentBar(double curr_val, double max_val, double min_val, double steps, int *strlen);
