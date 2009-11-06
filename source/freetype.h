#ifndef FreeTypeH
#define FreeTypeH

#include <malloc.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <ogcsys.h>

#include "gfx/helvetica.h" /* A truetype font converted with raw2c */
#include <ft2build.h> /* I presume you have freetype for the Wii installed */
#include FT_FREETYPE_H

#include "gx.h"

GX_Texture GX_Text(const char *string, uint fontSize=20, uint color=0xFFFFFFFF);
extern void GX_InitFreetype(void);
#endif