#pragma once

#include "os.h"
#include "inline_keywords.h" /* For H_INLINE */
#include <cstddef>
#include <cstdint>

/* Some generic, cross-platform types. */

typedef struct {
	int64_t x;
	int64_t y;
} MMPoint;

typedef struct {
	int64_t width;
	int64_t height;
} MMSize;

typedef struct _MMRect {
	MMPoint origin;
	MMSize size;
} MMRect;

H_INLINE MMPoint MMPointMake(int64_t x, int64_t y)
{
	MMPoint point;
	point.x = x;
	point.y = y;
	return point;
}

H_INLINE MMSize MMSizeMake(int64_t width, int64_t height)
{
	MMSize size;
	size.width = width;
	size.height = height;
	return size;
}

H_INLINE MMRect MMRectMake(int64_t x, int64_t y, int64_t width, int64_t height)
{
	MMRect rect;
	rect.origin = MMPointMake(x, y);
	rect.size = MMSizeMake(width, height);
	return rect;
}

#define MMPointZero MMPointMake(0, 0)

#if defined(IS_MACOSX)

#define CGPointFromMMPoint(p) CGPointMake((CGFloat)(p).x, (CGFloat)(p).y)
#define MMPointFromCGPoint(p) MMPointMake((size_t)(p).x, (size_t)(p).y)

#elif defined(IS_WINDOWS)

#define MMPointFromPOINT(p) MMPointMake((size_t)p.x, (size_t)p.y)

#endif

typedef int64_t WindowHandle;
