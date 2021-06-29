#include "../keypress.h"
#include "../deadbeef_rand.h"
#include "../microsleep.h"

#include <cctype> /* For isupper() */

#include <ApplicationServices/ApplicationServices.h>
#import <IOKit/hidsystem/IOHIDLib.h>
#import <IOKit/hidsystem/ev_keymap.h>

static io_connect_t _getAuxiliaryKeyDriver(void)
{
	static mach_port_t sEventDrvrRef = 0;
	mach_port_t masterPort, service, iter;
	kern_return_t kr;

	if (!sEventDrvrRef)
	{
		kr = IOMasterPort(bootstrap_port, &masterPort);
		assert(KERN_SUCCESS == kr);
		kr = IOServiceGetMatchingServices(masterPort, IOServiceMatching(kIOHIDSystemClass), &iter);
		assert(KERN_SUCCESS == kr);
		service = IOIteratorNext(iter);
		assert(service);
		kr = IOServiceOpen(service, mach_task_self(), kIOHIDParamConnectType, &sEventDrvrRef);
		assert(KERN_SUCCESS == kr);
		IOObjectRelease(service);
		IOObjectRelease(iter);
	}
	return sEventDrvrRef;
}

void toggleKeyCode(MMKeyCode code, const bool down, MMKeyFlags flags)
{
	/* The media keys all have 1000 added to them to help us detect them. */
	if (code >= 1000)
	{
		code = code - 1000; /* Get the real keycode. */
		NXEventData event;
		kern_return_t kr;
		IOGPoint loc = {0, 0};
		UInt32 evtInfo = code << 16 | (down ? NX_KEYDOWN : NX_KEYUP) << 8;
		bzero(&event, sizeof(NXEventData));
		event.compound.subType = NX_SUBTYPE_AUX_CONTROL_BUTTONS;
		event.compound.misc.L[0] = evtInfo;
		kr = IOHIDPostEvent(_getAuxiliaryKeyDriver(), NX_SYSDEFINED, loc, &event, kNXEventDataVersion, 0, FALSE);
		assert(KERN_SUCCESS == kr);
	}
	else
	{
		CGEventRef keyEvent = CGEventCreateKeyboardEvent(NULL, (CGKeyCode)code, down);
		assert(keyEvent != NULL);

		CGEventSetType(keyEvent, down ? kCGEventKeyDown : kCGEventKeyUp);
		CGEventSetFlags(keyEvent, flags);
		CGEventPost(kCGSessionEventTap, keyEvent);
		CFRelease(keyEvent);
	}
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
		toggleKeyCode(keyCode, down, flags | MOD_SHIFT);
	} else {
		toggleKeyCode(keyCode, down, flags);
	}

}

void tapKey(char c, MMKeyFlags flags)
{
	toggleKey(c, true, flags);
	toggleKey(c, false, flags);
}

void typeString(const std::u16string &str)
{
	for (auto it = str.begin(); it != str.end(); ++it) {
		UniChar c = *it;
		CGEventRef keyDown = CGEventCreateKeyboardEvent(NULL, 0, true);
		CGEventRef keyUp= CGEventCreateKeyboardEvent(NULL, 0, false);
		CGEventKeyboardSetUnicodeString(keyDown, 1, &c);
		CGEventPost(kCGSessionEventTap, keyDown);
		CGEventKeyboardSetUnicodeString(keyUp, 1, &c);
		CGEventPost(kCGSessionEventTap, keyUp);
		CFRelease(keyDown);
		CFRelease(keyUp);
	}
}
