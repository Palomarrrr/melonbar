#pragma once
#include "xcommon.h"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>

extern int L_OFFSET, R_OFFSET;

void DisplayTime(Display *dpy, Window *win, FontContext *fctx, GC *gc, unsigned char format_flag);
void DisplayMem(Display *dpy, Window *win, FontContext *fctx, GC *gc, char units, unsigned char get_swap);
void DisplayBattery(Display *dpy, Window *win, FontContext *fctx, GC *gc);
void DisplayCpu(Display *dpy, Window *win, FontContext *fctx, GC *gc);
void DisplayUser(Display *dpy, Window *win, FontContext *fctx, GC *gc);
void DisplayEnvVar(Display *dpy, Window *win, FontContext *fctx, GC *gc, char *var_name, char *format);
void DisplayKernel(Display *dpy, Window *win, FontContext *fctx, GC *gc, unsigned char format);
void DisplayShellCMD(Display *dpy, Window *win, FontContext *fctx, GC *gc, char *command);
//char *GetValueAsPercentBar(double curr_val, double max_val, double min_val, double steps, int *strlen);
