#include "viewer.h"
#include <rfb/rfbclient.h>
#define PIXEL_SIZE 4
#define TEXEL_SIZE 4 * 4 * 4

Viewer::ScreenPart::ScreenPart(int theOffset_x, int theOffset_y, int theWidth, int theHeight) : GX_Texture(theWidth, theHeight)
{
	offset_x = theOffset_x;
	offset_y = theOffset_y;
}




Viewer *Viewer::instance = NULL;

/**
 * Callback function for when
 */
char* Viewer::ReadPassword(rfbClient* client)
{
	if(instance->password == NULL)
	{
		instance->status = VNC_NEEDPASSWORD;
			
		while(instance->password == NULL)
			usleep(10);
			
		instance->status = VNC_CONNECTING;
	}


	return strdup(instance->password);
}


void* Viewer::BackgroundThread(void* nothing)
{
	//Start the connection 
	instance->status = VNC_CONNECTING;
	if(!rfbInitConnection(instance->client)) {
		instance->status = VNC_DISCONNECTED;
		return 0;
	}
	instance->status = VNC_CONNECTED;

	//Keep recieving updates until the connection closes
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
	password = newPassword;
	width = 0;
	height = 0;
	screenparts = NULL;
	num_screenparts = 0;
	
	cursor_x = 0;
	cursor_y = 0;
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

#define SCREENPART_WIDTH 640
#define SCREENPART_HEIGHT 480
#define hdiv(x,div)    (x/div + (x%div ? 1 : 0)) //Rounding up division
void Viewer::InitializeScreen(int theWidth, int theHeight)
{
	width = theWidth;
	height = theHeight;
	screen_x = width / 2;
	screen_y = height / 2;
	zoomto_x = screen_x;
	zoomto_y = screen_y;
	zoom_target = 3;
	zoom = zoom_target;

	//Delete old ScreenParts (if any)
	int i = 0;
	if(screenparts != NULL) {
		for(i = 0; i < num_screenparts; i++)
			delete screenparts[i];

		free(screenparts);
		screenparts = NULL;
	}
	
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

#define SCREEN_XCENTER 320
#define SCREEN_YCENTER 240
#include <math.h>

/**
 * Converts a point at the local screen to a pixel on the VNC display 
 */
struct Viewer::Point Viewer::Screen2VNC(int x, int y)
{
	struct Point vnc;
	vnc.x = screen_x + (x - SCREEN_XCENTER / powf(2, zoom));
	vnc.y = screen_y + (y - SCREEN_YCENTER / powf(2, zoom));
	return vnc;
}	

/**
 * Converts a point on the VNC display to a pixel on the local screen
 */
struct Viewer::Point Viewer::VNC2Screen(int x, int y)
{
	struct Point screen;
	screen.x = SCREEN_XCENTER + ((x - screen_x) * powf(2, zoom));
	screen.y = SCREEN_YCENTER + ((y - screen_y) * powf(2, zoom));
	return screen;
}	

void Viewer::Draw()
{
	int i;
	for(i = 0; i < num_screenparts; i++)
	{
		struct Point topleft = VNC2Screen(screenparts[i]->offset_x, screenparts[i]->offset_y);
		struct Point bottomright = VNC2Screen(screenparts[i]->offset_x + screenparts[i]->width, screenparts[i]->offset_y + screenparts[i]->height);
		
		screenparts[i]->Draw(topleft.x, topleft.y, bottomright.x - topleft.x, bottomright.y - topleft.y);
	}
}

void Viewer::Update()
{
	//Zoom animation
	//screen_x += (zoomto_x - screen_x) / 10;
	//screen_y += (zoomto_y - screen_y) / 10;
    zoom += (zoom_target - zoom) / 10;
}

void Viewer::SendKeyDown(int key)
{
	SendKeyEvent(client, key, TRUE);
}

void Viewer::SendKeyUp(int key)
{
	SendKeyEvent(client, key, FALSE);
}

void Viewer::SendCursorPosition(int x, int y)
{
	struct Point point = Screen2VNC(x, y);
	cursor_x = point.x;
	cursor_y = point.y;
	SendPointerEvent(client, x,y, cursor_state);
}

void Viewer::SendLeftClick(bool down)
{
	cursor_state = down ? 1 : 0;
	SendPointerEvent(client, cursor_x, cursor_y, cursor_state);
}

void Viewer::SendRightClick(bool down)
{
	cursor_state = down ? 2 : 0;
	SendPointerEvent(client, cursor_x, cursor_y, cursor_state);
}

void Viewer::SendMiddleClick(bool down)
{
	cursor_state = down ? 3 : 0;
	SendPointerEvent(client, cursor_x, cursor_y, cursor_state);
}

void Viewer::SendScrollDown()
{
	SendPointerEvent(client, cursor_x, cursor_y, 4);
}

void Viewer::SendScrollUp()
{
	SendPointerEvent(client, cursor_x, cursor_y, 5);
}

void Viewer::ZoomIn()
{
	zoom_target++;
}

void Viewer::ZoomOut()
{
	zoom_target--;
}
