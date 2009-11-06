/*
 *  cursor.cpp
 *  WiiVNC
 *
 *  Created by Paul Wagener on 31-10-09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "cursor.h"
#include <gccore.h>
#include "gfx/cursor_hand.h"

Cursor::Cursor()
{
	x = 320;
	y = 240;
	PAD_Init();
	texture = GX_Texture::LoadFromPNG(cursor_hand);
}

Cursor::~Cursor()
{
	delete texture;
}

#include <stdlib.h>
#include "viewer.h"
#include <rfb/rfbclient.h>
void Cursor::Update()
{
	PAD_ScanPads();
	if(PAD_ButtonsHeld(0) & PAD_BUTTON_START)
		exit(0);
	if(PAD_ButtonsHeld(0) & PAD_BUTTON_RIGHT)
		x++;
	if(PAD_ButtonsHeld(0) & PAD_BUTTON_LEFT)
		x--;
	if(PAD_ButtonsHeld(0) & PAD_BUTTON_UP)
		y--;
	if(PAD_ButtonsHeld(0) & PAD_BUTTON_DOWN)
		y++;
	
	if(PAD_ButtonsHeld(0) && Viewer::_instance != NULL && Viewer::_instance->status == CONNECTED)
		SendPointerEvent(Viewer::_instance->client, x, y, 0) ;
		
	if(PAD_ButtonsHeld(1) & PAD_BUTTON_A)
		SendPointerEvent(Viewer::_instance->client, x, y, 1) ;
}

void Cursor::Draw()
{
	texture->Draw(x, y);
}