#ifndef ControllerH
#define ControllerH

#include "gx.h"
#include "gfx/cursor_default.h"
#include <wiiuse/wpad.h>

/**
 * The ControllerListener is the interface classes have to implement
 * in order to recieve callbacks from the user
 */
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

/**
 * The controller retrieves all buttonpresses from the user and
 * handles all cursor events
 */
class Controller {
public:
	
	Controller();
	~Controller();
	void Update();
	void Draw();
	void SetListener(ControllerListener *listener);
private:
	//Current cursor location
	int x, y;
	
	//Previous cursor location
	int previous_x, previous_y;
	
	//Texture of the cursor
	GX_Texture *texture;
	
	//Current object that gets controller updates
	ControllerListener *listener;	
};



#endif
