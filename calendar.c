#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#define CLEANMASK(mask) (mask & ~(numlockmask|LockMask) & (ShiftMask|ControlMask|Mod1Mask|Mod2Mask|Mod3Mask|Mod4Mask|Mod5Mask))
#define LENGTH(X)       (sizeof X / sizeof X[0])

typedef union {
	int i;
	unsigned int ui;
	float f;
	const void *v;
} Arg;

typedef struct {
	unsigned int mod;
	KeySym keysym;
	void (*func)(const Arg *);
	const Arg arg;
} Key;

/* function declarations */
static void cal(const Arg *);
static void reset(void);
static void quit(void);
static void grabkeys(void);
static void keypress(XEvent *e);
static void updatenumlockmask(void);

/* Globals */
Display *display;
static unsigned int numlockmask = 0;
static Window root;
static GC      gc;
static int kill;
static Window  window;
static int monthOff;
static int yearOff;

#include "config.h"

void cal(const Arg *arg) {
	XClearWindow(display, window);

	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	monthOff += arg->i;
	if (tm.tm_mon+monthOff > 12) { monthOff = monthOff-12; yearOff++; }
	if (tm.tm_mon+monthOff < 1) { monthOff = monthOff+12; yearOff--; }

	char *cmdbuf;
	size_t sz;
	sz = snprintf(NULL, 0, "/bin/cal %d %d", tm.tm_mon+monthOff, tm.tm_year+yearOff);
	cmdbuf = (char *)malloc(sz + 1);
	snprintf(cmdbuf, sz+1, "/bin/cal %d %d", tm.tm_mon+monthOff, tm.tm_year+yearOff);

	FILE *fp;
	char path[1035];

	/* Open the command for reading. */
	fp = popen(cmdbuf, "r");
	if (fp == NULL) {
		printf("Failed to run command\n" );
		exit(1);
	}

	/* Read the output a line at a time - draw it to the window. */
	int y = 1;
	while (fgets(path, sizeof(path), fp) != NULL) {
		XDrawImageString(display, window, gc, 10, y * 20, path, strlen(path)-1);
		y++;
	}

	//free up memory
	free(cmdbuf);

	/* close */
	pclose(fp);
}

void reset() {
	monthOff = 1;
	yearOff = 1900;
	Arg a = {.i = 0};
	cal(&a);
}

void quit() {
	kill = 1;
}

void
keypress(XEvent *e)
{
	int keysyms_per_keycode_return;
	int i;
	XKeyEvent *ev;
    KeySym *keysym = XGetKeyboardMapping(display,
        e->xkey.keycode,
        1,
        &keysyms_per_keycode_return);

	ev = &e->xkey;
	for (i = 0; i < LENGTH(keys); i++)
		if (*keysym == keys[i].keysym
		&& CLEANMASK(keys[i].mod) == CLEANMASK(ev->state)
		&& keys[i].func)
			keys[i].func(&(keys[i].arg));

    XFree(keysym);
}

void
grabkeys(void)
{
	updatenumlockmask();
	{
		unsigned int i, j;
		unsigned int modifiers[] = { 0, LockMask, numlockmask, numlockmask|LockMask };
		KeyCode code;

		XUngrabKey(display, AnyKey, AnyModifier, window);
		for (i = 0; i < LENGTH(keys); i++)
			if ((code = XKeysymToKeycode(display, keys[i].keysym)))
				for (j = 0; j < LENGTH(modifiers); j++)
					XGrabKey(display, code, keys[i].mod | modifiers[j], window,
						True, GrabModeAsync, GrabModeAsync);
	}
}

void
updatenumlockmask(void)
{
	unsigned int i, j;
	XModifierKeymap *modmap;

	numlockmask = 0;
	modmap = XGetModifierMapping(display);
	for (i = 0; i < 8; i++)
		for (j = 0; j < modmap->max_keypermod; j++)
			if (modmap->modifiermap[i * modmap->max_keypermod + j]
				== XKeysymToKeycode(display, XK_Num_Lock))
				numlockmask = (1 << i);
	XFreeModifiermap(modmap);
}

int main(argc,argv)
	int argc;
	char **argv;
{
	/* setup display/screen */
	int screen;
	XEvent xevent;
	display = XOpenDisplay("");
	screen = DefaultScreen(display);
	root = RootWindow(display, screen);

	XColor backgroundColor;
	XColor foregroundColor;
	Colormap colormap;

	colormap = DefaultColormap(display, 0);
	XParseColor(display, colormap, bgColor, &backgroundColor);
	XAllocColor(display, colormap, &backgroundColor);
	XParseColor(display, colormap, fgColor, &foregroundColor);
	XAllocColor(display, colormap, &foregroundColor);


	XSizeHints myhint;
	myhint.x = 1050;
	myhint.y = 0;
	myhint.width = 180;
	myhint.height = 150;
	myhint.flags = PPosition|PSize;

	/* create window */
	window = XCreateSimpleWindow(display, DefaultRootWindow(display),
			myhint.x, myhint.y,
			myhint.width, myhint.height,
			0, foregroundColor.pixel, backgroundColor.pixel);

	/* window manager properties (yes, use of StdProp is obsolete) */
	XSetStandardProperties(display, window, "StatusCalendar", "StatusCalendar",
			None, argv, argc, &myhint);

	/* graphics context */
	gc = XCreateGC(display, window, 0, 0);
	XSetBackground(display, gc, backgroundColor.pixel);
	XSetForeground(display, gc, foregroundColor.pixel);

	char fontname[] = "-*-dejavusansmono nerd font-Medium-R-Normal--12-120-75-75-P-68-ISO8859-1";
	Font font = XLoadFont(display, fontname);
	XSetFont(display, gc, font);

	/* allow receiving mouse events */
	grabkeys();
	XSelectInput(display,window,
			ButtonPressMask|KeyPressMask|ExposureMask);

	/* show up window */
	XMapRaised(display, window);

	monthOff = 1;
	yearOff = 1900;

	/* event loop */
	while(!kill){

		/* fetch event */
		XNextEvent(display, &xevent);

		switch(xevent.type){

			case Expose:
				/* Window was showed. */
				if(xevent.xexpose.count==0) {

					Arg a = {.i = 0};
					cal(&a);

					break;
				}

			case MappingNotify:
				/* Modifier key was up/down. */
				XRefreshKeyboardMapping(&xevent.xmapping);
				break;
			case ButtonPress:
				/* Mouse button was pressed. */
				/* XDrawImageString(xevent.xbutton.display, */
				/*                  xevent.xbutton.window, */
				/*                  gc,  */
				/*                  xevent.xbutton.x, xevent.xbutton.y, */
				/*                  hi, strlen(hi)); */
				break;
			case KeyPress:
				/* Key input. */
				keypress(&xevent);
				break;
		}
	}

	/* finalization */
	XFreeGC(display,gc);
	XDestroyWindow(display, window);
	XCloseDisplay(display);

	exit(0);
}
