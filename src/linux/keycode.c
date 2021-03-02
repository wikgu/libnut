#include "../keycode.h"
#include <stdio.h>

/*
 * Structs to store key mappings not handled by XStringToKeysym() on some
 * Linux systems.
 */

struct XSpecialCharacterMapping {
	char name;
	MMKeyCode code;
};

struct XSpecialCharacterMapping XSpecialCharacterTable[] = {
	{'~', XK_asciitilde},
  	{'_', XK_underscore},
  	{'[', XK_bracketleft},
  	{']', XK_bracketright},
  	{'!', XK_exclam},
  	{'\'', XK_quotedbl},
  	{'#', XK_numbersign},
  	{'$', XK_dollar},
  	{'%', XK_percent},
  	{'&', XK_ampersand},
  	{'\'', XK_quoteright},
  	{'*', XK_asterisk},
  	{'+', XK_plus},
  	{',', XK_comma},
  	{'-', XK_minus},
  	{'.', XK_period},
  	{'?', XK_question},
  	{'<', XK_less},
  	{'>', XK_greater},
  	{'=', XK_equal},
  	{'@', XK_at},
  	{':', XK_colon},
  	{';', XK_semicolon},
  	{'\\', XK_backslash},
  	{'`', XK_grave},
  	{'{', XK_braceleft},
  	{'}', XK_braceright},
  	{'|', XK_bar},
  	{'^', XK_asciicircum},
  	{'(', XK_parenleft},
  	{')', XK_parenright},
  	{' ', XK_space},
  	{'/', XK_slash},
  	{'\t', XK_Tab},
  	{'\n', XK_Return}
};

MMKeyCode keyCodeForChar(const unsigned long c)
{
	MMKeyCode code;
    int index;
    char buf[6];
    snprintf(buf, sizeof(buf), "U%04lX", c);

	code = XStringToKeysym(buf);
	if (code == NoSymbol) {
		/* Some special keys are apparently not handled properly by
		 * XStringToKeysym() on some systems, so search for them instead in our
		 * mapping table. */
		size_t i;
		const size_t specialCharacterCount =
			sizeof(XSpecialCharacterTable) / sizeof(XSpecialCharacterTable[0]);
		for (i = 0; i < specialCharacterCount; ++i) {
			if (c == XSpecialCharacterTable[i].name) {
				code = XSpecialCharacterTable[i].code;
				break;
			}
		}
	}

	return code;
}
