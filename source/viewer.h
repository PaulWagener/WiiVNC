#ifndef ViewerH
#define ViewerH

#include "gx.h"
#include "keyboard.h"
#include "controller.h"
#include <rfb/rfbclient.h>
#include <math.h>
#include "screen.h"

enum ViewerStatus {
	VNC_CONNECTING,
	VNC_NEEDPASSWORD,
	VNC_CONNECTED,
	VNC_DISCONNECTED,
	VNC_USERQUITTED
};


#define LEFT_BUTTON		0x01
#define MIDDLE_BUTTON	0x02
#define RIGHT_BUTTON	0x04
#define SCROLL_UP		0x08
#define SCROLL_DOWN		0x10


/**
 *	Viewer class that takes care of the actual VNC part of this application
 */
class Viewer : public KeyboardListener, public Screen {
public:
	Viewer(const char* ip, int port=5900, const char* password = NULL);
	~Viewer();
	
	void Draw();
	void Update();
	ViewerStatus GetStatus();
	
	void OnButton(bool isDown);
	void OnMiddleButton(bool isDown);
	void OnSecondaryButton(bool isDown);
	void OnScrollUp();
	void OnScrollDown();
	void OnZoomIn(bool isDown);
	void OnZoomOut(bool isDown);
	void OnHome();
	void OnMouseMove(int x, int y);
	void OnKeyboard();	
	void OnScrollView(int x, int y);
	
	void OnKey(int keycode, bool isDown);
		
	const char* password;
private:
	enum ViewerStatus status;
	//Last created instance of this class
	//There can only be one viewer at a time due to the static callbacks I need to feed to rfbClient
	//(also: why on earth would you need more then one viewer?)
	static Viewer *instance;

	struct Point {
		int x, y;
	};

	//A list of textures that make up the entire screen
	//Note that we can't use a single texture for this as the Wii is unable to handle textures
	//larger than (roughly) 1000x1000. Most screen resolutions are bigger than that so we have to use multiple textures
	class ScreenPart : public GX_Texture {
	public:
		int offset_x, offset_y;
		ScreenPart(int offset_x, int offset_y, int width, int height);
	};
	ScreenPart **screenparts;
	int num_screenparts;
	
	#define SCREENPART_WIDTH 640 
	#define SCREENPART_HEIGHT 480
	
	int height, width;
	
	//Zoom status
	bool zooming_in;
	bool zooming_out;
	
	float zoom;
	float zoom_target;
	
	#define max_zoom 2
	float min_zoom;
	
	float scrollto_x, scrollto_y;
	float screen_x, screen_y;
	
	Keyboard *keyboard;	
	
	//The posiion of the cursor & the mouse buttons that are pressed
	int cursor_x, cursor_y,
		cursor_state;

	//The VNC connection to the server
	//Thanks libvncclient© for making my life this easy! 
public:
	rfbClient *client;
	
	lwp_t backgroundthread_handle;

	void InitializeScreen(int width, int height);
	void SendPointer();
	static void* BackgroundThread(void*);
	static void* InputSendThread(void*);
	static char* ReadPassword(rfbClient* client);
	static void ScreenUpdateCallback(rfbClient* client, int x, int y, int w, int h);
	
	//Coordinate converter helper functions
	int Screen2VNC(int pixels);
	int VNC2Screen(int pixels);
	struct Point Screen2VNCPoint(int x, int y);
	struct Point VNC2ScreenPoint(int x, int y);
};

#endif
