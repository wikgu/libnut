#include "../keycode.h"

MMKeyCode keyCodeForChar(const unsigned long c)
{
	return VkKeyScan(c);
}