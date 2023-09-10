#pragma once
//Standard headers
#include <X11/extensions/Xrender.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <locale.h>
#include <time.h>

//X11 Headers
#include <X11/X.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>
#include <X11/XF86keysym.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/xpm.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <X11/Xproto.h>
#include <X11/Xresource.h>
#include <X11/Xthreads.h>
#include <X11/Xft/Xft.h>

#include <sys/utsname.h>
#ifdef __gnu_linux__ // MAKE THIS ACTUALLY POSIX COMPLIANT. SWITCH TO SYS/SYSCTL
#include <sys/sysinfo.h>
#else
#include <sys/sysctl.h>
#include <sys/types.h>
#endif
    

#define THROW_ERR(module, msg) fprintf(stderr, "\x1b[31m==========| ERROR |==========\x1b[0m\n"), fprintf(stderr, "\x1b[31m%s: %s\x1b[0m\n",module, msg) // Make the process of throwing errors a little less monotonous

typedef struct {
	XftDraw *draw;
	XftColor fontColor;
	char *colorName;
	Drawable pixmap;
	Visual *visual;
	Colormap colorMap;
	int depth;
	char *fontName;
	XftFont *font;
    XGlyphInfo ext;
}FontContext;

typedef struct {
    int bar_x, bar_y; // Bar position
    int bar_h, bar_w; // Bar height and width
    int left_align_offset; // The left align offset. This should be updated every time a new module is added to the bar on the left
    int right_align_offset; // Right align offset. Same as left however you'll need to subtract the width of the module to be added from it to work properly
    int vertical_offset; // Vertical offset. I'm guessing how this is calculated is this... (floor(bar_h / 2) + 3). At least it works for everything I've tried so far
}BarContext;

unsigned long convertColorString(char *color);
