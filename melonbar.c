#include "./include/xcommon.h"
#include "./include/modules.h"
#include "./include/config.h"

#include <X11/X.h>
#include <X11/XKBlib.h>
#include <X11/Xatom.h>
#include <X11/Xft/Xft.h>
#include <X11/Xft/XftCompat.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrender.h>
#include <stdio.h>
#include <string.h>

// Globals
Display *dpy = NULL;
int screen = 0;

static inline void GCInit(Window *window, GC *gc, XGCValues *xgc_values, FontContext *fontctx){
    // Set up font stuff
    fontctx->fontName = malloc(sizeof(char) * strnlen(user_cfg.font_name, 235));
    fontctx->colorName = malloc(sizeof(char) * strnlen(user_cfg.color_font, 10));
    fontctx->fontName = user_cfg.font_name;
    fontctx->colorName = user_cfg.color_font;
    fontctx->font = XftFontOpenName(dpy, screen, fontctx->fontName);
    fontctx->visual = DefaultVisual(dpy, screen);
    fontctx->colorMap = DefaultColormap(dpy, screen);
    XftColorAllocName(dpy, fontctx->visual, fontctx->colorMap, fontctx->colorName, &fontctx->fontColor);

    // Set up XGC stuff
    xgc_values->line_style = LineSolid;
    xgc_values->line_width = 1;
    xgc_values->cap_style = CapButt;
    xgc_values->join_style = JoinMiter;
    xgc_values->fill_style = FillSolid;
    xgc_values->foreground = convertColorString(user_cfg.color_accent);
    xgc_values->background = convertColorString(user_cfg.color_accent);

    // Create a value mask to keep track of what we have
    unsigned long xgc_value_mask = GCForeground | GCBackground |GCLineStyle | GCLineWidth | GCCapStyle | GCJoinStyle | GCFillStyle;

    // Finally create the GC so we can actually do shit
    *gc = XCreateGC(dpy, *window, xgc_value_mask, xgc_values);
}

static inline void DisplayInit(Window *window, XSetWindowAttributes *xwa, GC *gc, XGCValues *xgc_values, FontContext *fontctx, XSizeHints *xsh){
   if(!(dpy = XOpenDisplay(NULL))){
        fprintf(stderr, "done fucked up\n"); 
        exit(1);
   }
   screen = DefaultScreen(dpy);

   xwa->background_pixel = convertColorString(user_cfg.color_bar);
   xwa->border_pixel = convertColorString(user_cfg.color_border);
   xwa->event_mask = ButtonPressMask|ButtonMotionMask|ButtonReleaseMask|KeyPressMask|ExposureMask;
   *window = XCreateWindow(dpy, RootWindow(dpy, screen), user_cfg.bar_x, user_cfg.bar_y, user_cfg.bar_wid, user_cfg.bar_hgt, 1, DefaultDepth(dpy, screen), InputOutput, DefaultVisual(dpy, screen), CWBackPixel | CWEventMask | CWBorderPixel, xwa);

   GCInit(window, gc, xgc_values, fontctx);

   xsh->min_width = user_cfg.bar_wid;
   xsh->min_height = user_cfg.bar_hgt; 
   xsh->max_width = user_cfg.bar_wid; 
   xsh->max_height = user_cfg.bar_hgt; 
   xsh->flags = PMinSize | PMaxSize; //| PPosition;
   XSetSizeHints(dpy, *window, xsh, XA_WM_NORMAL_HINTS);

   XStoreName(dpy,*window, "Status Bar");
   XMapWindow(dpy, *window);
}

int main(int argc, char **argv){
    // Variables
    Window window = 0;
    XSetWindowAttributes xwa;
    XSizeHints xsh;
    GC gc = 0;
    XGCValues xgc_values;
    XEvent ev = {0};
    FontContext fontctx = {
        .colorName = NULL,
        .fontName = NULL,
    };
    // Allocate and create the config file path
    user_cfg.config_location = malloc(sizeof(char) * 200);
    strncat(user_cfg.config_location, getenv("HOME"), 169); // Should leave enough space for the config path and null terminator
    strcat(user_cfg.config_location, "/.config/melonbar/melonbar.conf");

    // Parse command line flags
    ParseOptions(argc, argv);
    ReadConfigFile(user_cfg.config_location);

    // Init the dpy and draw the window
    DisplayInit(&window, &xwa, &gc, &xgc_values, &fontctx, &xsh);

    // Loop till you die... or something
    while(1){
        // Create the font drawable
        fontctx.draw = XftDrawCreate(dpy, window, fontctx.visual, fontctx.colorMap);

        // Reset the offsets
        L_OFFSET = 15;
        R_OFFSET = user_cfg.bar_wid - 15;

        XClearWindow(dpy, window);
        //XNextEvent(dpy, &ev); // Grab the next event

        for(int i = 0; i < user_cfg.n_modules; i++){ // TODO - Find a way to cache this instead of constantly running
            if(!strncmp(user_cfg.modules[i].module_name, "DisplayTime", 20)){
                DisplayTime(dpy, &window, &fontctx, &gc, 0);
            }else if(!strncmp(user_cfg.modules[i].module_name, "DisplayMem", 20)){
                DisplayMem(dpy, &window, &fontctx, &gc, user_cfg.modules[i].module_params[0][0], atoi(user_cfg.modules[i].module_params[1]));
            }else if(!strncmp(user_cfg.modules[i].module_name, "DisplayBattery", 20)){
                DisplayBattery(dpy, &window, &fontctx, &gc);
            }else if(!strncmp(user_cfg.modules[i].module_name, "DisplayCpu", 20)){
                DisplayCpu(dpy, &window, &fontctx, &gc);
            }else if(!strncmp(user_cfg.modules[i].module_name, "DisplayUser", 20)){
                DisplayUser(dpy, &window, &fontctx, &gc);
            }else if(!strncmp(user_cfg.modules[i].module_name, "DisplayKernel", 20)){
                DisplayKernel(dpy, &window, &fontctx, &gc, 0);
            }else if(!strncmp(user_cfg.modules[i].module_name, "DisplayEnvVar", 20)){
                DisplayEnvVar(dpy, &window, &fontctx, &gc, user_cfg.modules[i].module_params[0], user_cfg.modules[i].module_params[1]);
            }else if(!strncmp(user_cfg.modules[i].module_name, "DisplayShellCMD", 20)){
                DisplayShellCMD(dpy, &window, &fontctx, &gc, user_cfg.modules[i].module_params[0]);
            }

        }

        XSync(dpy, True); // Update the display
        usleep(500000);
        XftDrawDestroy(fontctx.draw);
    }

    // Queue the clean up song
    XFreeGC(dpy, gc);
    XUnmapWindow(dpy, window);
    XDestroyWindow(dpy, window);
    XCloseDisplay(dpy);

    return 0;
}
