#ifndef ViewerH
#define ViewerH

#include "gx.h"
#include <rfb/rfbclient.h>

enum ViewerStatus {CONNECTING, NEED_PASSWORD, CONNECTED, DISCONNECTED};

/**
 *	Viewer class that takes care of the actual VNC part of this application
 */
class Viewer {
public:
	Viewer(const char* ip, const char* password = NULL);
	~Viewer();
	void Draw();
	
	enum ViewerStatus status;
	const char* password;
private:
	//Last created instance of this class
	//There can only be 1 viewer at a time due to the static callbacks I need to feed to rfbClient
public:
	static Viewer *_instance;
	
	//A list of all separate textures 
	GX_Texture **texture;

	//The VNC connection to the server
	//Thanks libvncclient© for making my life this easy! 
	rfbClient *client;
	

public:
	static char* ReadPassword(rfbClient* client);
	static void* BackgroundThread(void*);
	static void UpdateCallback(rfbClient* client, int x, int y, int w, int h);

};

#endif
