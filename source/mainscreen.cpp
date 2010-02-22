#include "mainscreen.h"
#include <network.h>
#include <errno.h>
#include <fat.h>
#include <sys/dir.h>

#include "language.h"
#define BUTTON_WIDTH 200
#define BUTTON_HEIGHT 40

//Static variables
network_status MainScreen::networkStatus = NO_NETWORK;
lwp_t MainScreen::networkThread;

bool fatInited = false;
FILE* openFile(const char* mode)
{
	if(!fatInited && fatInitDefault())
		fatInited = true;
	   
	if(fatInited) {
		DIR_ITER *homeDir = diropen("sd:/apps/wiivnc");
	   
		const char* filename;
		if(homeDir != NULL)
		{
			filename = "sd:/apps/wiivnc/ip.txt";
			dirclose(homeDir);
		} else {
			filename = "sd:/wiivnc.txt";
		}
		
		return fopen(filename, mode);
	}
	return NULL;
}

MainScreen::MainScreen() :
	titleText(GX_Text("WiiVNC", 80, 0xFFFFFF)),
	initializingText(GX_Text(TEXT_InitializingNetwork, 20, 0xFFFFFF)),
	noNetworkText(GX_Text(TEXT_CouldNotConnect, 18, 0xFFFFFF)),
	throbberTexture(GX_Texture::LoadFromPNG(throbber)),

	addressTextbox(new Textbox((int)(SCREEN_WIDTH*0.05), 150, (int)(SCREEN_WIDTH*0.6), 40)),
	portTextbox(new Textbox((int)(SCREEN_WIDTH*0.7), 150, (int)(SCREEN_WIDTH*0.15), 40)),
	mainKeyboard(new Keyboard(50, 220, ALPHA_NUMERIC)),

	exitButton(new Button(SCREEN_XCENTER - BUTTON_WIDTH - 20, SCREEN_HEIGHT - BUTTON_HEIGHT - 30, BUTTON_WIDTH, BUTTON_HEIGHT, TEXT_Exit)),
	connectButton(new Button(SCREEN_XCENTER + 20, SCREEN_HEIGHT - BUTTON_HEIGHT - 30, BUTTON_WIDTH, BUTTON_HEIGHT, TEXT_Connect)),
	throbberRotation(0)
{
	//Start initializing the network
	if(networkStatus == NO_NETWORK)
		LWP_CreateThread(&MainScreen::networkThread, MainScreen::InitializeNetwork, NULL, NULL, 0, 80);

	//TODO: Remember the values entered last time (from file on SD)
	addressTextbox->SetText("192.168.0.1");
	addressTextbox->hasFocus = true;
	portTextbox->SetText("5900");
	
	FILE *file = openFile("r");
	if(file != NULL) {
		char address[1024];
		char port[64];
		if(fscanf(file, "%[^\n]\n%[^\n]", &address[0], &port[0]) == 2) {
			addressTextbox->SetText(address);
			portTextbox->SetText(port);
		}
		fclose(file);
	}

	mainKeyboard->Show();
	mainKeyboard->SetListener(addressTextbox);
}

MainScreen::~MainScreen() {
	LWP_JoinThread(networkThread, NULL);
	delete titleText;
	delete initializingText;
	delete noNetworkText;
	delete throbberTexture;
	delete addressTextbox;
	delete portTextbox;
	delete mainKeyboard;
	delete exitButton;
	delete connectButton;
}

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
		throbberTexture->Draw(SCREEN_XCENTER + 20, SCREEN_HEIGHT - BUTTON_HEIGHT - 30, BUTTON_HEIGHT, BUTTON_HEIGHT, 255, throbberRotation);
		initializingText->Draw(SCREEN_XCENTER + BUTTON_HEIGHT + 30, SCREEN_HEIGHT - BUTTON_HEIGHT - 30);
	}
	
	if(networkStatus == NO_NETWORK)
		noNetworkText->Draw(SCREEN_XCENTER + 30, SCREEN_HEIGHT - BUTTON_HEIGHT - 30);
}

void MainScreen::OnHome()
{
	FadeToExit();
}


void MainScreen::Update() {
	addressTextbox->Update();
	portTextbox->Update();
	mainKeyboard->Update();
	exitButton->Update();
	
	if(networkStatus == NETWORK_CONNECTED)
	   connectButton->Update();
	
	throbberRotation += 4;
	
	if(exitButton->clicked)
		FadeToExit();

	//Make the viewer and go the the connecting screen
	if(connectButton->clicked && !Fading())
	{
		//Save for the next time!
		FILE *file = openFile("w");
		if(file != NULL)
		{
			fprintf(file, "%s\n%s", addressTextbox->GetText(), portTextbox->GetText());
			fclose(file);
		}
		
		Viewer *viewer = new Viewer(addressTextbox->GetText(), atoi(portTextbox->GetText()));//, strdup("wac"));
		FadeToScreen(new ConnectingScreen(viewer));
	}
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
 ConnectingScreen
\*==============*/
ConnectingScreen::ConnectingScreen(Viewer *viewer) :
	connectingText(GX_Text(TEXT_ConnectingTo, 20, 0xFFFFFF)),
	addressText(GX_Text(viewer->GetHost(), 40, 0xFFFFFF)),
	throbberTexture(GX_Texture::LoadFromPNG(throbber)),
	cancelButton(new Button(SCREEN_XCENTER - BUTTON_WIDTH - 20, SCREEN_HEIGHT - BUTTON_HEIGHT - 30, BUTTON_WIDTH, BUTTON_HEIGHT, TEXT_Exit)),
	connectionFailedText(GX_Text(TEXT_ConnectionFailed, 30, 0xFFFFFF)),
	failedCounter(0),
	viewer(viewer),
	throbberRotation(0)
{
	return;
}

ConnectingScreen::~ConnectingScreen()
{
	delete connectingText;
	delete addressText;
	delete throbberTexture;
	delete cancelButton;
	delete connectionFailedText;
}

void ConnectingScreen::Update()
{
	//Find out if we can advance to a next screen
	if(!Fading()) {
		if(viewer->GetStatus() == VNC_NEEDPASSWORD)
			FadeToScreen(new PasswordScreen(viewer));
		
		if(viewer->GetStatus() == VNC_CONNECTED)
			FadeToScreen(viewer);
		
		if(viewer->GetStatus() == VNC_DISCONNECTED && failedCounter == 0)
			failedCounter = 1;
	}
	
	if(failedCounter > 0) {
		failedCounter++;
		
		if(failedCounter > 100 && !Fading()) {
			delete viewer;
			FadeToScreen(new MainScreen());
		}
	}
	
	cancelButton->Update();
	throbberRotation += 4;
	
	if(cancelButton->clicked && !Fading())
		FadeToRestart();
}

void ConnectingScreen::Draw()
{
	if(failedCounter > 0) {
		connectionFailedText->Draw(SCREEN_XCENTER - connectionFailedText->width/2, 200);
	} else {
		connectingText->Draw(SCREEN_XCENTER - connectingText->width/2, SCREEN_YCENTER - THROBBER_SIZE - connectingText->height - 40);
		addressText->Draw(SCREEN_XCENTER - addressText->width/2, SCREEN_YCENTER - THROBBER_SIZE - connectingText->height);
		throbberTexture->Draw(SCREEN_XCENTER - THROBBER_SIZE/2 ,SCREEN_YCENTER - THROBBER_SIZE/2, THROBBER_SIZE, THROBBER_SIZE, 255, throbberRotation);
		cancelButton->Draw();
	}	
}

void ConnectingScreen::OnHome()
{
	FadeToRestart();
}

void ConnectingScreen::OnButton(bool isDown)
{
	cancelButton->OnButton(isDown);
}


/*==============*\
  PasswordScreen
\*==============*/
PasswordScreen::PasswordScreen(Viewer *viewer) :
	enterPasswordTexture(GX_Text(TEXT_EnterPassword, 70, 0xFFFFFF)),
	textbox(new Textbox(SCREEN_XCENTER - TEXTBOX_WIDTH/2, 130, TEXTBOX_WIDTH, 50)),
	keyboard(new Keyboard(30, 200, TEXT)),
	cancelButton(new Button(SCREEN_XCENTER - BUTTON_WIDTH - 20, SCREEN_HEIGHT - BUTTON_HEIGHT - 30, BUTTON_WIDTH, BUTTON_HEIGHT, TEXT_Cancel)),
	enterButton(new Button(SCREEN_XCENTER + 20, SCREEN_HEIGHT - BUTTON_HEIGHT - 30, BUTTON_WIDTH, BUTTON_HEIGHT, TEXT_Enter)),
	viewer(viewer)
{
	textbox->hasFocus = true;
	textbox->SetText("");

	keyboard->SetListener(textbox);
	keyboard->Show();
	
}

PasswordScreen::~PasswordScreen()
{
	delete enterPasswordTexture;
	
	delete textbox;
	delete keyboard;

	delete cancelButton;
	delete enterButton;
}

void PasswordScreen::Update()
{
	textbox->Update();
	keyboard->Update();
	
	cancelButton->Update();
	enterButton->Update();
	
	if(cancelButton->clicked && !Fading())
		this->OnHome();
	
	if(enterButton->clicked && !Fading())
	{
		viewer->password = strdup(textbox->GetText());
		FadeToScreen(new ConnectingScreen(viewer));
	}
}

void PasswordScreen::Draw()
{
	enterPasswordTexture->Draw(SCREEN_XCENTER - enterPasswordTexture->width/2, 20);
	textbox->Draw();
	keyboard->Draw();
	
	cancelButton->Draw();
	enterButton->Draw();
	
}

void PasswordScreen::OnButton(bool isDown)
{
	textbox->OnButton(isDown);
	cancelButton->OnButton(isDown);
	enterButton->OnButton(isDown);
}

void PasswordScreen::OnHome()
{
	delete viewer;
	FadeToScreen(new MainScreen());
}