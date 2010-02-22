#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <ogcsys.h>

#include <rfb/rfbclient.h>
#include "gx.h"
#include "viewer.h"
#include "controller.h"
#include "keyboard.h"
#include "mainscreen.h"
#include "language.h"

static Screen *currentScreen = NULL;

static int fadeOpacity = 255;
static u8 fadeToExit = 0;
static bool firstFade = true;
static Screen *fadeTo = NULL;

void WiiResetPressed() { fadeToExit = SYS_RETURNTOMENU; }
void WiiPowerPressed() { fadeToExit = SYS_POWEROFF_STANDBY;}
void WiimotePowerPressed(s32 chan) { fadeToExit = SYS_POWEROFF_STANDBY; }

bool Fading()
{
	return fadeOpacity > 0;
}

void FadeToScreen(Screen *screen)
{
	if(fadeTo != NULL)
		delete fadeTo;
	
	fadeTo = screen;
}

void FadeToExit()
{
	fadeToExit = SYS_SHUTDOWN;
}

void FadeToRestart()
{
	fadeToExit = SYS_HOTRESET;
}

#define FADEOUT_SPEED 10
#define FADEIN_SPEED 10

int main(int argc, char **argv) {
	
	InitializeLanguage();
	
	//Initialize graphics
	GX_Initialize();
	GX_InitFreetype();	

	//Initialize controls
	Controller::instance();
	
	//Initialize hardware buttons
	SYS_SetResetCallback(WiiResetPressed);
	SYS_SetPowerCallback(WiiPowerPressed);
	WPAD_SetPowerButtonCallback(WiimotePowerPressed);

	currentScreen = new MainScreen();
	Controller::instance()->SetListener(currentScreen);
	
	//Main loop
	while(1) {
	
		Controller::instance()->Update();		

		currentScreen->Update();
		currentScreen->Draw();
		

		//Do the fade between screens
		if((fadeTo != NULL || fadeToExit) && fadeOpacity < 255) fadeOpacity += FADEOUT_SPEED; //fade out
		if(fadeTo == NULL && !fadeToExit && fadeOpacity > 0) fadeOpacity -= FADEIN_SPEED; //fade in
		if(fadeOpacity < 0) { fadeOpacity = 0; firstFade = false; }
		if(fadeOpacity > 255) fadeOpacity = 255;
		
	
		if(fadeToExit || firstFade)
			Controller::instance()->Draw();

		if(fadeOpacity > 0) {
			GX_DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, fadeOpacity);
		}
		
		if(!fadeToExit && !firstFade)
			Controller::instance()->Draw();
	
		GX_Render();
		
		
		//Actually change the screen when nothing is visible
		if((fadeTo != NULL || fadeToExit) && fadeOpacity == 255) {
			if(fadeToExit) {
				SYS_ResetSystem(fadeToExit, 0, 0);
				return 0;
			}
			
			if(currentScreen != NULL)
				delete currentScreen;
			
			currentScreen = fadeTo;
			Controller::instance()->SetListener(currentScreen);
			fadeTo = NULL;
		}
	}
	return 0;
}
