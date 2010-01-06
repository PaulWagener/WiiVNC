#include "keyboard.h"
#include "freetype.h"
#include "gfx/keyboard_tex.h"
#include "gfx/button.h"
#include "controller.h"

#define	BUTTON_GROW_MAX 5

Controller *Keyboard::controller = NULL;

void KeyboardListener::OnKey(int keycode, bool isDown) {}

Button::Button(Keyboard *theKeyboard, const char* theText, int x, int y) :
	x(x),
	y(y),
	text(theText),
	keyboard(theKeyboard),
	texture(keyboard->buttonTexture),
	text_texture(GX_Text(text, 30, 0)),
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

void Button::Draw()
{
	if(visible) {
		texture->Draw(keyboard->position_x + x - grow, keyboard->position_y + y - grow,
					  texture->width + grow*2, texture->height + grow*2);
	
		
		text_texture.Draw(keyboard->position_x + x + (texture->width/2 - 10) - grow, keyboard->position_y + y - grow,
						text_texture.width + grow*30, text_texture.height + grow*3);
								  
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
	if(!keyboard_inited) {
		KEYBOARD_Init(NULL);
		keyboard_inited = true;
	}
	
	controller->SetKeyboard(this);
	
	listener = NULL;
	position_x = 10;
	position_y = 100;
	buttonTexture = GX_Texture::LoadFromPNG(button);
	texture = GX_Texture::LoadFromPNG(keyboard_tex);
	
	for(int i = 0; i < NUM_BUTTONS; i++)
		Buttons[i] = NULL;	

	static const char*const keys[] = {
		"q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "[", "]"
	};
	
	/*static const char*const keys2[] = {
		"a", "s", "d", "f", "h", "j", "k", "l"
	};*/
	
	static const int x_start = 27;
	static const int x_delta = 47;
	static const int i_start = 0;
	int y = 37;
	for (unsigned int i = 0; i < sizeof(keys) / sizeof(keys[0]); ++i)
		Buttons[i + i_start] = new Button(this, keys[i], x_start + i * x_delta, y);
	
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
					
					

void Keyboard::Update()
{
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
	texture->Draw(position_x, position_y);
	for(int i = 0; i < NUM_BUTTONS; i++)
		if(Buttons[i] != NULL && !Buttons[i]->hover)
			Buttons[i]->Draw();

	if(hoverButton)
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
				listener->OnKey(Buttons[i]->text[0], isDown);
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