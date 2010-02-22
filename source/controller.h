#ifndef ControllerH
#define ControllerH

#include "gx.h"
#include "gfx/cursor_default.h"
#include <wiiuse/wpad.h>

class Keyboard;

/**
 * The ControllerListener is what classes have to subclass
 * in order to recieve callbacks from the Controller class
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
	virtual void OnMouseMove(int x, int y);
	virtual void OnKeyboard();	
	virtual void OnScrollView(int x, int y);	
};

/**
 * The controller retrieves all buttonpresses from the user and
 * handles all cursor events.
 * All input events are then delegated to the current 'ControllerListener', this is
 * is something that can use the controller events in a meaningfull way.
 *
 * If a keyboard is present on the screen than all cursor movement/click events are consumed by the keyboard if
 * the cursor is above the keyboard
 */
class Controller {
public:	
	void Update();
	void Draw();
	void SetListener(ControllerListener *listener);
	void SetKeyboard(Keyboard *keyboard);
	
	//Getters for current cursor position
	static int GetX();
	static int GetY();
	
	static Controller* instance();

private:
	//Current cursor position
	int x, y;
	
	//Previous cursor position
	int previous_x, previous_y;
	
	//Current object that gets controller updates
	ControllerListener *listener;
	
	//Texture of the cursor
	GX_Texture *cursor_texture;
	
	Keyboard *keyboard;
	
	Controller();
	~Controller();
	static Controller *_instance;
};

#include "keyboard.h"


#endif
