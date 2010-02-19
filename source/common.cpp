#include "common.h"

u8 tween(u8 v1, u8 v2, float progress) {
	return v1 + (v2 - v1) * progress;
}

u32 colortween(u32 from_color, u32 to_color, float progress) {
	return tween(from_color, to_color, progress) |
	(tween(from_color >> 8, to_color >> 8, progress) << 8) |
	(tween(from_color >> 16, to_color >> 16, progress) << 16) |
	(tween(from_color >> 24, to_color >> 24, progress) << 24);
}
