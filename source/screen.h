#ifndef ScreenH
#define ScreenH

#include "controller.h"

class Screen : public ControllerListener {
public:

	virtual ~Screen();
	virtual void Update()=0;
	virtual void Draw()=0;
};

void FadeToScreen(Screen *screen);
void FadeToExit();
void FadeToRestart();
bool Fading();

#endif