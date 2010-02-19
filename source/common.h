//Color animation functions

#ifndef CommonH
#define CommonH

#include <ogcsys.h>
u8 tween(u8 v1, u8 v2, float progress);
u32 colortween(u32 from_color, u32 to_color, float progress);

#endif