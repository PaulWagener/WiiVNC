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
	#include <math.h>
#include "viewer.h"
#include <rfb/rfbclient.h>
void Cursor::Update()
{
	PAD_ScanPads();
	if(PAD_ButtonsHeld(0) & PAD_BUTTON_START)
		exit(0);
	if(PAD_ButtonsHeld(0) & PAD_BUTTON_RIGHT)
		x++;
		
	if(PAD_ButtonsDown(0) & PAD_BUTTON_Y)
		Viewer::instance->ZoomIn();
	if(PAD_ButtonsDown(0) & PAD_BUTTON_X)
		Viewer::instance->ZoomOut();
				
	if(PAD_ButtonsHeld(0) & PAD_BUTTON_LEFT)
		x--;
	if(PAD_ButtonsHeld(0) & PAD_BUTTON_B) {
		Viewer::instance->screen_x = 0;
		Viewer::instance->screen_y = 0;
	}
	if(PAD_ButtonsHeld(0) & PAD_BUTTON_UP)
		y--;
	if(PAD_ButtonsHeld(0) & PAD_BUTTON_DOWN)
		y++;
		
	//*
	if(Viewer::instance != NULL && Viewer::instance->status == VNC_CONNECTED) {
		Viewer::instance->screen_x += PAD_StickX(0) / 10;
		Viewer::instance->screen_y -= PAD_StickY(0) / 10;
	}
	//*/
	
	
	
	if(PAD_ButtonsHeld(0) && Viewer::instance != NULL && Viewer::instance->status == VNC_CONNECTED)
		Viewer::instance->SendCursorPosition(x,y);
		
	if(PAD_ButtonsHeld(0) & PAD_BUTTON_A)
		SendPointerEvent(Viewer::instance->client, x, y, 0) ;

}

void Cursor::Draw()
{
	texture->Draw(x, y);
}