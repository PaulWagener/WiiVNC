#include "keyboard.h"
#include "freetype.h"
#include "gfx/keyboard_tex.h"
#include "gfx/button.h"

Button *b;

Button::Button(Keyboard *theKeyboard, const char* theText, int x, int y) 
{
	this->x = x;
	this->y = y;
	keyboard = theKeyboard;
	text = theText;
	texture = keyboard->buttonTexture;
}

void Button::Draw()
{
	texture->Draw(keyboard->position_x + x, keyboard->position_y + y);
	GX_Texture text_texture = GX_Text(text, 30, 0);
	
	text_texture.Draw(keyboard->position_x + x + (texture->width/2 - 6), 
					  keyboard->position_y + y);
}

Keyboard::Keyboard() {
	
	position_x = 10;
	position_y = 100;
	buttonTexture = GX_Texture::LoadFromPNG(button);
	texture = GX_Texture::LoadFromPNG(keyboard_tex);
	b = new Button(this, "h", 10, 10);

}

Keyboard::~Keyboard() {
	delete texture;
}

void Keyboard::Update()
{
}

void Keyboard::Draw()
{
	texture->Draw(position_x, position_y);
	GX_Text("Space", 14, 0x0).Draw(position_x, position_y);
	b->Draw();
}
