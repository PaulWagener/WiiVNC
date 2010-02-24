#include "viewer.h"
#include <rfb/rfbclient.h>

Viewer *Viewer::instance = NULL;

Viewer::ScreenPart::ScreenPart(int offset_x, int offset_y, int width, int height) : 
	GX_Texture(width, height),
	offset_x(offset_x),
	offset_y(offset_y)
{
	return;
}

/**
 * Callback function for when a password is needed
 */
char* Viewer::ReadPassword(rfbClient* client)
{
	if(instance->password == NULL)
	{
		//Wait until a password is given to us
		instance->status = VNC_NEEDPASSWORD;
			
		while(instance->password == NULL) {
			usleep(10);
			
			//User can quit while entering password
			if(instance->GetStatus() == VNC_USERQUITTED)
				return strdup("");
		}
			
		instance->status = VNC_CONNECTING;
	}

	return strdup(instance->password);
}

/**
 * Method that does all the actual networking with the server in the background
 */
void* Viewer::BackgroundThread(void*)
{
	//Start the connection 
	instance->status = VNC_CONNECTING;
	if(!rfbInitConnection(instance->client)) {
		instance->status = VNC_DISCONNECTED;
		return 0;
	}
	instance->status = VNC_CONNECTED;

	//Keep recieving various updates until the connection closes
	while(1) {
		if(!HandleRFBServerMessage(instance->client)) {
			instance->status = VNC_DISCONNECTED;
			return 0;
		}
	}
	
	return 0;
}

Viewer::Viewer(const char* ip, int port, const char* password) :
	password(password),	
	status(VNC_CONNECTING),	
	screenparts(NULL),
	num_screenparts(0),
	width(0),
	height(0),
	zooming_in(false),
	zooming_out(false),
	keyboard(new Keyboard(300, FULL)),
	cursor_state(0)
{
	if (instance != NULL)
		throw "There can only be one viewer instance";
		
	instance = this;

	//Initialize the remote connetion
	client=rfbGetClient(8,3,4);
	client->programName = "WiiVNC";
	client->serverHost = strdup(ip);
	client->serverPort = port;
	client->GetPassword = ReadPassword;
	client->GotFrameBufferUpdate = ScreenUpdateCallback;

	//Start the backgroundthread
	LWP_CreateThread(&backgroundthread_handle, BackgroundThread, client, NULL, 0, 80);
}

Viewer::~Viewer()
{
	status = VNC_USERQUITTED;
	
	delete keyboard;
	
	//Close the connection
	rfbClientCleanup(client);
	
	//Wait on backgroundthread to terminate
	//(should automatically break out of its while loop once it notices the connection is broken)
	LWP_JoinThread(backgroundthread_handle, NULL);
	
	//Delete screenparts
	if(screenparts != NULL)
	{	
		int i;
		for(i = 0; i < num_screenparts; i++)
			delete screenparts[i];
		
		free(screenparts);
	}
	//*/
	instance = NULL;
}

ViewerStatus Viewer::GetStatus()
{
	return status;
}

const char* Viewer::GetHost()
{
	return client->serverHost;
}

//How much border there is between screenedge and vnc in most zoomedout state
#define ZOOMEDOUT_MARGIN 40

void Viewer::InitializeScreen(int width, int height)
{
	this->width = width;
	this->height = height;
	scrollto_x = width / 2;
	scrollto_y = height / 2;
	screen_x = scrollto_x;
	screen_y = scrollto_y;
		
	min_zoom = MIN(log2f(SCREEN_HEIGHT-ZOOMEDOUT_MARGIN / (float)height), log2f((SCREEN_WIDTH-ZOOMEDOUT_MARGIN) / (float)width));
	
	zoom_target = min_zoom;
	zoom = zoom_target;

	//Delete old ScreenParts (if any)
	int i = 0;
	if(screenparts != NULL) {
		for(i = 0; i < num_screenparts; i++)
			delete screenparts[i];

		free(screenparts);
		screenparts = NULL;
	}

	#define hdiv(x,div)    (x/div + (x%div ? 1 : 0)) //Rounding up division
	
	//Create new ScreenParts
	int num_horizontal = hdiv(width, SCREENPART_WIDTH);
	int num_vertical = hdiv(height, SCREENPART_HEIGHT);
	num_screenparts = num_horizontal * num_vertical;
	
	screenparts = (ScreenPart**)malloc(num_screenparts * sizeof(ScreenPart*));

	int x, y;
	i = 0;
	for(x = 0; x < num_horizontal; x++)
		for(y = 0; y < num_vertical; y++)
			screenparts[i++] = new ScreenPart(x*SCREENPART_WIDTH,
											y*SCREENPART_HEIGHT,
											x == num_horizontal-1 ? ((width-1) % SCREENPART_WIDTH)+1 : SCREENPART_WIDTH,
											y == num_vertical-1 ? ((height-1) % SCREENPART_HEIGHT)+1 : SCREENPART_HEIGHT);

	
}

#define PIXEL_SIZE 4
#define TEXEL_SIZE 4 * 4 * 4

/**
 * Callback for every time the a rectangle of the screen is changed
 */ 
void Viewer::ScreenUpdateCallback(rfbClient* client, int x, int y, int w, int h)
{
	//Adapt to changing screen resolution (also used for first initialization)
	if(client->width != instance->width || instance->client->height != instance->height)
		instance->InitializeScreen(client->width, client->height);

	int _x, _y, i;
	
	for(i = 0; i < instance->num_screenparts; i++)
	{
		ScreenPart *screenpart = instance->screenparts[i];

		//Update the relevant portion of this texture
		for(_x = MAX(x, screenpart->offset_x); _x < MIN(x + w, screenpart->offset_x + screenpart->width); _x++)
		{
			for(_y = MAX(y, screenpart->offset_y); _y < MIN(y + h, screenpart->offset_y + screenpart->height); _y++)
			{
				//Find pixelpointer in 4x4RGBA8 texture of ScreenPart
				int texel_x = (_x - screenpart->offset_x) / 4,
					texel_y = (_y - screenpart->offset_y) / 4;
	
				u8* texel_pixel = screenpart->buffer + ((texel_y*( (screenpart->width/4))) + (texel_x))*TEXEL_SIZE;
		
				int local_x = (_x - screenpart->offset_x) % 4,
					local_y = (_y - screenpart->offset_y) % 4;
				texel_pixel += ((local_y*4)+local_x)*(PIXEL_SIZE/2);
				
				//Find pixelpointer in VNC framebuffer
				u8* pixel = client->frameBuffer + ((_y*client->width) + _x) * 4;
	
				//Copy pixel
				texel_pixel[0] = 255;
				texel_pixel[1] = pixel[0];
		
				//Jump to GB pixel
				texel_pixel += TEXEL_SIZE/2;
				texel_pixel[0] = pixel[1];
				texel_pixel[1] = pixel[2];
			}
		}
		screenpart->Flush();
	}
	

}

//Converts pixeldistance on screen to pixeldistance on VNC display
int Viewer::Screen2VNC(int pixels) {
	return pixels / powf(2, zoom);
}	

//Converts pixeldistance on VNC display to pixeldistance on screen
int Viewer::VNC2Screen(int pixels) {
	return pixels * powf(2, zoom);
}

/**
 * Converts a pixel at the local screen to a pixel on the VNC display 
 */
struct Viewer::Point Viewer::Screen2VNCPoint(int x, int y)
{
	struct Point vnc;
	vnc.x = screen_x + Screen2VNC(x - SCREEN_XCENTER);
	vnc.y = screen_y + Screen2VNC(y - SCREEN_YCENTER);
	return vnc;
}	

/**
 * Converts a pixel on the VNC display to a pixel on the local screen
 */
struct Viewer::Point Viewer::VNC2ScreenPoint(int x, int y)
{
	struct Point screen;
	screen.x = SCREEN_XCENTER + VNC2Screen(x - screen_x);
	screen.y = SCREEN_YCENTER + VNC2Screen(y - screen_y);
	return screen;
}

void Viewer::Draw()
{
	//Draws all textures making up the screen
	int i;
	for(i = 0; i < num_screenparts; i++)
	{
		struct Point topleft = VNC2ScreenPoint(screenparts[i]->offset_x, screenparts[i]->offset_y);
		struct Point bottomright = VNC2ScreenPoint(screenparts[i]->offset_x + screenparts[i]->width, screenparts[i]->offset_y + screenparts[i]->height);
		
		screenparts[i]->Draw(topleft.x, topleft.y, bottomright.x - topleft.x, bottomright.y - topleft.y);
	}
	
	//Draw keyboard
	keyboard->Draw();
}


#define ZOOM_SPEED 0.05

bool closetozero(double var)
{
	return var > -0.001 && var < 0.001;
}

void Viewer::Update()
{
	Controller::instance()->SetKeyboard(keyboard);
	keyboard->SetListener(this);
	
	//Zoom animation
	if(zooming_in && zoom < max_zoom) {
		zoom_target += ZOOM_SPEED;
		struct Point cursorpoint = Screen2VNCPoint(Controller::GetX(), Controller::GetY());
		
		//While zooming move towards the cursor
		scrollto_x += (cursorpoint.x - scrollto_x) / 20;
		scrollto_y += (cursorpoint.y - scrollto_y) / 20;
	}
	
	if(zooming_out && zoom_target > min_zoom) {
		zoom_target -= ZOOM_SPEED;
		
	}
	
	//Snap back to center if zoomied all the way back
	if(zooming_out && zoom_target <= min_zoom) {
	   scrollto_x = width/2;
	   scrollto_y = height/2;
	}
	
	//Don't drift off the edges of the display
	int horizontal_margin = Screen2VNC(SCREEN_WIDTH / 3);
	int vertical_margin = Screen2VNC(SCREEN_HEIGHT / 3);
	
	if(scrollto_x < horizontal_margin) scrollto_x = horizontal_margin;
	if(scrollto_x > width - horizontal_margin) scrollto_x = width - horizontal_margin;
	if(scrollto_y < vertical_margin) scrollto_y = vertical_margin;
	if(scrollto_y > height - vertical_margin) scrollto_y = height - vertical_margin;


	//Smoothly move the screen
	double scrolldiff_x = (scrollto_x - screen_x) / 10;
	double scrolldiff_y = (scrollto_y - screen_y) / 10;
	screen_x += scrolldiff_x;
	screen_y += scrolldiff_y;

	double zoom_diff = (zoom_target - zoom) / 10;
    zoom += zoom_diff;
	
	//Make the pointer move if the display moves
	if(!closetozero(scrolldiff_x) || !closetozero(scrolldiff_y) || !closetozero(zoom_diff))
		SendPointer();
	
	keyboard->Update();
	
	if(GetStatus() == VNC_DISCONNECTED && !Fading())
		FadeToExit();
}

void Viewer::SendPointer()
{
	if(status == VNC_CONNECTED) {
		struct Point cursor = Screen2VNCPoint(Controller::GetX(), Controller::GetY());
		SendPointerEvent(client, cursor.x, cursor.y, cursor_state);
	}
}

//Controller event handlers
void Viewer::OnButton(bool isDown)
{
	if(isDown)
		cursor_state |= LEFT_BUTTON;
	else
		cursor_state &= ~LEFT_BUTTON;
	SendPointer();
}

void Viewer::OnMiddleButton(bool isDown)
{
	if(isDown)
		cursor_state |= MIDDLE_BUTTON;
	else
		cursor_state &= ~MIDDLE_BUTTON;
	SendPointer();
}

void Viewer::OnSecondaryButton(bool isDown)
{
	if(isDown)
		cursor_state |= RIGHT_BUTTON;
	else
		cursor_state &= ~RIGHT_BUTTON;
	SendPointer();
}

void Viewer::OnScrollUp()
{
	cursor_state |= SCROLL_UP;
	SendPointer();
	cursor_state &= ~SCROLL_UP;
}

void Viewer::OnScrollDown()
{
	cursor_state |= SCROLL_DOWN;
	SendPointer();
	cursor_state &= ~SCROLL_DOWN;
}

void Viewer::OnZoomIn(bool isDown)
{		
	zooming_in = isDown;
}

void Viewer::OnZoomOut(bool isDown)
{
	zooming_out = isDown;
}

void Viewer::OnHome()
{
	FadeToExit();
}

void Viewer::OnScrollView(int x, int y)
{
	scrollto_x += x;
	scrollto_y -= y;
}

void Viewer::OnMouseMove(int x, int y)
{
	SendPointer();
}

void Viewer::OnKeyboard()
{
	if(keyboard->Visible())
	{
		keyboard->Hide();
	} else {
		keyboard->Show();
	}
}


u16 keycodeToVNC(u16 keycode) {
	switch(keycode) {
		case KS_Home:		return VNC_HOME;
		case KS_End:		return VNC_END;
		case KS_Left:		return VNC_LEFT;
		case KS_Right:		return VNC_RIGHT;
		case KS_Up:			return VNC_UP;
		case KS_Down:		return VNC_DOWN;
		case KS_Prior:		return VNC_PAGE_UP;
		case KS_Next:		return VNC_PAGE_DOWN;
		case KS_Insert:		return VNC_INSERT;
		case KS_Delete:		return VNC_DELETE;
		case KS_f1:			return VNC_F1;
		case KS_f2:			return VNC_F1;
		case KS_f3:			return VNC_F3;
		case KS_f4:			return VNC_F4;
		case KS_f5:			return VNC_F5;
		case KS_f6:			return VNC_F6;
		case KS_f7:			return VNC_F7;
		case KS_f8:			return VNC_F8;
		case KS_f9:			return VNC_F9;
		case KS_f10:		return VNC_F10;
		case KS_f11:		return VNC_F11;
		case KS_f12:		return VNC_F12;
		case KS_BackSpace:	return VNC_BACKSPACE;
		case KS_Tab:		return VNC_TAB;
		case KS_Return:		return VNC_ENTER;
		case KS_Escape:		return VNC_ESCAPE;
		case KS_Shift_L:	return VNC_SHIFTLEFT;
		case KS_Shift_R:	return VNC_SHIFTRIGHT;
		case KS_Control_L:	return VNC_CTRLLEFT;
		case KS_Control_R:	return VNC_CTRLRIGHT;
		case KS_Meta_L:		return VNC_METALEFT;
		case KS_Meta_R:		return VNC_METARIGHT;
		case KS_Alt_L:		return VNC_ALTLEFT;
		case KS_Alt_R:		return VNC_ALTRIGHT;
	}
	return keycode;
}

void Viewer::OnKey(int keycode, bool isDown) {
	SendKeyEvent(client, keycodeToVNC(keycode), isDown);
}