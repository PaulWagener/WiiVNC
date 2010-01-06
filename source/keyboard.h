#ifndef KeyboardH
#define KeyboardH

#include "gx.h"
#include "controller.h"
class Keyboard;

class Button {
public:
	int x, y, width, height;
	const char* text;
	Keyboard *keyboard;
	GX_Texture *texture;
	GX_Texture text_texture;
	bool visible;
	
	bool hover;
	int grow;
	
	Button(Keyboard *keyboard, const char* text, int x, int y);
	~Button();
	void Update();
	void Draw();
	bool IsMouseOver(int x, int y);
};

class KeyboardListener {
public:
	virtual void OnKey(int keycode, bool isDown);
};


#define NUM_BUTTONS 100
class Keyboard : public ControllerListener {
public:
	static Controller *controller;
	int position_x, position_y;
	
	Button *Buttons[NUM_BUTTONS];
	
	GX_Texture *texture;
	GX_Texture *buttonTexture;
	KeyboardListener *listener;
	Keyboard();
	~Keyboard();
	void Update();
	void Draw();
	void OnCursorMove(int x, int y);
	bool IsMouseOver(int x, int y);
	void SetListener(KeyboardListener *listener);
	void OnButton(bool isDown);
};
#endif