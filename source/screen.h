#ifndef ScreenH
#define ScreenH

#include "controller.h"
/**
 * A screen is a screen-filling 
 */
class Screen : public ControllerListener {
public:

	virtual ~Screen();
	virtual void Update()=0;
	virtual void Draw()=0;
};

//static Screen *currentScreen = NULL;

void FadeToScreen(Screen *screen);
void FadeToExit();
bool Fading();

#endif