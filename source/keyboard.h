#include "gx.h"

class Keyboard;

class Button {
public:
	int x, y, width, height;
	const char* text;
	Keyboard *keyboard;
	GX_Texture *texture;
	Button(Keyboard *keyboard, const char* text, int x, int y);
	void Update();
	void Draw();
};

class Keyboard {
public:
	int position_x, position_y;
	
	Button *Buttons[100];
	
	GX_Texture *texture;
	GX_Texture *buttonTexture;
	Keyboard();
	~Keyboard();
	void Update();
	void Draw();
};