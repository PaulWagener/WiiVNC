/**
 *	Extra convenience functions for using the Wii Graphics Library 
 */

#ifndef GxH
#define GxH

#include <string.h>
#include <malloc.h>
#include <ogcsys.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define SCREEN_XCENTER SCREEN_WIDTH / 2
#define SCREEN_YCENTER SCREEN_HEIGHT / 2

/**
 * GX_Texture is a wrapper around the GXtexObj provided by Nintendo
 * It keeps track of the original texture buffer and its dimensions
 */
class GX_Texture {
public:
	int height;
	int width;
	
	//Texture buffer in the specified format
	u8* buffer;

	GX_Texture(int width, int height, u8 format=GX_TF_RGBA8, u8* buffer=NULL);
	~GX_Texture();
    	
	void Draw(int x, int y, int width=-1, int height=-1, int opacity=255, int degrees=0, int color=0xFFFFFF);
	void Flush();
	
	static GX_Texture* LoadFromPNG(const void* png);
private:
	u8 format;
	GXTexObj texObj;
};

void GX_DrawRectangle(int x, int y, int width, int height, int color, u8 opacity=255);
int GX_Initialize();
void GX_Render();

#endif
