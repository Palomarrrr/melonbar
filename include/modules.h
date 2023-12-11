#pragma once
#include "xcommon.h"
#include "config.h"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>

#define STYLE_L_ALIGN 0b00000001
#define STYLE_R_ALIGN 0b00000010
#define STYLE_M_ALIGN 0b00000100
// These will be filled out later ig?

extern int L_OFFSET, R_OFFSET;

//char *GetValueAsPercentBar(double curr_val, double max_val, double min_val, double steps, int *strlen);
void DisplayTime(Display *dpy, Window *win, FontContext *fctx, GC *gc, void *module);
void DisplayMem(Display *dpy, Window *win, FontContext *fctx, GC *gc, void *module);
void DisplayBattery(Display *dpy, Window *win, FontContext *fctx, GC *gc, void *module);
void DisplayCpu(Display *dpy, Window *win, FontContext *fctx, GC *gc, void *module);
void DisplayUser(Display *dpy, Window *win, FontContext *fctx, GC *gc, void *module);
void DisplayEnvVar(Display *dpy, Window *win, FontContext *fctx, GC *gc, void *module);
void DisplayKernel(Display *dpy, Window *win, FontContext *fctx, GC *gc, void *module);
void DisplayShellCMD(Display *dpy, Window *win, FontContext *fctx, GC *gc, void *module);
