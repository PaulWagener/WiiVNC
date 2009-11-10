#ifndef ViewerH
#define ViewerH

#include "gx.h"
#include <rfb/rfbclient.h>

enum ViewerStatus {VNC_CONNECTING, VNC_NEEDPASSWORD, VNC_CONNECTED, VNC_DISCONNECTED};

/**
 *	Viewer class that takes care of the actual VNC part of this application
 */
class Viewer {
public:
	Viewer(const char* ip, int port=5900, const char* password = NULL);
	~Viewer();
	
	enum ViewerStatus status;
	
	const char* password;
//private:
	//Last created instance of this class
	//There can only be 1 viewer at a time due to the static callbacks I need to feed to rfbClient
	static Viewer *instance;

//private:
	class ScreenPart : public GX_Texture {
		public:
			int offset_x, offset_y;
			ScreenPart(int offset_x, int offset_y, int width, int height);
	};
	
	struct Point {
		int x, y;
	};
	struct Point Screen2VNC(int x, int y);
	struct Point VNC2Screen(int x, int y);

public:
	//A list of textures that make up the entire screen
	//Note that we can't use a single texture for this as the Wii is unable to handle textures
	//larger than (roughly) 1000x1000. Most screen resolutions are bigger than that so we have to use multiple textures
	ScreenPart **screenparts;
	int num_screenparts;
	
	int height, width;
	
	float zoom;
	int zoom_target;
	float zoomto_x, zoomto_y;
	float screen_x, screen_y;

	//The VNC connection to the server
	//Thanks libvncclient© for making my life this easy! 
	rfbClient *client;
	
	lwp_t backgroundthread_handle;
	lwp_t inputsendthread_handle;

	void InitializeScreen(int width, int height);
	void SendKeyDown(int key);
	void SendKeyUp(int key);
	void SendCursorPosition(int x, int y);
	void SendLeftClick(bool down);
	void SendRightClick(bool down);
	void SendMiddleClick(bool down);
	void SendScrollDown();
	void SendScrollUp();
	void Draw();
	void Update();
	void ZoomIn();
	void ZoomOut();
	
	int cursor_x, cursor_y, cursor_state;
public:
	static void* BackgroundThread(void*);
	static void* InputSendThread(void*);
	static char* ReadPassword(rfbClient* client);
	static void ScreenUpdateCallback(rfbClient* client, int x, int y, int w, int h);

};

#endif
