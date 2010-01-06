#include "keyboard.h"
#include "freetype.h"
#include "gfx/keyboard_tex.h"
#include "gfx/button.h"
#include "controller.h"

#define	BUTTON_GROW_MAX 5

Controller *Keyboard::controller = NULL;

void KeyboardListener::OnKey(int keycode, bool isDown) {}

Button::Button(Keyboard *theKeyboard, const struct key key, int x, int y) :
	x(x),
	y(y),
	keyboard(theKeyboard),
	texture(keyboard->buttonTexture),
	key(key),
	text_texture( GX_Text( (char[2]){key.ch, '\0'} , 20, 0) ),
	visible(true),
	hover(false),
	grow(0)
{
	return;
}

Button::~Button()
{
}
	

#define GROW_SPEED 2
void Button::Update()
{
	if(hover && grow < BUTTON_GROW_MAX)
		grow += GROW_SPEED;
	else if(!hover && grow > 0)
		grow -= GROW_SPEED;
}

#define HOVER_COLOR 0xbec3d6
#define CLICK_COLOR 0xf7f726
void Button::Draw()
{
	if(visible) {
		texture->Draw(keyboard->position_x + x - grow, keyboard->position_y + y - grow,
					  texture->width + grow*2, texture->height + grow*2, keyboard->opacity, hover ? HOVER_COLOR : 0xFFFFFF);
	
		
		text_texture.Draw(keyboard->position_x + x + (texture->width/2 - 7) - grow, keyboard->position_y + y - grow,
						text_texture.width + grow*30, text_texture.height + grow*3, keyboard->opacity);
								  
	}
}

bool Button::IsMouseOver(int mouse_x, int mouse_y) {
	return	keyboard->position_x + x < mouse_x && mouse_x < keyboard->position_x + x + texture->width &&
			keyboard->position_y + y < mouse_y && mouse_y < keyboard->position_y + y + texture->height;
}

bool keyboard_inited = false;
#include <wiikeyboard/keyboard.h>



Keyboard::Keyboard()
{
	opacity = 0;
	show = false;
	
	if(!keyboard_inited) {
		KEYBOARD_Init(NULL);
		keyboard_inited = true;
	}
	
	controller->SetKeyboard(this);
	
	listener = NULL;
	position_x = 40;
	position_y = 300;
	buttonTexture = GX_Texture::LoadFromPNG(button);
	texture = GX_Texture::LoadFromPNG(keyboard_tex);
	
	for(int i = 0; i < NUM_BUTTONS; i++)
		Buttons[i] = NULL;	

	const int row_pos[][2] = {
		{0,0},
		{60, 32},
		{70, 64},
		{90, 96},
	};
	const struct key keys[][13] = {
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
			   Buttons[b++] = new Button(this, keys[row][i], x, y);
			
			x += buttonTexture->width;
		}
	}
		
	
}

Keyboard::~Keyboard() {
	
	for(int i = 0; i < NUM_BUTTONS; i++)
		if(Buttons[i] != NULL)
			delete Buttons[i];

	delete texture;
	delete buttonTexture;
	controller->SetKeyboard(NULL);
}

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
#define VNC_LEFTSHIFT	0xffe1 
#define VNC_RIGHTSHIFT	0xffe2 
#define VNC_CTRLLEFT	0xffe3 
#define VNC_CTRLRIGHT	0xffe4 
#define VNC_METALEFT	0xffe7 
#define VNC_METARIGHT	0xffe8 
#define VNC_ALTLEFT		0xffe9 
#define VNC_ALTRIGHT	0xffea 
					
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
		case KS_Shift_L:	return VNC_LEFTSHIFT;
		case KS_Shift_R:	return VNC_RIGHTSHIFT;
		case KS_Control_L:	return VNC_CTRLLEFT;
		case KS_Control_R:	return VNC_CTRLRIGHT;
		case KS_Meta_L:		return VNC_METALEFT;
		case KS_Meta_R:		return VNC_METARIGHT;
		case KS_Alt_L:		return VNC_ALTLEFT;
		case KS_Alt_R:		return VNC_ALTRIGHT;
	}
	return keycode;
}

#define KEYBOARD_APPEAR_SPEED 20
#define KEYBOARD_DISAPPEAR_SPEED 35

bool Keyboard::Visible()
{
	return opacity > 0;
}

void Keyboard::Show()
{
	show = true;
}

void Keyboard::Hide()
{
	show = false;
}


void Keyboard::Update()
{
	if(show && opacity < 255) opacity += KEYBOARD_APPEAR_SPEED;
	if(!show && opacity > 0) opacity -= KEYBOARD_DISAPPEAR_SPEED;
	
	if(opacity > 255) opacity = 255;
	if(opacity < 0) opacity = 0;
	
	for(int i = 0; i < NUM_BUTTONS; i++)
		if(Buttons[i] != NULL)
			Buttons[i]->Update();
	
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

		if(listener != NULL)
			listener->OnKey(keycodeToVNC(ke.symbol), isDown);
	}

}
Button *hoverButton = NULL;

void Keyboard::Draw()
{
	//texture->Draw(position_x, position_y, -1, -1, opacity);
	
	for(int i = 0; i < NUM_BUTTONS; i++)
		if(Buttons[i] != NULL)
			Buttons[i]->Draw();

	if(hoverButton && opacity == 255)
		hoverButton->Draw();
}

int cursor_x = 0, cursor_y = 0;

void Keyboard::OnCursorMove(int x, int y)
{
	cursor_x = x;
	cursor_y = y;
	hoverButton = NULL;
	for(int i = 0; i < NUM_BUTTONS; i++) {
		if(Buttons[i] != NULL) {
			Buttons[i]->hover = Buttons[i]->IsMouseOver(x, y);
			if(Buttons[i]->hover)
				hoverButton = Buttons[i];
			 
		}
	}
}

void Keyboard::SetListener(KeyboardListener *listener)
{
	this->listener = listener;
}

#include <wiikeyboard/wsksymdef.h>
void Keyboard::OnButton(bool isDown)
{
	if(listener != NULL) {
		for(int i = 0; i < NUM_BUTTONS; i++) {
			if(Buttons[i] != NULL && Buttons[i]->IsMouseOver(cursor_x, cursor_y)) {
				listener->OnKey(Buttons[i]->key.ch, isDown);
				return;
			}
		}
		
	}
}

bool Keyboard::IsMouseOver(int x, int y)
{
	return	position_x < x && x < position_x + texture->width &&
			position_y < y && y < position_y + texture->height;
}