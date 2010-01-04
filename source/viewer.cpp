#include "viewer.h"
#include <rfb/rfbclient.h>

Viewer *Viewer::instance = NULL;

Viewer::ScreenPart::ScreenPart(int theOffset_x, int theOffset_y, int theWidth, int theHeight) : GX_Texture(theWidth, theHeight)
{
	offset_x = theOffset_x;
	offset_y = theOffset_y;
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
			
		while(instance->password == NULL)
			usleep(10);
			
		instance->status = VNC_CONNECTING;
	}


	return strdup(instance->password);
}

/**
 * Method that does all the actual networking with the server in the background
 */
void* Viewer::BackgroundThread(void* nothing)
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

Viewer::Viewer(const char* ip, int port, const char* newPassword)
{
	if (instance != NULL)
		throw "There can only be one viewer instance";
		
	instance = this;
	this->status = VNC_CONNECTING;
	password = newPassword;
	width = 0;
	height = 0;
	zooming_in = false;
	zooming_out = false;
	screenparts = NULL;
	keyboard = NULL;
	num_screenparts = 0;
	
	cursor_x = SCREEN_XCENTER;
	cursor_y = SCREEN_YCENTER;
	cursor_state = 0;
	
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
	
	instance = NULL;
}

ViewerStatus Viewer::GetStatus()
{
	return status;
}


void Viewer::InitializeScreen(int width, int height)
{
	this->width = width;
	this->height = height;
	scrollto_x = width / 2;
	scrollto_y = height / 2;
	screen_x = scrollto_x;
	screen_y = scrollto_y;
		
	min_zoom = MIN(log2f(SCREEN_HEIGHT / (float)height), log2f(SCREEN_WIDTH / (float)width));
	
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
	if(keyboard)
		keyboard->Draw();
}


#define AUTOSCROLL_MARGIN 15
#define ZOOM_SPEED 0.05

bool closetozero(double var)
{
	return var > -0.001 && var < 0.001;
}

void Viewer::Update()
{
	//Autoscrolling if the cursor is at the screen edge
	if(cursor_x <  AUTOSCROLL_MARGIN) scrollto_x--;
	if(cursor_x > SCREEN_WIDTH - AUTOSCROLL_MARGIN) scrollto_x++;
	if(cursor_y <  AUTOSCROLL_MARGIN) scrollto_y--;
	if(cursor_y > SCREEN_HEIGHT - AUTOSCROLL_MARGIN) scrollto_y++;
	
	//Zoom animation
	if(zooming_in && zoom < max_zoom) {
		zoom_target += ZOOM_SPEED;
		struct Point cursorpoint = Screen2VNCPoint(cursor_x, cursor_y);
		
		//While zooming move towards the cursor
		scrollto_x += (cursorpoint.x - scrollto_x) / 20;
		scrollto_y += (cursorpoint.y - scrollto_y) / 20;
	}
	
	if(zooming_out && zoom > min_zoom) {
		zoom_target -= ZOOM_SPEED;
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
}

void Viewer::SendPointer()
{
	if(status == VNC_CONNECTED) {
		struct Point cursor = Screen2VNCPoint(cursor_x, cursor_y);
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
	status = VNC_DISCONNECTED;
}

void Viewer::OnScrollView(int x, int y)
{
	scrollto_x += x;
	scrollto_y -= y;
}

void Viewer::OnCursorMove(int x, int y)
{
	cursor_x = x;
	cursor_y = y;
	SendPointer();
}

void Viewer::OnKeyboard()
{
	if(keyboard == NULL) {
		keyboard = new Keyboard();
	} else {
		delete keyboard;
		keyboard = NULL;
	}
}