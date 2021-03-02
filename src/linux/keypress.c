#include "../keypress.h"
#include "../deadbeef_rand.h"
#include "../microsleep.h"

#include <ctype.h> /* For isupper() */

#include <X11/extensions/XTest.h>
#include <stdio.h>
#include "../xdisplay.h"

#define X_KEY_EVENT(display, key, is_press)            \
	(XTestFakeKeyEvent(display,                        \
					   XKeysymToKeycode(display, key), \
					   is_press, CurrentTime),         \
	 XSync(display, false))
#define X_KEY_EVENT_WAIT(display, key, is_press) \
	(X_KEY_EVENT(display, key, is_press),        \
	 microsleep(DEADBEEF_UNIFORM(0.0, 62.5)))

void toggleKeyCode(MMKeyCode code, const bool down, MMKeyFlags flags)
{
	Display *display = XGetMainDisplay();
	const Bool is_press = down ? True : False; /* Just to be safe. */

	/* Parse modifier keys. */
	if (flags & MOD_META)
		X_KEY_EVENT_WAIT(display, K_META, is_press);
	if (flags & MOD_ALT)
		X_KEY_EVENT_WAIT(display, K_ALT, is_press);
	if (flags & MOD_CONTROL)
		X_KEY_EVENT_WAIT(display, K_CONTROL, is_press);
	if (flags & MOD_SHIFT)
		X_KEY_EVENT_WAIT(display, K_SHIFT, is_press);

	X_KEY_EVENT(display, code, is_press);
}

void tapKeyCode(MMKeyCode code, MMKeyFlags flags)
{
	toggleKeyCode(code, true, flags);
	toggleKeyCode(code, false, flags);
}

void toggleKey(unsigned long c, const bool down, MMKeyFlags flags)
{
	MMKeyCode keyCode = keyCodeForChar(c);

	//Prevent unused variable warning for Mac and Linux.

	if (isupper(c) && !(flags & MOD_SHIFT))
	{
		flags |= MOD_SHIFT; /* Not sure if this is safe for all layouts. */
	}

	toggleKeyCode(keyCode, down, flags);
}

void tapKey(unsigned long c, MMKeyFlags flags)
{
	toggleKey(c, true, flags);
	toggleKey(c, false, flags);
}

#define toggleUniKey(c, down) toggleKey(c, down, MOD_NONE)

static void tapUniKey(unsigned long c)
{
	toggleUniKey(c, true);
	toggleUniKey(c, false);
}

void typeString(const char *str)
{
	unsigned short c;
	unsigned short c1;
	unsigned short c2;
	unsigned short c3;
	unsigned long n;

	int scratch_keycode = 0;
    Display *display = XGetMainDisplay();
    KeySym *keysyms = NULL;
    int keysyms_per_keycode = 0;
    int keycode_low, keycode_high;

    XDisplayKeycodes(display, &keycode_low, &keycode_high);
    keysyms = XGetKeyboardMapping(display, keycode_low, keycode_high - keycode_low, &keysyms_per_keycode);

    int i;
    for (i = keycode_low; i <= keycode_high; i++)
    {
        int j = 0;
        int key_is_empty = 1;
        for (j = 0; j < keysyms_per_keycode; j++)
        {
          int symindex = (i - keycode_low) * keysyms_per_keycode + j;

          if(keysyms[symindex] != 0) {
            key_is_empty = 0;
          } else {
            break;
          }
        }
        if(key_is_empty) {
        scratch_keycode = i;
        break;
        }
    }

	while (*str != '\0')
	{
		c = *str++;

		// warning, the following utf8 decoder
		// doesn't perform validation
		if (c <= 0x7F)
		{
			// 0xxxxxxx one byte
			n = c;
		}
		else if ((c & 0xE0) == 0xC0)
		{
			// 110xxxxx two bytes
			c1 = (*str++) & 0x3F;
			n = ((c & 0x1F) << 6) | c1;
		}
		else if ((c & 0xF0) == 0xE0)
		{
			// 1110xxxx three bytes
			c1 = (*str++) & 0x3F;
			c2 = (*str++) & 0x3F;
			n = ((c & 0x0F) << 12) | (c1 << 6) | c2;
		}
		else if ((c & 0xF8) == 0xF0)
		{
			// 11110xxx four bytes
			c1 = (*str++) & 0x3F;
			c2 = (*str++) & 0x3F;
			c3 = (*str++) & 0x3F;
			n = ((c & 0x07) << 18) | (c1 << 12) | (c2 << 6) | c3;
		}

        char buf[6];
        snprintf(buf, sizeof(buf), "U%04lX", n);


        //find the keysym for the given unicode char
        //map that keysym to our previous unmapped keycode
        //click that keycode/'button' with our keysym on it
        KeySym sym = XStringToKeysym(buf);
        KeySym keysym_list[] = { sym, sym };
        XChangeKeyboardMapping(display, scratch_keycode, 2, keysym_list, 1);
        KeyCode code = scratch_keycode;

		XTestFakeKeyEvent(display, code, true, CurrentTime);
		XTestFakeKeyEvent(display, code, false, CurrentTime);

        XSync(display, false);
        microsleep(DEADBEEF_UNIFORM(0.0, 62.5));
	}

    KeySym keysym_list[] = { 0 };
    XChangeKeyboardMapping(display, scratch_keycode, 1, keysym_list, 1);
}

void typeStringDelayed(const char *str, const unsigned cpm)
{
	/* Characters per second */
	const double cps = (double)cpm / 60.0;

	/* Average milli-seconds per character */
	const double mspc = (cps == 0.0) ? 0.0 : 1000.0 / cps;

	while (*str != '\0')
	{
		tapUniKey(*str++);
		microsleep(mspc + (DEADBEEF_UNIFORM(0.0, 62.5)));
	}
}
