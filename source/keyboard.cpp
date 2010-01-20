#include "keyboard.h"
#include "freetype.h"
#include "gfx/keyboard_tex.h"
#include "gfx/button.h"
#include "controller.h"

#define	BUTTON_GROW_MAX 5

Controller *Keyboard::controller = NULL;
#define HOVER_COLOR 0xbec3d6
#define CLICK_COLOR 0xf7f726
#define COMMAND_COLOR 0xAEE9FA
#define COMMAND_TEXT_COLOR 0x475F66

int cursor_x = 0, cursor_y = 0;

void KeyboardListener::OnKey(int keycode, bool isDown) {}

Key* pressedButton;

/**
 * Key class
 */


Key::Key(Keyboard *theKeyboard, int x, int y, int width) :
	x(x),
	y(y),
	width(width),
	keyboard(theKeyboard),
	hover(false),
	grow(0),
	click_fade(0),
	pressed_counter(0),
	pressed(false),
	sticky(false)
{
	return;
}

Key::~Key()
{
}
	

#define GROW_SPEED 2 //The higher the faster
#define CLICKFADE_SPEED 20 //The higher the faster

#define PRESS_DELAY 30 //Time key has te be pressed before it starts repeating
#define PRESS_REPEAT 10 //Time between each repeat
void Key::Update()
{
	if(hover && grow < BUTTON_GROW_MAX) grow += GROW_SPEED;
	else if(!hover && grow > 0) grow -= GROW_SPEED;
	
	if(click_fade > 0) click_fade -= CLICKFADE_SPEED;
	else click_fade = 0;
	
	if(!sticky && pressed) {
		pressed_counter++;
		
		if(pressed_counter > PRESS_DELAY && pressed_counter % PRESS_REPEAT == 0)
			Trigger();
	}
}

void Key::Trigger()
{
	click_fade = 255;
	if(keyboard->listener != NULL) {
		keyboard->listener->OnKey(GetKeyCode(), true);
	}
}
	
u8 tween(u8 v1, u8 v2, float progress)
{
	return v1 + (v2 - v1) * progress;
}

u32 colortween(u32 from_color, u32 to_color, float progress)
{
	return tween(from_color, to_color, progress) |
			(tween(from_color >> 8, to_color >> 8, progress) << 8) |
			(tween(from_color >> 16, to_color >> 16, progress) << 16) |
			(tween(from_color >> 24, to_color >> 24, progress) << 24);
}

/**
 * Draws the generic key background
 */
void Key::Draw()
{
	u32 base_color = hover ? HOVER_COLOR : this->GetBaseColor();
	u32 color = colortween(base_color, CLICK_COLOR, click_fade / (float)255);
	
	GX_Texture *texture = keyboard->buttonTexture;
	texture->Draw(keyboard->position_x + x - grow, keyboard->position_y + y - grow,
				  width + grow*2, texture->height + grow*2, keyboard->opacity, color);
}

bool Key::IsMouseOver(int mouse_x, int mouse_y) {
	GX_Texture *texture = keyboard->buttonTexture;
	
	return	keyboard->position_x + x < mouse_x && mouse_x < keyboard->position_x + x + width &&
			keyboard->position_y + y < mouse_y && mouse_y < keyboard->position_y + y + texture->height;
}

void Key::Press()
{
	click_fade = 255;
	Trigger();
	pressed_counter = 0;
	pressed = true;
}

void Key::Release()
{
	pressed = false;
}

bool keyboard_inited = false;
#include <wiikeyboard/keyboard.h>


/**
 Character Key
*/
u16 CharacterKey::GetKeyCode()
{
	return key.ch;
}

CharacterKey::CharacterKey(Keyboard *keyboard, int x, int y, struct ch_key key)
: Key(keyboard, x, y, keyboard->buttonTexture->width),
key(key),
lowercase_texture( GX_Text( (char[2]){key.ch, '\0'} , 20, 0) ),
uppercase_texture( GX_Text( (char[2]){key.ucase_ch, '\0'} , 20, 0) )
{
	
}

void CharacterKey::Draw()
{
	Key::Draw();
	
	GX_Texture *texture = keyboard->buttonTexture;
	lowercase_texture.Draw(keyboard->position_x + x + (texture->width/2 - 7) - grow, keyboard->position_y + y - grow,
				  lowercase_texture.width + grow*30, lowercase_texture.height + grow*3, keyboard->opacity);
}

u32 CharacterKey::GetBaseColor()
{
	return 0xFFFFFFFF;
}




/**
 Command Key
 */

CommandKey::CommandKey(Keyboard *keyboard, int x, int y, int width, const char* text, int key_code)
: Key(keyboard, x, y, width),
	text_texture(GX_Text( text, 12, 0)),
	key_code(key_code)
{
}

void CommandKey::Draw()
{
	Key::Draw();
	
	text_texture.Draw(keyboard->position_x + x + 5 - grow, keyboard->position_y + y - grow,
						   text_texture.width + grow*30, text_texture.height + grow*2, keyboard->opacity);
}

u32 CommandKey::GetBaseColor()
{
	return COMMAND_COLOR;
}

u16 CommandKey::GetKeyCode()
{
	return key_code;				
}




/**
 Keyboard
*/

Keyboard::Keyboard() :
	listener(NULL),
	opacity(0),
	position_x(40),
	position_y(300),
	buttonTexture(GX_Texture::LoadFromPNG(button))
{
	//Initialize 
	if(!keyboard_inited) {
		KEYBOARD_Init(NULL);
		keyboard_inited = true;
	}
	
	//Notify the controller that this is the latest keyboard
	//(used for ignoring mouse movements above keyboard)
	controller->SetKeyboard(this);
	
	for(int i = 0; i < NUM_KEYS; i++)
		Keys[i] = NULL;	

	//Initialize a US layout keyboard
	const int row_pos[][2] = {
		{0,0},
		{60, 32},
		{70, 64},
		{90, 96},
	};
	const struct ch_key keys[][13] = {
	{
		{'`','~'},
		{'1','!'},
		{'2','@'},
		{'3','#'},
		{'4','$'},
		{'5','%'},
		{'6','^'},
		{'7','&'},
		{'8','*'},
		{'9','('},
		{'0',')'},
		{'-','_'},
		{'=','+'}
	},
	{
		{'q','Q'},
		{'w','W'},
		{'e','E'},
		{'r','R'},
		{'t','T'},
		{'y','Y'},
		{'u','U'},
		{'i','I'},
		{'o','O'},
		{'p','P'},
		{'[','{'},
		{']','}'},
		{'\\', '|'}
	},
	{
		{'a', 'A'},
		{'s', 'S'},
		{'d', 'D'},
		{'f', 'F'},
		{'g', 'G'},
		{'h', 'H'},
		{'j', 'J'},
		{'k', 'K'},
		{'l', 'L'},
		{';', ':'},
		{'\'', '"'}
	},
	{
		{'z', 'Z'},
		{'x', 'X'},
		{'c', 'C'},
		{'v', 'V'},
		{'b', 'B'},
		{'n', 'N'},
		{'m', 'M'},
		{',', '<'},
		{'.', '>'},
		{'/', '?'}
	}};
	
	int b = 0;
	for(unsigned int row = 0; row < sizeof(keys) / sizeof(keys[0]); row++) {
		int x = row_pos[row][0];
		int y = row_pos[row][1];
		
		//Create a row of buttons
		for (unsigned int i = 0; i < sizeof(keys[row]) / sizeof(keys[row][0]); ++i) {
			if(keys[row][i].ch != '\0')
			   Keys[b++] = new CharacterKey(this, x, y, keys[row][i]);
			
			x += buttonTexture->width;
		}
	}
	//Special extra wide space key
	struct ch_key space_ch = {' ', ' '};
	Key *spaceKey = new CharacterKey(this, 170, 128, space_ch);
	spaceKey->width = 200;
	Keys[b++] = spaceKey;
	
	//Command Keys (left side)
	Keys[b++] = new CommandKey(this, 0, 32, 60, "tab", VNC_TAB);
	Keys[b++] = new CommandKey(this, 0, 64, 70, "capslock", VNC_TAB);
	Keys[b++] = new CommandKey(this, 0, 96, 90, "shift", VNC_SHIFTLEFT);
	Keys[b++] = new CommandKey(this, 0, 128, 60, "ctrl", VNC_CTRLLEFT);
	Keys[b++] = new CommandKey(this, 60, 128, 60, "alt", VNC_ALTLEFT);
	Keys[b++] = new CommandKey(this, 120, 128, 50, "meta", VNC_METALEFT);
	
	//(right side)
	Keys[b++] = new CommandKey(this, 370, 128, 40, "del", VNC_DELETE);
	Keys[b++] = new CommandKey(this, 450, 128, 40, "left", VNC_LEFT);
	Keys[b++] = new CommandKey(this, 490, 128, 40, "down", VNC_DOWN);
	Keys[b++] = new CommandKey(this, 530, 128, 40, "right", VNC_RIGHT);
	Keys[b++] = new CommandKey(this, 490, 96, 40, "up", VNC_UP);
	
	Keys[b++] = new CommandKey(this, 510, 64, 70, "return", VNC_ENTER);
	Keys[b++] = new CommandKey(this, 515, 0, 65, "backspace", VNC_BACKSPACE);
}

Keyboard::~Keyboard() {
	
	for(int i = 0; i < NUM_KEYS; i++)
		if(Keys[i] != NULL)
			delete Keys[i];

	delete buttonTexture;
	controller->SetKeyboard(NULL);
}

	
u16 gkeycode = 0;
u16 keycodeToVNC(u16 keycode) {
	switch(keycode) {
		case KS_Home:		return VNC_HOME;
		case KS_End:		return VNC_END;
		case KS_Left:		return VNC_LEFT;
		case KS_Right:		return VNC_RIGHT;
		case KS_Up:			return VNC_UP;
		case KS_Down:		return VNC_DOWN;
		case KS_Prior:		return VNC_PAGE_UP;
		case KS_Next:		return VNC_PAGE_DOWN;
		case KS_Insert:		return VNC_INSERT;
		case KS_f1:			return VNC_F1;
		case KS_f2:			return VNC_F1;
		case KS_f3:			return VNC_F3;
		case KS_f4:			return VNC_F4;
		case KS_f5:			return VNC_F5;
		case KS_f6:			return VNC_F6;
		case KS_f7:			return VNC_F7;
		case KS_f8:			return VNC_F8;
		case KS_f9:			return VNC_F9;
		case KS_f10:		return VNC_F10;
		case KS_f11:		return VNC_F11;
		case KS_f12:		return VNC_F12;
		case KS_BackSpace:	return VNC_BACKSPACE;
		case KS_Tab:		return VNC_TAB;
		case KS_Return:		return VNC_ENTER;
		case KS_Escape:		return VNC_ESCAPE;
		case KS_Shift_L:	return VNC_SHIFTLEFT;
		case KS_Shift_R:	return VNC_SHIFTRIGHT;
		case KS_Control_L:	return VNC_CTRLLEFT;
		case KS_Control_R:	return VNC_CTRLRIGHT;
		case KS_Meta_L:		return VNC_METALEFT;
		case KS_Meta_R:		return VNC_METARIGHT;
		case KS_Alt_L:		return VNC_ALTLEFT;
		case KS_Alt_R:		return VNC_ALTRIGHT;
		
		//US Localization (row 1)
		case KS_at:			return '`';
		case KS_section:	return '~';
		case KS_quotedbl:	return '@';
		case KS_ampersand:	return '^';
		case KS_underscore:	return '&';
		case KS_parenleft:	return '*';
		case KS_parenright:	return '(';
		case KS_apostrophe:	return ')';
		case KS_slash:		return '-';
		case KS_question:	return '_';
		case KS_degree:		return '=';
		
		//US Localization (row 2)
		case KS_asterisk:	return ']';
		case KS_bar:		return '}';
		case KS_less:		return '\\';
		case KS_greater:	return '|';
		
		//US Localization (row 3)
		case KS_plus:		return ';';
		case KS_plusminus:	return ':';	
			
		//US Localization (row 4)
		case KS_semicolon:	return '<';
		case KS_colon:		return '>';
		case KS_minus:		return '/';
		case KS_equal:		return '?';
	}
	return keycode;
}

#define KEYBOARD_APPEAR_SPEED 20
#define KEYBOARD_DISAPPEAR_SPEED 35

bool Keyboard::Visible()
{
	return opacity > 0;
}
Key *hoverButton = NULL;

void Keyboard::Show()
{
	for(int i = 0; i < NUM_KEYS; i++) {
		if(Keys[i] != NULL) {
			Keys[i]->grow = 0;
			Keys[i]->hover = false;
			if(Keys[i]->IsMouseOver(cursor_x, cursor_y)) {
				Keys[i]->hover = true;
				hoverButton = Keys[i];
			}
		}
	}
	show = true;
}

void Keyboard::Hide()
{
	for(int i = 0; i < NUM_KEYS; i++)
		if(Keys[i] != NULL)
			Keys[i]->hover = false;
	show = false;
}


void Keyboard::Update()
{
	if(show && opacity < 255) opacity += KEYBOARD_APPEAR_SPEED;
	if(!show && opacity > 0) opacity -= KEYBOARD_DISAPPEAR_SPEED;
	
	if(opacity > 255) opacity = 255;
	if(opacity < 0) opacity = 0;
	
	for(int i = 0; i < NUM_KEYS; i++)
		if(Keys[i] != NULL)
			Keys[i]->Update();
	
	//Send keys pressed by a USB keyboard
	keyboard_event ke;
	while(KEYBOARD_GetEvent(&ke))
	{
		bool isDown;
		if(ke.type == KEYBOARD_PRESSED) {
			isDown = true;
		} else if(ke.type == KEYBOARD_RELEASED) {
			isDown = false;
		} else {
			continue;
		}
		
		gkeycode = ke.keycode;

		if(listener != NULL)
			listener->OnKey(keycodeToVNC(ke.symbol), isDown);
	}

}

void Keyboard::Draw()
{
	if(opacity == 0)
		return;
	
	//TODO: Temporary debug code
	char temp[100];
	sprintf(temp, "0x%x", gkeycode);
	GX_Text(temp, 40, 0xFFFFFFFF).Draw(30,30);
	
	//Draw all the keys
	for(int i = 0; i < NUM_KEYS; i++)
		if(Keys[i] != NULL)
			Keys[i]->Draw();

	if(hoverButton && IsVisible())
		hoverButton->Draw();
}

/**
 * Update which key has the mouse hovering above it
 */
void Keyboard::OnCursorMove(int x, int y)
{
	cursor_x = x;
	cursor_y = y;
	
	//Do not update hover information if the user is pressing a key
	if(pressedButton == NULL) {
		hoverButton = NULL;
		
		//Update for all keys wether there is a pointer hovering above it
		for(int i = 0; i < NUM_KEYS; i++) {
			if(Keys[i] != NULL) {
				Keys[i]->hover = Keys[i]->IsMouseOver(x, y);
				if(Keys[i]->hover)
					hoverButton = Keys[i];
			}
		}
	}
}

void Keyboard::SetListener(KeyboardListener *listener)
{
	this->listener = listener;
}

#include <wiikeyboard/wsksymdef.h>

/**
 * User clicks with his pointer
 */
void Keyboard::OnButton(bool isDown)
{
	if(isDown) {
		//Start pressing the key that the user clicked on
		for(int i = 0; i < NUM_KEYS; i++) {
			if(Keys[i] != NULL && Keys[i]->IsMouseOver(cursor_x, cursor_y)) {
				pressedButton = Keys[i];
				pressedButton->Press();
				return;
			}
		}
	} else if(pressedButton != NULL) {
		//Release the key the user last pressed
		pressedButton->Release();
		pressedButton = NULL;
		OnCursorMove(cursor_x, cursor_y); //Updates the hoverButtn
	}
}

/**
 * Returns if the coordinate is hovering over a key
 */
bool Keyboard::IsMouseOver(int x, int y)
{
	if(!IsVisible())
		return false;

	for(int i = 0; i < NUM_KEYS; i++)
	{
		if(Keys[i] != NULL && Keys[i]->IsMouseOver(x, y))
			return true;
	}
	return false;
}

bool Keyboard::IsVisible()
{
	return opacity >= 255;
}