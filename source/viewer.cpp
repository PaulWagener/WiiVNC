/*
 *  viewer.cpp
 *  WiiVNC
 *
 *  Created by Paul Wagener on 29-10-09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "viewer.h"

#define PIXEL_SIZE 4
#define TEXEL_SIZE 4 * 4 * 4

Viewer *Viewer::_instance = NULL;

char* Viewer::ReadPassword(rfbClient* client) {
	return strdup("wac");
	if(_instance->password == NULL)
	{
		_instance->status = NEED_PASSWORD;
			
		while(_instance->password == NULL)
			usleep(10);
			
		_instance->status = CONNECTING;
	}


	return strdup(_instance->password);
}


static lwp_t updatethread;
void* Viewer::BackgroundThread(void* nothing)
{
	printf("   init\n");
	if(!rfbInitConnection(_instance->client)) {
		_instance->status = DISCONNECTED;
		printf("  helaas\n");
		return 0;
	}
	_instance->status = CONNECTED;
	printf("   inited, doorgaan met Handle\n");	
	while(1) {
		if(!HandleRFBServerMessage(_instance->client)) {
			exit(0);
			_instance->status = DISCONNECTED;
			return 0;
		}
			
	}
	return 0;
}

Viewer::Viewer(const char* ip, const char* newPassword)
{
	if (_instance != NULL)
		throw "There can only be one viewer instance";
		
	_instance = this;
	
	status = CONNECTING;
	
	password = newPassword;
	
	client=rfbGetClient(8,3,4);
	client->programName = "WiiVNC";
	client->serverHost = strdup("192.168.0.130");//strdup(ip);
	client->serverPort = 5900;
	client->GetPassword = ReadPassword;
	client->GotFrameBufferUpdate = UpdateCallback;

	texture	= new GX_Texture*[4];
	texture[0] = new GX_Texture(1000,800);
	memset(texture[0]->buffer, 0, 1000*800*4);
	texture[0]->Flush();
	

	//BackgroundThread((void*)4);
	LWP_CreateThread(&updatethread, BackgroundThread, client, NULL, 0, 80);
}

Viewer::~Viewer()
{
	rfbClientCleanup(client);
	_instance = NULL;
}

void Viewer::UpdateCallback(rfbClient* client, int x, int y, int w, int h) {
	GX_Texture *texture = _instance->texture[0];
	int i_x, i_y;
	for(i_x = x; i_x < MIN(x + w, texture->width); i_x++)
	{
		for(i_y = y; i_y < MIN(y + h, texture->height); i_y++)
		{
			int texel_x = i_x / 4,
				texel_y = i_y / 4;

			//Find pointer to texel
			u8* texel_pixel = texture->buffer + ((texel_y*( (texture->width/4))) + (texel_x))*TEXEL_SIZE;
	
			int local_x = i_x % 4,
				local_y = i_y % 4;

			u8* pixel = client->frameBuffer + ((i_y*client->width) + i_x) * 4;

			//Jump to AR pixel
			texel_pixel += ((local_y*4)+local_x)*(PIXEL_SIZE/2);
			texel_pixel[0] = 255;
			texel_pixel[1] = pixel[0];
	
			//Jump to GB pixel
			texel_pixel += TEXEL_SIZE/2;
			texel_pixel[0] = pixel[1];
			texel_pixel[1] = pixel[2];
		}
	}
	
	DCFlushRange(texture->buffer, texture->width*texture->height*4);
}

void Viewer::Draw()
{
	texture[0]->Draw(0, 0, 640, 480);
}
