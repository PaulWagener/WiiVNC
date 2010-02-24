/*
 *  Button.c
 *  WiiVNC
 *
 *  Created by Paul Wagener on 19-02-10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "button.h"
#include "gfx/wiibutton.h"
#include "freetype.h"
#include "common.h"
#include "controller.h"


int even(int x)
{
	return x - x % 2;
}

Button::Button(int x, int y, int width, int height, const wchar_t* text) :
	x(x),
	y(y),
	width(width),
	height(height),
	grow(0),
	clickfade(0),
	clicked(false)
{
	textTexture = GX_Text(text, even(height/2.0f), 0x000000);
	backgroundTexture = GX_Texture::LoadFromPNG(wiibutton);
}

Button::~Button()
{
	delete textTexture;
	delete backgroundTexture;
}

void Button::Draw()
{
	u32 color = colortween(0xFFFFFF, 0x2f8c97, clickfade/255.0f);
	backgroundTexture->Draw(x-grow, y-grow, width+grow*2, height+grow*2, 255, 0, color);
	textTexture->Draw(x + width/2 - textTexture->width/2 - grow, y-grow + 5, textTexture->width + grow*2, textTexture->height + grow*2);
}

bool Button::IsMouseOver(int x, int y)
{
	return this->x < x && x < this->x + width
	&& this->y < y && y < this->y + height;
}

void Button::OnButton(bool isDown)
{
	if(isDown && IsMouseOver(Controller::GetX(), Controller::GetY()))
		clickfade = 255;
}

void Button::Update()
{
	if(IsMouseOver(Controller::GetX(), Controller::GetY()))
		grow += 2;
	else
		grow -= 2;
	
	if(grow > height/8) grow = height/8;
	if(grow < 0) grow = 0;
	
	if(clickfade > 0) clickfade -= 20;
	if(clickfade < 0) clickfade = 0;
	if(clickfade > 0 && clickfade < 100) clicked = true;
}
