#include "mainscreen.h"
#include <network.h>
#include <errno.h>
#include "freetype.h"
#include "textbox.h"

#include "common.h"
#include "button.h"
#include "gfx/throbber.h"

#define BUTTON_WIDTH 200
#define BUTTON_HEIGHT 40

//Static variables
network_status MainScreen::networkStatus = NO_NETWORK;
lwp_t MainScreen::networkThread;


/**
 * Network initialization
 */
void* MainScreen::InitializeNetwork(void*)
{
	s32 ip;
	
	networkStatus = NETWORK_CONNECTING;
    while ((ip = net_init()) == -EAGAIN) {
		usleep(100 * 1000); //100ms
	}
	
    if(ip < 0) {
		networkStatus = NO_NETWORK;
		return 0;
	}
	
	char myIP[16];
    if (if_config(myIP, NULL, NULL, true) < 0) {
		networkStatus = NO_NETWORK;
		return 0;
	}
	
	networkStatus = NETWORK_CONNECTED;
	return 0;
}


GX_Texture *titleText;
Textbox *addressTextbox;
Textbox *portTextbox;
Keyboard *mainKeyboard;
Button *exitButton;
Button *connectButton;
GX_Texture *throbberTexture;
GX_Texture *initializingText;
GX_Texture *noNetworkText;
int turn = 1;

MainScreen::MainScreen() {
	viewer = NULL;
	
	//Start initializing the network
	if(networkStatus == NO_NETWORK) {
		LWP_CreateThread(&MainScreen::networkThread, MainScreen::InitializeNetwork, NULL, NULL, 0, 80);
	}
	
	//Text
	titleText = GX_Text("WiiVNC", 80, 0xFFFFFF);
	initializingText = GX_Text("Initializing network", 20, 0xFFFFFF);
	noNetworkText = GX_Text("Could not connect to network", 18, 0xFFFFFF);
	
	
	//Textboxes
	addressTextbox = new Textbox((int)(SCREEN_WIDTH*0.05), 150, (int)(SCREEN_WIDTH*0.6), 40);
	portTextbox = new Textbox((int)(SCREEN_WIDTH*0.7), 150, (int)(SCREEN_WIDTH*0.15), 40);
	addressTextbox->SetText("192.168.0.128");
	addressTextbox->hasFocus = true;
	portTextbox->SetText("5900");

	//Keyboard
	mainKeyboard = new Keyboard(50, 220, ALPHA_NUMERIC);
	mainKeyboard->Show();
	mainKeyboard->SetListener(addressTextbox);
	
	//Buttons
	exitButton = new Button(SCREEN_XCENTER - BUTTON_WIDTH - 20, SCREEN_HEIGHT - BUTTON_HEIGHT - 30, BUTTON_WIDTH, BUTTON_HEIGHT, "Exit");
	connectButton = new Button(SCREEN_XCENTER + 20, SCREEN_HEIGHT - BUTTON_HEIGHT - 30, BUTTON_WIDTH, BUTTON_HEIGHT, "Connect");
	
	//Connecting
	throbberTexture = GX_Texture::LoadFromPNG(throbber);	
}

MainScreen::~MainScreen() {
	LWP_JoinThread(networkThread, NULL);
	delete titleText;
	delete addressTextbox;
	delete mainKeyboard;
	delete exitButton;
	delete connectButton;
}
void MainScreen::Draw() {
	titleText->Draw(SCREEN_XCENTER - titleText->width/2, 30);
	addressTextbox->Draw();
	portTextbox->Draw();
	mainKeyboard->Draw();
	exitButton->Draw();
	
	//Draw networkstatus or connect button
	if(networkStatus == NETWORK_CONNECTED)
		connectButton->Draw();
	
	if(networkStatus == NETWORK_CONNECTING) {
		throbberTexture->Draw(SCREEN_XCENTER + 20, SCREEN_HEIGHT - BUTTON_HEIGHT - 30, BUTTON_HEIGHT, BUTTON_HEIGHT, 255, turn);
		initializingText->Draw(SCREEN_XCENTER + BUTTON_HEIGHT + 30, SCREEN_HEIGHT - BUTTON_HEIGHT - 30);
	}
	
	if(networkStatus == NO_NETWORK)
		noNetworkText->Draw(SCREEN_XCENTER + 30, SCREEN_HEIGHT - BUTTON_HEIGHT - 30);
}

void MainScreen::OnHome()
{
	FadeToExit();
}

#define THROBBER_SIZE 60
bool connecting = false;

void MainScreen::Update() {
	addressTextbox->Update();
	portTextbox->Update();
	mainKeyboard->Update();
	exitButton->Update();
	
	if(networkStatus == NETWORK_CONNECTED)
	   connectButton->Update();
	
	turn += 4;
	
	if(exitButton->clicked)
		FadeToExit();
	
	if(connectButton->clicked && !connecting)
	{
		connecting = TRUE;
		viewer = new Viewer(addressTextbox->GetText(), atoi(portTextbox->GetText()));
		FadeToScreen(new ConnectingScreen(viewer));
	}
	/*
	if(viewer != NULL) {
		viewer->Update();
		
		if(viewer->GetStatus() == VNC_DISCONNECTED) {
			delete viewer;
			exit(0);
		}
	} else {
		if(networkStatus == NETWORK_CONNECTED && viewer == NULL) {
			viewer = new Viewer("192.168.0.128", 5900);
			
		}
	}
	FadeToScreen(new ConnectingScreen(NULL));
	*/
}

void MainScreen::OnButton(bool isDown)
{
	addressTextbox->OnButton(isDown);
	portTextbox->OnButton(isDown);
	exitButton->OnButton(isDown);
	connectButton->OnButton(isDown);
	
	if(addressTextbox->hasFocus) mainKeyboard->SetListener(addressTextbox);
	else if(portTextbox->hasFocus) mainKeyboard->SetListener(portTextbox);
	else mainKeyboard->SetListener(NULL);
}

Screen::~Screen()
{
}

/*==============*\
  PasswordScreen
\*==============*/
Keyboard *passwordKeyboard = NULL;
GX_Texture *enterPasswordTexture = NULL;
Textbox *textbox;
Button *enterButton;
Button *pCancelButton;
Viewer *pViewer;

PasswordScreen::PasswordScreen(Viewer *viewer)
{
	pViewer = viewer;
	enterPasswordTexture = GX_Text("Enter Password:", 70, 0xFFFFFF);
	
	textbox = new Textbox(SCREEN_XCENTER - TEXTBOX_WIDTH/2, 130, TEXTBOX_WIDTH, 50);
	textbox->hasFocus = true;
	
	passwordKeyboard = new Keyboard(30, 200, TEXT);
	passwordKeyboard->SetListener(textbox);
	passwordKeyboard->Show();
	
	pCancelButton = new Button(SCREEN_XCENTER - BUTTON_WIDTH - 20, SCREEN_HEIGHT - BUTTON_HEIGHT - 30, BUTTON_WIDTH, BUTTON_HEIGHT, "Cancel");
	enterButton = new Button(SCREEN_XCENTER + 20, SCREEN_HEIGHT - BUTTON_HEIGHT - 30, BUTTON_WIDTH, BUTTON_HEIGHT, "Connect");
}

PasswordScreen::~PasswordScreen()
{
	delete enterPasswordTexture;
	
	delete textbox;
	delete passwordKeyboard;

	delete pCancelButton;
	delete enterButton;
}

void PasswordScreen::Update()
{
	textbox->Update();
	passwordKeyboard->Update();
	
	pCancelButton->Update();
	enterButton->Update();
	
	if(pCancelButton->clicked && !Fading())
		this->OnHome();
	
	if(enterButton->clicked && !Fading())
	{
		pViewer->password = strdup(textbox->GetText());
		FadeToScreen(new ConnectingScreen(pViewer));
	}
}

void PasswordScreen::Draw()
{
	enterPasswordTexture->Draw(SCREEN_XCENTER - enterPasswordTexture->width/2, 20);
	textbox->Draw();
	passwordKeyboard->Draw();
	
	pCancelButton->Draw();
	enterButton->Draw();
	
}

void PasswordScreen::OnButton(bool isDown)
{
	textbox->OnButton(isDown);
	pCancelButton->OnButton(isDown);
	enterButton->OnButton(isDown);
	
}

void PasswordScreen::OnHome()
{
	delete pViewer;
	FadeToScreen(new MainScreen());
}

/*==============*\
 ConnectingScreen
\*==============*/
bool fadingToViewer = false;
GX_Texture *connectingText;
GX_Texture *addressText;
Button *cancelButton;
int counter;

ConnectingScreen::ConnectingScreen(Viewer *viewer) :
	viewer(viewer)
{
	counter = 0;
	connectingText = GX_Text("Connecting to:", 20, 0xFFFFFF);
	addressText = GX_Text(viewer->client->serverHost, 40, 0xFFFFFF);
	throbberTexture = GX_Texture::LoadFromPNG(throbber);
	
	cancelButton = new Button(SCREEN_XCENTER - BUTTON_WIDTH - 20, SCREEN_HEIGHT - BUTTON_HEIGHT - 30, BUTTON_WIDTH, BUTTON_HEIGHT, "Cancel");
}

void ConnectingScreen::Update()
{
	counter++;
	
	if(counter > 100 && !Fading()) {
		if(viewer->GetStatus() == VNC_NEEDPASSWORD)
			FadeToScreen(new PasswordScreen(viewer));
		
		if(viewer->GetStatus() == VNC_CONNECTED) {
			FadeToScreen(viewer);
		}
		
		if(viewer->GetStatus() == VNC_DISCONNECTED) {
			delete viewer;
			FadeToScreen(new MainScreen());
		}
	}
	
	cancelButton->Update();
	turn += 4;
	
	if(cancelButton->clicked)
		FadeToExit();
	
	if(fadingToViewer)
		return;
}



void ConnectingScreen::Draw()
{
	connectingText->Draw(SCREEN_XCENTER - connectingText->width/2, SCREEN_YCENTER - THROBBER_SIZE - connectingText->height - 40);
	addressText->Draw(SCREEN_XCENTER - addressText->width/2, SCREEN_YCENTER - THROBBER_SIZE - connectingText->height);
	throbberTexture->Draw(SCREEN_XCENTER - THROBBER_SIZE/2 ,SCREEN_YCENTER - THROBBER_SIZE/2, THROBBER_SIZE, THROBBER_SIZE, 255, turn);
	cancelButton->Draw();
}

void ConnectingScreen::OnHome()
{
	FadeToScreen(new PasswordScreen(NULL));
}

void ConnectingScreen::OnButton(bool isDown)
{
	cancelButton->OnButton(isDown);
}