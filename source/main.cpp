#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <ogcsys.h>

#include <rfb/rfbclient.h>
#include <network.h>
#include <errno.h>
#include "gx.h"
#include "viewer.h"
#include "freetype.h"
#include "controller.h"
enum network_status {NO_NETWORK, NETWORK_CONNECTING, NETWORK_CONNECTED};
network_status network_status = NO_NETWORK;

s32 ip;
void* init_network(void*)
{
	printf("\n\nhello\n");
	network_status = NETWORK_CONNECTING;
	printf("    sdf\n");
    while ((ip = net_init()) == -EAGAIN) {
		usleep(100 * 1000); //100ms
		printf(".");
	}
	
    if(ip < 0) {
		network_status = NO_NETWORK;
		return 0;
	}
        
	char myIP[16];
    if (if_config(myIP, NULL, NULL, true) < 0) {
		network_status = NO_NETWORK;
		return 0;
	}
	
	network_status = NETWORK_CONNECTED;
	printf("Connected\n");
	return 0;
}

static lwp_t initnetworkthread;

void* xfbx[2];
int main(int argc, char **argv) {
	GX_Initialize();
	GX_InitFreetype();
	Viewer *viewer = NULL;

	Controller cursor = Controller();
	Keyboard keyboard = Keyboard();
	LWP_CreateThread(&initnetworkthread, init_network, NULL, NULL, 0, 80);
	while(1) {
		
		cursor.Update();
		keyboard.Update();
		keyboard.Draw();

		if(viewer != NULL)
		{
			viewer->Update();
			viewer->Draw();
			//*
			switch(viewer->status)
			{
				case VNC_CONNECTING:
					GX_Text("VNC verbinden...").Draw(100, 200);
					break;
				case VNC_NEEDPASSWORD:
					GX_Text("Wachtwoord nodig...").Draw(100, 200);
					break;
				case VNC_CONNECTED:
					GX_Text("VNC verbonden...").Draw(100, 200);
					break;
				case VNC_DISCONNECTED:
					GX_Text("VNC niet verbonden...").Draw(100, 200);
					break;
			}
			//*/
			
			if(viewer->status == VNC_DISCONNECTED) {
				delete viewer;
				exit(0);
			}
			
		}
		//*/
			
		//*
		switch(network_status)
		{
			case NO_NETWORK:
				GX_Text("Geen netwerk verbinding...").Draw(100, 100);
				break;
			case NETWORK_CONNECTING:
				GX_Text("Bezig met verbinding maken...").Draw(100, 100);
				break;
			case NETWORK_CONNECTED:
				GX_Text("Verbinding gemaakt...").Draw(100, 100);
				break;
		}
		//*/
	
		
		//*
		if(network_status == NETWORK_CONNECTED && viewer == NULL) {
			printf("\n\n\n\n\n\n    Connecting to VNC...\n");
			viewer = new Viewer("192.168.0.128", 5900, "wac");
			cursor.listener = viewer;
		}
		//*/

		cursor.Draw();
	
		GX_Render();
		
	}
	return 0;
}
