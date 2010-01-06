#ifndef KeyboardH
#define KeyboardH

#include "gx.h"
#include "controller.h"
class Keyboard;

struct key {
	const char ch;
	const char ucase_ch;
};

class Button {
public:
	int x, y, width, height;

	Keyboard *keyboard;
	GX_Texture *texture;
	struct key key;
	GX_Texture text_texture;
	bool visible;
	
	bool hover;
	int grow;
	
	Button(Keyboard *keyboard, const struct key key, int x, int y);
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
	int opacity;
	bool show;
	
	static Controller *controller;
	int position_x, position_y;
	
	Button *Buttons[NUM_BUTTONS];
	
	GX_Texture *texture;
	GX_Texture *buttonTexture;
	KeyboardListener *listener;
	Keyboard();
	~Keyboard();
	void Draw();
	void Update();
	void Show();
	void Hide();
	bool Visible();
	void OnCursorMove(int x, int y);
	bool IsMouseOver(int x, int y);
	void SetListener(KeyboardListener *listener);
	void OnButton(bool isDown);
};
#endif