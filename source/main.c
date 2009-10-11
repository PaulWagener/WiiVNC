#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ogcsys.h>
#include <gccore.h>

#include <rfb/rfbclient.h>
#include <network.h>
#include <errno.h>
#include "GRRLIB/GRRLIB.h"
	
	
Mtx GXmodelView2D;

static char* MyReadPassword(rfbClient* client) {
	printf("    MyReadPassword called\n");
	return strdup("wac");
}


/**
 * Wraps the GRRLIB_DrawImg function with parameters for width & height instead of scaleX & scaleY
 */
void DrawImg(float x, float y, float width, float height, GRRLIB_texImg texture, u8 opacity)
{
        float scaleX = width / texture.w;
        float scaleY = height / texture.h;

        //Adjust for the fact that images scale at their center
        float newX = x - (texture.w / 2) + (width / 2);
        float newY = y - (texture.h / 2) + (height / 2); 
        
        GRRLIB_DrawImg(newX, newY, texture, 0, scaleX, scaleY, opacity);
} 

u8* texture;

#define PIXEL_SIZE 4
#define TEXEL_SIZE 4 * 4 * PIXEL_SIZE

int tex_width = 1000, tex_height = 800;

void copypixeltotexture(rfbClient* client, int x, int y) {
	u8* pixel = client->frameBuffer + ((y*client->width) + x) * PIXEL_SIZE;
	int texel_x = x / 4,
		texel_y = y / 4;

	//Find pointer to texel
	u8* texel_pixel = texture + ((texel_y*( (tex_width/4))) + texel_x)*TEXEL_SIZE;
	
	int local_x = x % 4,
		local_y = y % 4;

	//Jump to AR pixel
	texel_pixel += ((local_y*4)+local_x)*(PIXEL_SIZE/2);
	texel_pixel[0] = 255;
	texel_pixel[1] = pixel[0]; //0 = green, 1 = blue, 2 = red
	
	//Jump to GB pixel
	texel_pixel += TEXEL_SIZE/2;
	texel_pixel[0] = pixel[1];
	texel_pixel[1] = pixel[2];
}

int min(int a, int b)
{
	return a < b ? a : b;
}

bool graphical = false;
static void update(rfbClient* client, int x, int y, int w, int h) {
	printf("    UPDATE %u %u %u %u\n",x,y,w,h);
	if(!graphical)
		return;
	int i_x, i_y;
	
	
	for(i_x = 0; i_x < min(x + w, tex_width); i_x++)
	{
		for(i_y = 0; i_y < min(y + h, tex_height); i_y++)
		{
			//copypixeltotexture(client, i_x, i_y);
		}
	}
/*	texture[0] = 255;
	texture[1] = 255;
	texture[TEXEL_SIZE/2 + 0] = 255;
	texture[TEXEL_SIZE/2 + 1] = 255;
//*/	
	
	//DCFlushRange(texture, tex_width*tex_height*4);
	//GX_InvalidateTexAll();
}

void *xfb[2];
void framebuffertest()
{
	
static GXRModeObj *rmode = NULL;
int fbi = 0;

u8 *fb;

	VIDEO_Init();
	PAD_Init();
 
	rmode = VIDEO_GetPreferredMode(NULL);
 
	xfb[0] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	xfb[1] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

	VIDEO_Configure(rmode);
	VIDEO_SetNextFramebuffer(xfb);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();
 
	while(1) {
		VIDEO_ClearFrameBuffer(rmode,xfb[fbi],COLOR_BLACK);
		
		PAD_ScanPads();

		if(PAD_ButtonsHeld(0) & PAD_BUTTON_START)
			exit(0);
			
		fb = (u8*)xfb[fbi];
		
		
		int x, y;
		for(x = 30; x < 300; x++) {
			for(y = 30; y < 500; y++) {/*
				fb[(rmode->fbWidth/VI_DISPLAY_PIX_SZ*y + x)*4 + 0] = 255;
				fb[(rmode->fbWidth/VI_DISPLAY_PIX_SZ*y + x)*4 + 1] = 255;
				fb[(rmode->fbWidth/VI_DISPLAY_PIX_SZ*y + x)*4 + 2] = 0;
				fb[(rmode->fbWidth/VI_DISPLAY_PIX_SZ*y + x)*4 + 3] = 0;//*/
			}
		}

		//fb[rmode->fbWidth*403 + 400] = COLOR_GREEN;
//				drawdot(xfb[fbi], rmode, rmode->fbWidth, rmode->xfbHeight, wd->ir.x, wd->ir.y, COLOR_RED);
//				drawdot(xfb[fbi], rmode, rmode->fbWidth, rmode->xfbHeight, wd->ir.x + 10*sinf(theta), wd->ir.y - 10*cosf(theta), COLOR_BLUE);
 
		VIDEO_SetNextFramebuffer(xfb[fbi]);
		VIDEO_Flush();
		VIDEO_WaitVSync();
		fbi ^= 1;
	}//*
}

int main(int argc, char **argv) {
	framebuffertest();
	VIDEO_Init();

	static GXRModeObj *vmode;

		
	vmode = VIDEO_GetPreferredMode(NULL);

	xfb[0] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(vmode));
	xfb[1] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(vmode));
	console_init(xfb[0],20,20,vmode->fbWidth,vmode->xfbHeight,vmode->fbWidth*VI_DISPLAY_PIX_SZ);
	VIDEO_Configure(vmode);
	VIDEO_SetNextFramebuffer(xfb[0]);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(vmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();
	//*
	printf("\n\n\n\n\n\n\n\n\n\n");
	printf("              WiiVNC test\n");
	
	printf("      Waiting for network to initialize...");
	
	s32 ip;
    while ((ip = net_init()) == -EAGAIN)
	{
		printf(".");
		usleep(100 * 1000); //100ms
	}
	printf("\n");
	
    if(ip < 0) {
		printf("      Error while initialising, error is %i, exiting\n", ip);
		usleep(1000 * 1000 * 1); //1 sec
		exit(0);
	}
        
	char myIP[16];
    if (if_config(myIP, NULL, NULL, true) < 0) {
		printf("      Error reading IP address, exiting");
		usleep(1000 * 1000 * 1); //1 sec
		exit(0);
	}
	printf("      Network initialised.\n");
	printf("      IP: %s\n\n", myIP);
	printf("      Have Fun!");
	

	
	rfbClient* client;
	client=rfbGetClient(8,3,4);
	client->programName = "Test";
	client->serverHost = strdup("192.168.0.130");
	client->serverPort = 5900;
	client->GetPassword = MyReadPassword;
	client->GotFrameBufferUpdate = update;

	printf("       Trying to connect to %s on port %u\n", client->serverHost, client->serverPort);
	if(!rfbInitConnection(client)) {
		printf("         Connection failed\n");
		rfbClientCleanup(client);
		exit(0);
	}
	
	printf("          Connection succeeded (%u, %u)\n", client->width, client->height);
	
	
	printf("    End of program: Press HOME.\n");
	#include <wiiuse/wpad.h>
	#include <ogcsys.h>
	PAD_Init();
	printf("   Start event loop\n");
	
	sleep(1);
	//*/
	while(1)
	{
	/*
		if(WaitForMessage(client,50)<0)
			break;
		if(!HandleRFBServerMessage(client))
			break;

		PAD_ScanPads();
		if(PAD_ButtonsHeld(0) & PAD_BUTTON_START)
			exit(0);
		//*/
		if(1){//wpaddown & WPAD_BUTTON_1 || 1) {
			graphical = true;
			printf("     Entering graphical mode\n\n");

			GRRLIB_InitVideo();
			GRRLIB_Start();
			PAD_Init();
			
			int kl = 10;
//			int width = tex_width, height = tex_height;
			texture = memalign(32, tex_width*tex_height*4);//client->width * client->height * 4);
			memset(texture, 255, tex_width*tex_height*4);//client->width * client->height * 4);
			DCFlushRange(texture, tex_width*tex_height*4);//client->width * client->height * 4);

			while(1) {
				GRRLIB_texImg tex;
				tex.w = tex_width;//client->width;
				tex.h = tex_height;//client->height;			
				tex.data = texture;

				GRRLIB_FillScreen(0xFF000000);
				DrawImg(0, 0, 640, 480, tex, 255);
				GRRLIB_Rectangle(315,100 + (kl%300),10,50,0xFF0000FF,1);


				if(WaitForMessage(client,50)<0)
					break;
				if(!HandleRFBServerMessage(client))
					break;
//*/				
				GRRLIB_Render();
				kl += 5;
				PAD_ScanPads();
				
/*				if(PAD_ButtonsHeld(0) & PAD_BUTTON_A) {
					memset(texture, 255, client->width * client->height * 4);
				}
*/				
				if(PAD_ButtonsHeld(0) & PAD_BUTTON_START)
					exit(0);
			}
		}
		
		//if(wpadup & WPAD_BUTTON_A) {
		//	SendKeyEvent(client,XK_a, false);
		//}
				
	}

	return 0;
}
