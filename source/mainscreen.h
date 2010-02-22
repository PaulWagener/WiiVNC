#ifndef MainscreenH
#define MainscreenH

#include "screen.h"
#include "viewer.h"
#include "freetype.h"
#include "textbox.h"

#include "common.h"
#include "button.h"
#include "gfx/throbber.h"
enum network_status {NO_NETWORK, NETWORK_CONNECTING, NETWORK_CONNECTED};

#define TEXTBOX_WIDTH 600
#define THROBBER_SIZE 60

/**
 * The mainscreen is where the user fills in the IP address and the port of the VNC server
 * This screen also makes sure that there is a network connection
 */
class MainScreen : public Screen {
private:
	GX_Texture *titleText;
	GX_Texture *initializingText;
	GX_Texture *noNetworkText;
	GX_Texture *throbberTexture;
	
	Textbox *addressTextbox;
	Textbox *portTextbox;
	Keyboard *mainKeyboard;
	Button *exitButton;
	Button *connectButton;

	int throbberRotation;
	
	static lwp_t networkThread;
	static bool networkInitialized;
	static network_status networkStatus;
	static void* InitializeNetwork(void*);
	
public:
	MainScreen();
	~MainScreen();
	void Draw();
	void Update();
	void OnHome();
	void OnButton(bool isDown);
};

/**
 * ConnectingScreen shows a rotating throbber to indicate to the user that WiiVNC really is
 * trying to connect.
 * It then either goes to passwordScreen if a password is needed, to the viewer itself if the connection is ok
 * or back to the mainscreen if the connection didn't work out
 */
class ConnectingScreen : public Screen {
private:
	GX_Texture *connectingText;
	GX_Texture *addressText;
	GX_Texture *throbberTexture;
	Button *cancelButton;
	GX_Texture *connectionFailedText;

	int failedCounter;
	
	Viewer *viewer;
	int throbberRotation;

public:
	ConnectingScreen(Viewer *viewer);
	~ConnectingScreen();
	void Update();
	void Draw();
	void OnHome();
	void OnButton(bool isDown);
	
};

/**
 * Passwordscreen is where the user fills in the password for the VNC server.
 * This is only shown if it turns out that the server is password protected
 */
class PasswordScreen : public Screen {
private:
	GX_Texture *enterPasswordTexture;
	Textbox *textbox;
	Keyboard *keyboard;
	Button *cancelButton;
	Button *enterButton;
	Viewer *viewer;
public:
	PasswordScreen(Viewer *viewer);
	~PasswordScreen();
	void Update();
	void Draw();
	void OnButton(bool isDown);
	void OnHome();
};

#endif