/*
 *  cursor.cpp
 *  WiiVNC
 *
 *  Created by Paul Wagener on 31-10-09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "controller.h"
#include <gccore.h>
#include "gfx/cursor_default.h"

//Listener stubs
void ControllerListener::OnButton(bool isDown) {}
void ControllerListener::OnSecondaryButton(bool isDown) {}
void ControllerListener::OnMiddleButton(bool isDown) {}
void ControllerListener::OnScrollUp() {}
void ControllerListener::OnScrollDown() {}
void ControllerListener::OnZoomIn(bool isDown) {}
void ControllerListener::OnZoomOut(bool isDown) {}
void ControllerListener::OnHome() {}
void ControllerListener::OnCursorMove(int x, int y) {}
void ControllerListener::OnKeyboard() {}
void ControllerListener::OnScrollView(int x, int y) {}

#include "gx.h"
Controller::Controller()
{
	x = SCREEN_XCENTER;
	y = SCREEN_YCENTER;
	listener = NULL;
	PAD_Init();
	texture = GX_Texture::LoadFromPNG(cursor_default);
}

Controller::~Controller()
{
	delete texture;
}

#include <stdlib.h>
#include <math.h>
#include "viewer.h"
#include <rfb/rfbclient.h>
void Controller::Update()
{
	
	//GC Controller
	PAD_ScanPads();
	
	int pad_down = PAD_ButtonsDown(0);
	int pad_up = PAD_ButtonsUp(0);
	int pad_held = PAD_ButtonsHeld(0);
	

	
	//Move the mouse with the big stick
	int px = PAD_StickX(0) / 10;
	int py = PAD_StickY(0) / 10;
	x += px;
	y -= py;
	if(x < 0) x = 0;
	if(x > SCREEN_WIDTH) x = SCREEN_WIDTH;
	if(y < 0) y = 0;
	if(y > SCREEN_HEIGHT) y = SCREEN_HEIGHT;
	
	if(pad_down & PAD_BUTTON_START) exit(0);
	
	if(listener != NULL) {
		//Button event handlers
		if(pad_down & PAD_BUTTON_A) listener->OnButton(true);
		if(pad_up & PAD_BUTTON_A) listener->OnButton(false);
		if(pad_down & (PAD_BUTTON_LEFT | PAD_BUTTON_RIGHT)) listener->OnMiddleButton(true);
		if(pad_up & (PAD_BUTTON_LEFT | PAD_BUTTON_RIGHT)) listener->OnMiddleButton(false);
		if(pad_down & PAD_BUTTON_B) listener->OnSecondaryButton(true);
		if(pad_up & PAD_BUTTON_B) listener->OnSecondaryButton(false);
		
		if(pad_down & PAD_BUTTON_UP) listener->OnScrollUp();
		if(pad_down & PAD_BUTTON_DOWN) listener->OnScrollDown();
		
		if(pad_down & PAD_BUTTON_Y) listener->OnZoomIn(true);
		if(pad_up & PAD_BUTTON_Y) listener->OnZoomIn(false);
		if(pad_down & PAD_BUTTON_X) listener->OnZoomOut(true);
		if(pad_up & PAD_BUTTON_X) listener->OnZoomOut(false);		

		if(pad_down & PAD_BUTTON_START) listener->OnHome();

		if(pad_down & PAD_TRIGGER_Z) listener->OnKeyboard();
		
		if(px != 0 || py != 0)
			listener->OnCursorMove(x, y);
		
		//Scroll the view with the C stick
		int cx = PAD_SubStickX(0) / 10;
		int cy = PAD_SubStickY(0) / 10;
		if(cx != 0 || cy != 0)
			listener->OnScrollView(cx, cy);	
	}
}

void Controller::Draw()
{
	texture->Draw(x, y);
}
