#include "../xdisplay.h"
#include <cstdio> /* For fputs() */
#include <cstdlib> /* For atexit() */
#include <string> /* For strdup() */

static Display *mainDisplay = NULL;
static int registered = 0;
static char *displayName = NULL;
static int hasDisplayNameChanged = 0;

Display *XGetMainDisplay(void)
{
	/* Close the display if displayName has changed */
	if (hasDisplayNameChanged) {
		XCloseMainDisplay();
		hasDisplayNameChanged = 0;
	}

	if (mainDisplay == NULL) {
		/* First try the user set displayName */
		mainDisplay = XOpenDisplay(displayName);

		if (mainDisplay == NULL) {
			fputs("Could not open main display\n", stderr);
		} else if (!registered) {
			atexit(&XCloseMainDisplay);
			registered = 1;
		}
	}

	return mainDisplay;
}

void XCloseMainDisplay(void)
{
	if (mainDisplay != NULL) {
		XCloseDisplay(mainDisplay);
		mainDisplay = NULL;
	}
}

char * getXDisplay(void)
{
	return displayName;
}

void setXDisplay(const char *name)
{
	displayName = const_cast<char *>(std::string(name).c_str());
	hasDisplayNameChanged = 1;
}
