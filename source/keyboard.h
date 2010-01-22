#ifndef KeyboardH
#define KeyboardH

#include "gx.h"
#include "controller.h"
#include "freetype.h"
#include "gfx/button.h"
#include "controller.h"

#include <wiikeyboard/keyboard.h>
#include <wiikeyboard/wsksymdef.h>
class Keyboard;

//How big buttons get upon hovering over it
#define	BUTTON_GROW_MAX 5
#define GROW_SPEED 2 //The higher the faster

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

//A character / uppercase character pair
struct ch_key {
	const char ch;
	const char ucase_ch;
};


/**
 * The keys are the clickable buttons that make up the keyboard
 */
class Key {
public:
	//Position & size of key
	int x, y, width;
	
	//How yellow the button is
	int click_fade; // 0 = no color, 255 = CLICK_COLOR

	//Sticky buttons have the property that the key-up and key-down
	//events can be sent seperately depending on how long the key is pressed
	bool sticky;
	
	//Keeps track of if this key is pressed by the USB keyboard
	//Onscreen keyboard may not release keys that have been pressed by USB keyboard
	bool pressedByUSB;
	
	//Keep track of how long the button has been pressed
	//(used for repeat functionality when key is depressed longer than normal)
	bool pressed;

protected:
	int pressed_counter;
	
	//Reference to the keyboard to use for keyboard-wide variables
	Keyboard *keyboard;
	
	//How much bigger the button is than normal
	u8 grow;
		
public:
	Key(Keyboard *keyboard, int x, int y, int width);
	virtual void Update();
	virtual void Draw();
	
	virtual void Press();
	virtual void Release();
	
	virtual u16 GetKeyCode()=0;
	virtual bool HasKeyCode(u16 keycode)=0;
	bool IsMouseOver(int x, int y);
protected:
	virtual u32 GetBaseColor()=0;
private:
	virtual void Trigger();
};



/**
 * A key that can be pressed to print a character,
 * is used for all the dull grey keys.
 * Note that these keys can change character if shift is pressed
 */
class CharacterKey : public Key {
private:
	//Character & uppercase character this key represents
	struct ch_key key;
	
	//Texture chaches for the characters
	GX_Texture lowercase_texture;
	GX_Texture uppercase_texture;

public:
	CharacterKey(Keyboard *keyboard, int x, int y, struct ch_key key);
	void Draw();
	u16 GetKeyCode();
	bool HasKeyCode(u16 keycode);
protected:
	u32 GetBaseColor();
};

/**
 * Key that doesn't print a character, these are the lightblue keys with text on them.
 */
class CommandKey : public Key {
private:
	GX_Texture text_texture;
	int key_code; //Keycode this key sends

public:
	CommandKey(Keyboard *keyboard, int x, int y, int width, const char* text, int key_code, bool sticky);
	void Draw();
	bool HasKeyCode(u16 keycode);
	u16 GetKeyCode();
protected:
	u32 GetBaseColor();
};


/**
 * Interface that can be inherited to listen to all key events
 */
class KeyboardListener {
public:
	virtual void OnKey(int keycode, bool isDown);
};


/**
 * 
 */
class Keyboard : public ControllerListener {
	friend class Key;
	friend class CharacterKey;
	friend class CommandKey;
	friend class ShiftKey;
	
public:
	//A reference to the controller so that we can get an up-to-date cursor location
	//And let it now to where it should send its buttonpresses.
	//The Controller class fills this field in as soon as it is created
	//As that class is one of the first to be created we can assume that the controller in this field always exists.
	static Controller *controller;
	
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
	
	
	//Texture that is used to display ALL keys (even the ones with another color
	GX_Texture *buttonTexture;
	

public:
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
	bool IsVisible();
	
private:
	bool IsUppercase();
	void UpdateHoverButton();
};
#endif