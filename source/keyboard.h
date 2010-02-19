#ifndef KeyboardH
#define KeyboardH

#include "gx.h"
#include "controller.h"
#include "freetype.h"
#include "gfx/key.h"

#include <wiikeyboard/keyboard.h>
#include <wiikeyboard/wsksymdef.h>

//How big buttons get upon hovering over it
#define	BUTTON_GROW_MAX 5
#define GROW_SPEED 1 //The higher the faster

#define CLICKFADE_SPEED 20

#define PRESS_DELAY 30 //Time key has te be pressed before it starts repeating
#define PRESS_REPEAT 5 //Time between each repeat

#define HOVER_COLOR 0xbec3d6
#define CLICK_COLOR 0xf7f726
#define COMMAND_COLOR 0xAEE9FA

#define KEYBOARD_APPEAR_SPEED 20
#define KEYBOARD_DISAPPEAR_SPEED 35

//VNC Keycodes for command keys
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


#define NUM_KEYS 100

class Keyboard;
class Key;

enum KeyboardType {
	ALPHA_NUMERIC,
	TEXT,
	FULL,
};

/**
 * Interface that can be inherited to listen to all key events
 */
class KeyboardListener {
public:
	virtual void OnKey(int keycode, bool isDown)=0;
};


/**
 * 
 */
class Keyboard : public ControllerListener {
	friend class Key;
	friend class CharacterKey;
	friend class CommandKey;
	friend class ShiftKey;
 
private:
	int position_x, position_y;
	
	//Visibility of keyboard
	int opacity;
	
	//Wether keyboard should be fading in or out
	bool show;	
	
	//Keep track of the shift state of the keyboard
	//Note that these keys are also stored in Keys array, these are just shortcuts
	Key *shiftKey;
	Key *capslockKey;
	
	//Key that is currently depressed
	Key* pressedButton;
	Key *hoverButton;

	//Make sure the USB keyboard only gets initialized ones
	static bool keyboard_inited;
	
	//Object that wants to recieve key events
	KeyboardListener *listener;

	//Array with all keys, can contain NULL values at the end.
	Key *Keys[NUM_KEYS];
	
	
	//Texture that is used to display ALL keys (even the ones with another color)
	GX_Texture *buttonTexture;
	

public:
	Keyboard(int position_x, int position_y, KeyboardType type=FULL);
	~Keyboard();

	void Draw();
	void Update();
	
	void Show();
	void Hide();
	bool Visible();
	bool IsVisible();

	void SetListener(KeyboardListener *listener);
	void OnMouseMove(int x, int y);
	void OnButton(bool isDown);
	bool IsMouseOver(int x, int y);

private:
	bool IsUppercase();
	void UpdateHoverButton();
};
#endif