#ifndef KeyboardH
#define KeyboardH

#include "gx.h"
#include "controller.h"
class Keyboard;

#define VNC_HOME		0xff50
#define VNC_LEFT		0xff51
#define VNC_UP			0xff52
#define VNC_RIGHT		0xff53
#define VNC_DOWN		0xff54
#define VNC_PAGE_UP		0xff55
#define VNC_PAGE_DOWN	0xff56
#define VNC_END			0xff57
#define VNC_INSERT		0xff63
#define VNC_F1			0xffbe
#define VNC_F2			0xffbf
#define VNC_F3			0xffc0
#define VNC_F4			0xffc1
#define VNC_F5			0xffc2
#define VNC_F6			0xffc3
#define VNC_F7			0xffc4
#define VNC_F8			0xffc5
#define VNC_F9			0xffc6
#define VNC_F10			0xffc7
#define VNC_F11			0xffc8
#define VNC_F12			0xffc9
#define VNC_BACKSPACE	0xff08
#define VNC_TAB			0xff09
#define VNC_ENTER		0xff0d
#define VNC_ESCAPE		0xff1b
#define VNC_SHIFTLEFT	0xffe1 
#define VNC_SHIFTRIGHT	0xffe2 
#define VNC_CTRLLEFT	0xffe3 
#define VNC_CTRLRIGHT	0xffe4 
#define VNC_METALEFT	0xffe7 
#define VNC_METARIGHT	0xffe8 
#define VNC_ALTLEFT		0xffe9 
#define VNC_ALTRIGHT	0xffea 
#define VNC_DELETE		0xffff


struct ch_key {
	const char ch;
	const char ucase_ch;
};

class Key {
public:
	int x, y, width;

	Keyboard *keyboard;

	bool hover;

	u8 grow;
	int click_fade; // 0 = no color, 255 = CLICK_COLOR
	
	int pressed_counter;
	bool pressed;
	
	
	Key(Keyboard *keyboard, int x, int y, int width);
	~Key();
	virtual void Update();
	virtual void Draw();
	virtual void Press();
	virtual void Trigger();
	virtual void Release();
	virtual u32 GetBaseColor()=0;
	
	bool IsMouseOver(int x, int y);
	virtual u16 GetKeyCode()=0;
	virtual bool HasKeyCode(u16 keycode)=0;
};

class CharacterKey : public Key {
public:
	struct ch_key key;
	GX_Texture lowercase_texture;
	GX_Texture uppercase_texture;
	CharacterKey(Keyboard *keyboard, int x, int y, struct ch_key key);
	void Draw();
	u16 GetKeyCode();
	bool HasKeyCode(u16 keycode);
	u32 GetBaseColor();
	void Release();
};

class CommandKey : public Key {
public:
	GX_Texture text_texture;
	int key_code;
	CommandKey(Keyboard *keyboard, int x, int y, int width, const char* text, int key_code);
	void Draw();
	u32 GetBaseColor();
	u16 GetKeyCode();
	bool HasKeyCode(u16 keycode);
};

class ShiftKey : public CommandKey {
public:
	ShiftKey(Keyboard *keyboard, int x, int y, int width, const char* text);
	void Press();
	void Release();
	void Update();
};

class KeyboardListener {
public:
	virtual void OnKey(int keycode, bool isDown);
};


#define NUM_KEYS 100
class Keyboard : public ControllerListener {
public:
	Key *shiftKey;
	Key *capslockKey;
	bool usbShiftPressed;
	
	KeyboardListener *listener;
	int opacity;
	bool show;

	//A reference to the controller so that we can get an up-to-date cursor location
	//And let it now to where it should send its buttonpresses.
	//The Controller class fills this field in as soon as it is created
	//As that class is one of the first to be created we can assume that the controller in this field always exists.
	static Controller *controller;
	
	
	int position_x, position_y;
	
	Key *Keys[NUM_KEYS];
	
	GX_Texture *buttonTexture;
	


	Keyboard();
	~Keyboard();
	bool IsUppercase();
	void UpdateHoverButton();
	void Draw();
	void Update();
	void Show();
	void Hide();
	bool Visible();
	void OnCursorMove(int x, int y);
	bool IsMouseOver(int x, int y);
	void SetListener(KeyboardListener *listener);
	void OnButton(bool isDown);
	bool IsVisible();
};
#endif