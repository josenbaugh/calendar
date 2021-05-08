static const char bgColor[] = "#222222";
static const char fgColor[] = "#999999";

static Key keys[] = {
	/*Modifier   Key   function   args*/
	{ 0        , XK_h, cal,       {.i = -1} },
	{ ShiftMask, XK_h, cal,       {.i = -12} },
	{ 0        , XK_l, cal,       {.i = 1} },
	{ ShiftMask, XK_l, cal,       {.i = 12} },
	{ 0        , XK_q, quit,      0},
	{ 0        , XK_r, reset,      0},
};
