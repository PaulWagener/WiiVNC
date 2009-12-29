#ifndef CursorH
#define CursorH

#include "gx.h"


class ControllerListener {
public:
	virtual void OnButton(bool isDown);
	virtual void OnSecondaryButton(bool isDown);
	virtual void OnMiddleButton(bool isDown);
	virtual void OnScrollUp();
	virtual void OnScrollDown();
	virtual void OnZoomIn(bool isDown);
	virtual void OnZoomOut(bool isDown);
	virtual void OnHome();
	virtual void OnCursorMove(int x, int y);
	virtual void OnKeyboard();	
	virtual void OnScrollView(int x, int y);	
};

class Controller {
public:
	int x, y;
	Controller();
	~Controller();
	void Update();
	void Draw();
public:
	ControllerListener *listener;
	GX_Texture *texture;
};



#endif
