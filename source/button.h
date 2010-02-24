#ifndef ButtonH
#define ButtonH

#include "gx.h"

class Button {
public:
	int x;
	int y;
	int width;
	int height;
	int grow;
	int clickfade;
	bool clicked;
	GX_Texture *backgroundTexture;
	GX_Texture *textTexture;
	Button(int x, int y, int width, int height, const wchar_t* text);
	~Button();
	void Update();
	void Draw();
	void OnButton(bool isDown);
	bool IsMouseOver(int x, int y);
};

#endif