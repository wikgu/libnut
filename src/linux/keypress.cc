#include "../keypress.h"
#include "../deadbeef_rand.h"
#include "../microsleep.h"

#include <cctype> /* For isupper() */
#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <sstream>
#include <iomanip>

#include <X11/extensions/XTest.h>
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

void toggleKey(char c, const bool down, MMKeyFlags flags)
{
	MMKeyCode keyCode = keyCodeForChar(c);

	if (isupper(c) && !(flags & MOD_SHIFT))
	{
		toggleKeyCode(keyCode, down, static_cast<MMKeyFlags>(flags | MOD_SHIFT));
	} else {
		toggleKeyCode(keyCode, down, static_cast<MMKeyFlags>(flags));
	}
}

void tapKey(char c, MMKeyFlags flags)
{
	toggleKey(c, true, flags);
	toggleKey(c, false, flags);
}

int32_t getScratchKeyCode() {
	// TODO: Add link to xdotool src
	Display *dpy = XGetMainDisplay();
	KeySym *keySyms = nullptr;
	int32_t scratchKey;
	int32_t keySymbolsPerKeyCode = 0;
	int32_t keyCodeLow, keyCodeHigh;
	XDisplayKeycodes(dpy, &keyCodeLow, &keyCodeHigh);
	keySyms = XGetKeyboardMapping(dpy, keyCodeLow, keyCodeHigh - keyCodeLow, &keySymbolsPerKeyCode);
	for (int32_t keyCode = keyCodeLow; keyCode < keyCodeHigh; ++keyCode) {
		bool keyIsEmpty = true;
		for (int32_t keySymbolIndex = 0; keySymbolIndex < keySymbolsPerKeyCode; ++ keySymbolIndex) {
			int32_t index = (keyCode - keyCodeLow) * keySymbolsPerKeyCode + keySymbolIndex;
			if (keySyms[index] != 0) {
				keyIsEmpty = false;
			} else {
				break;
			}
		}
		if (keyIsEmpty) {
			scratchKey = keyCode;
			break;
		}
	}
	XFree(keySyms);

	return scratchKey;
}

bool isSurrogate(const char16_t &value) {
	return (value >= 0xD800 && value <= 0xDFFF);
}

char32_t decodeSurrogates(const char16_t &high, const char16_t &low) {
	// See https://en.wikipedia.org/wiki/UTF-16#U+D800_to_U+DFFF
	char32_t highResult = (high - 0xD800) * 0x400;
	char32_t lowResult = low - 0xDC00;
	return (highResult + lowResult + 0x10000);
}

std::vector<std::string> convertToUnicodeSymbols(const std::u16string &str)
{
	std::vector<std::string> symbols(str.length());
	for (auto it = str.begin(); it != str.end();) {
		std::stringstream symbol;
		const char16_t c = *it;
		if (isSurrogate(c)) {
			++it;
			const char16_t lowSurrogate = *it;
			symbol << "U" << std::setw(4) << std::setfill('0') << std::hex << decodeSurrogates(c, lowSurrogate);
		} else {
			symbol << "U" << std::setw(4) << std::setfill('0') << std::hex << c;
		}
		symbols.push_back(symbol.str());
		++it;
	}
	return symbols;
}

void revertKeyboardMapping(Display *dpy, int32_t keyCode) {
	KeySym keySyms[1] = {0};
	XChangeKeyboardMapping(dpy, keyCode, 1, keySyms, 1);
	XFlush(dpy);
}

void typeString(const std::u16string &str)
{
	Display *dpy = XGetMainDisplay();
	int32_t scratchKeyCode = getScratchKeyCode();
	std::vector<std::string> symbols = convertToUnicodeSymbols(str);
	for (auto it = symbols.begin(); it != symbols.end(); ++it) {
		KeySym sym = XStringToKeysym(it->c_str());
		KeySym keySyms[2] = {sym, sym};
		XChangeKeyboardMapping(dpy, scratchKeyCode, 2, keySyms, 1);
		XFlush(dpy);
		KeyCode code = scratchKeyCode;

		XTestFakeKeyEvent(dpy, code, True, CurrentTime);
		XTestFakeKeyEvent(dpy, code, False, CurrentTime);
		revertKeyboardMapping(dpy, scratchKeyCode);
	}
}

