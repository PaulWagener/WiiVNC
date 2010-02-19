#ifndef MainscreenH
#define MainscreenH

#include "screen.h"
#include "viewer.h"

enum network_status {NO_NETWORK, NETWORK_CONNECTING, NETWORK_CONNECTED};

#define TEXTBOX_WIDTH 600

void* init_network(void*);

class MainScreen : public Screen {
public:
	MainScreen();
	~MainScreen();
	void Draw();
	void Update();
	void OnHome();
	void OnButton(bool isDown);
	
private:
	Viewer *viewer;
	static lwp_t networkThread;
	static bool networkInitialized;
	static network_status networkStatus;
	static void* InitializeNetwork(void*);
};

class ConnectingScreen : public Screen {
private:
	Keyboard *passwordKeyboard;
	Viewer *viewer;
public:
	ConnectingScreen(Viewer *viewer);
	void Update();
	void Draw();
	void OnHome();
	void OnButton(bool isDown);
	
};

class PasswordScreen : public Screen {
public:
	PasswordScreen(Viewer *viewer);
	~PasswordScreen();
	void Update();
	void Draw();
	void OnButton(bool isDown);
	void OnHome();
};

#endif