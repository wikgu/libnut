#include "../keypress.h"
#include "../deadbeef_rand.h"
#include "../microsleep.h"

#include <cctype> /* For isupper() */

/* Convenience wrappers around ugly APIs. */
#define WIN32_KEY_EVENT_WAIT(key, flags) \
	(win32KeyEvent(key, flags), Sleep(DEADBEEF_RANDRANGE(0, 63)))

void win32KeyEvent(int key, MMKeyFlags flags)
{
	int scan = MapVirtualKey(key & 0xff, MAPVK_VK_TO_VSC);

	/* Set the scan code for extended keys */
	switch (key)
	{
	case VK_RCONTROL:
	case VK_SNAPSHOT: /* Print Screen */
	case VK_RMENU:	  /* Right Alt / Alt Gr */
	case VK_PAUSE:	  /* Pause / Break */
	case VK_HOME:
	case VK_UP:
	case VK_PRIOR: /* Page up */
	case VK_LEFT:
	case VK_RIGHT:
	case VK_END:
	case VK_DOWN:
	case VK_NEXT: /* 'Page Down' */
	case VK_INSERT:
	case VK_DELETE:
	case VK_LWIN:
	case VK_RWIN:
	case VK_APPS: /* Application */
	case VK_VOLUME_MUTE:
	case VK_VOLUME_DOWN:
	case VK_VOLUME_UP:
	case VK_MEDIA_NEXT_TRACK:
	case VK_MEDIA_PREV_TRACK:
	case VK_MEDIA_STOP:
	case VK_MEDIA_PLAY_PAUSE:
	case VK_BROWSER_BACK:
	case VK_BROWSER_FORWARD:
	case VK_BROWSER_REFRESH:
	case VK_BROWSER_STOP:
	case VK_BROWSER_SEARCH:
	case VK_BROWSER_FAVORITES:
	case VK_BROWSER_HOME:
	case VK_LAUNCH_MAIL:
	{
		flags = flags | KEYEVENTF_EXTENDEDKEY;
		break;
	}
	}

	/* Set the scan code for keyup */
	if (flags & KEYEVENTF_KEYUP)
	{
		scan |= 0x80;
	}

	INPUT keyboardInput;
	keyboardInput.type = INPUT_KEYBOARD;
	keyboardInput.ki.wScan = (WORD)scan;
	keyboardInput.ki.dwFlags = KEYEVENTF_SCANCODE | flags;
	keyboardInput.ki.time = 0;
	SendInput(1, &keyboardInput, sizeof(keyboardInput));
}

void toggleKeyCode(MMKeyCode code, const bool down, MMKeyFlags flags)
{
	const DWORD dwFlags = down ? 0 : KEYEVENTF_KEYUP;

	/* Parse modifier keys. */
	if (flags & MOD_META)
		WIN32_KEY_EVENT_WAIT(K_META, dwFlags);
	if (flags & MOD_ALT)
		WIN32_KEY_EVENT_WAIT(K_ALT, dwFlags);
	if (flags & MOD_CONTROL)
		WIN32_KEY_EVENT_WAIT(K_CONTROL, dwFlags);
	if (flags & MOD_SHIFT)
		WIN32_KEY_EVENT_WAIT(K_SHIFT, dwFlags);

	win32KeyEvent(code, dwFlags);
}

void tapKeyCode(MMKeyCode code, MMKeyFlags flags)
{
	toggleKeyCode(code, true, flags);
	toggleKeyCode(code, false, flags);
}

void toggleKey(char c, const bool down, MMKeyFlags flags)
{
	MMKeyCode keyCode = keyCodeForChar(c);

	//Prevent unused variable warning for Mac and Linux.
	int modifiers;

	if (isupper(c) && !(flags & MOD_SHIFT))
	{
		flags = flags | MOD_SHIFT; /* Not sure if this is safe for all layouts. */
	}

	modifiers = keyCode >> 8; // Pull out modifers.
	if ((modifiers & 1) != 0)
		flags = flags | MOD_SHIFT; // Uptdate flags from keycode modifiers.
	if ((modifiers & 2) != 0)
		flags = flags | MOD_CONTROL;
	if ((modifiers & 4) != 0)
		flags = flags | MOD_ALT;
	keyCode = keyCode & 0xff; // Mask out modifiers.
	toggleKeyCode(keyCode, down, flags);
}

void tapKey(char c, MMKeyFlags flags)
{
	toggleKey(c, true, flags);
	toggleKey(c, false, flags);
}

void typeString(const std::u16string &str)
{
	for (auto it = str.begin(); it != str.end(); ++it) {
		INPUT keyPress;
		keyPress.type = INPUT_KEYBOARD;
		keyPress.ki.wVk = 0;
		keyPress.ki.wScan = static_cast<WORD>(*it);
		keyPress.ki.dwFlags = KEYEVENTF_UNICODE;
		keyPress.ki.time = 0;
		SendInput(1, &keyPress, sizeof(keyPress));
		keyPress.ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
		SendInput(1, &keyPress, sizeof(keyPress));
	}
}
