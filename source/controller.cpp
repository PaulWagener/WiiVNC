#include "controller.h"

//Listener stubs
void ControllerListener::OnButton(bool isDown) {}
void ControllerListener::OnSecondaryButton(bool isDown) {}
void ControllerListener::OnMiddleButton(bool isDown) {}
void ControllerListener::OnScrollUp() {}
void ControllerListener::OnScrollDown() {}
void ControllerListener::OnZoomIn(bool isDown) {}
void ControllerListener::OnZoomOut(bool isDown) {}
void ControllerListener::OnHome() {}
void ControllerListener::OnCursorMove(int x, int y) {}
void ControllerListener::OnKeyboard() {}
void ControllerListener::OnScrollView(int x, int y) {}

#include <ogc/usbmouse.h>
Controller::Controller() :
	x(SCREEN_XCENTER),
	y(SCREEN_YCENTER),
	listener(NULL),
	cursor_texture(GX_Texture::LoadFromPNG(cursor_default)),
	keyboard(NULL)
{
	//Init controllers
	PAD_Init();
	WPAD_Init();
	WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);
	WPAD_SetVRes(WPAD_CHAN_0, SCREEN_WIDTH, SCREEN_HEIGHT);

	MOUSE_Init();
}

Controller::~Controller()
{
	delete cursor_texture;
	MOUSE_Deinit();
}

PADStatus pads[PAD_CHANMAX];
#include <stdlib.h>

u8 usb_oldbutton = 0;

#define USB_LEFTBUTTON		0x01
#define USB_RIGHTBUTTON		0x02
#define USB_MIDDLEBUTTON	0x04


void Controller::Update()
{
	bool onKeyboard = keyboard != NULL && keyboard->IsMouseOver(x, y); //TODO
	previous_x = x;
	previous_y = y;

	//USB Mouse
	if(MOUSE_IsConnected()) {
		mouse_event event;
		while(MOUSE_GetEvent(&event)) {
			x += event.rx;
			y += event.ry;
			
			if(listener != NULL)
			{
				u8 up = usb_oldbutton & ~event.button;
				u8 down = event.button & (event.button ^ usb_oldbutton);
				
				if(down & USB_LEFTBUTTON) listener->OnButton(true);
				if(up & USB_LEFTBUTTON) listener->OnButton(false);
				if(down & USB_MIDDLEBUTTON) listener->OnMiddleButton(true);
				if(up & USB_MIDDLEBUTTON) listener->OnMiddleButton(false);
				if(down & USB_RIGHTBUTTON) listener->OnSecondaryButton(true);
				if(up & USB_RIGHTBUTTON) listener->OnSecondaryButton(false);
			}
			
			usb_oldbutton = event.button;
		}
	}
	
	//Wii controller
	WPAD_ScanPads();
	WPADData *wd = WPAD_Data(WPAD_CHAN_0);
	
	if(wd->err == WPAD_ERR_NONE) {
		if(wd->ir.num_dots > 0) {
			x = wd->ir.x;
			y = wd->ir.y;
		}
	
		int wpad_down = wd->btns_d;
		int wpad_up = wd->btns_u;
		
		ControllerListener *l = onKeyboard ? keyboard : listener; //Buttonpresses may be intercepted by the on-screen keyboard
		if(l != NULL) {
			if(wpad_down & WPAD_BUTTON_A) l->OnButton(true);
			if(wpad_up & WPAD_BUTTON_A) l->OnButton(false);
			if(wpad_down & (WPAD_BUTTON_LEFT | WPAD_BUTTON_RIGHT)) l->OnMiddleButton(true);
			if(wpad_up & (WPAD_BUTTON_LEFT | WPAD_BUTTON_RIGHT)) l->OnMiddleButton(false);
			if(wpad_down & WPAD_BUTTON_B) l->OnSecondaryButton(true);
			if(wpad_up & WPAD_BUTTON_B) l->OnSecondaryButton(false);
		}
			
		
		if(listener != NULL) {
			if(wpad_down & WPAD_BUTTON_UP) listener->OnScrollUp();
			if(wpad_down & WPAD_BUTTON_DOWN) listener->OnScrollDown();
			
			if(wpad_down & WPAD_BUTTON_PLUS) listener->OnZoomIn(true);
			if(wpad_up & WPAD_BUTTON_PLUS) listener->OnZoomIn(false);
			if(wpad_down & WPAD_BUTTON_MINUS) listener->OnZoomOut(true);
			if(wpad_up & WPAD_BUTTON_MINUS) listener->OnZoomOut(false);		
			
			if(wpad_down & WPAD_BUTTON_HOME) listener->OnHome();
			
			if(wpad_down & WPAD_BUTTON_1) listener->OnKeyboard();
			
			//Scroll the view with the Nunchuk
			WPADData *data = WPAD_Data(WPAD_CHAN_0);
			
			if(data->exp.type == WPAD_EXP_NUNCHUK) {
				int cx = data->exp.nunchuk.js.mag * 10;
				int cy = data->exp.nunchuk.js.ang * 10;
				if(cx != 0 || cy != 0)
					listener->OnScrollView(cx, cy);	
			}
		}
	}
	 //*/

	//GC Controller
	PAD_ScanPads();
	
	//Allow any plugged in controller to work
	PAD_Read(pads);
	for(int controller = 0; controller < PAD_CHANMAX; controller++) {
		
		if(pads[controller].err == PAD_ERR_NO_CONTROLLER)
			continue;
		
		//Move the cursor with the big stick
		x += PAD_StickX(controller) / 10;
		y -= PAD_StickY(controller) / 10;
		

		int pad_down = PAD_ButtonsDown(controller);
		int pad_up = PAD_ButtonsUp(controller);

		//TODO: remove this
		if(listener == NULL && pad_down & PAD_BUTTON_START) exit(0);
		

		//Button event handlers
		ControllerListener *l = onKeyboard ? keyboard : listener; //Buttonpresses may be intercepted by the on-screen keyboard
		if(l != NULL) {
			if(pad_down & PAD_BUTTON_A) l->OnButton(true);
			if(pad_up & PAD_BUTTON_A) l->OnButton(false);
			if(pad_down & (PAD_BUTTON_LEFT | PAD_BUTTON_RIGHT)) l->OnMiddleButton(true);
			if(pad_up & (PAD_BUTTON_LEFT | PAD_BUTTON_RIGHT)) l->OnMiddleButton(false);
			if(pad_down & PAD_BUTTON_B) l->OnSecondaryButton(true);
			if(pad_up & PAD_BUTTON_B) l->OnSecondaryButton(false);
		}
		
		if(listener != NULL) {
			if(pad_down & PAD_BUTTON_UP) listener->OnScrollUp();
			if(pad_down & PAD_BUTTON_DOWN) listener->OnScrollDown();
			
			if(pad_down & PAD_BUTTON_Y) listener->OnZoomIn(true);
			if(pad_up & PAD_BUTTON_Y) listener->OnZoomIn(false);
			if(pad_down & PAD_BUTTON_X) listener->OnZoomOut(true);
			if(pad_up & PAD_BUTTON_X) listener->OnZoomOut(false);		
			
			if(pad_down & PAD_BUTTON_START) listener->OnHome();
			
			if(pad_down & PAD_TRIGGER_Z) listener->OnKeyboard();
			
			//Scroll the view with the C stick
			int cx = PAD_SubStickX(controller) / 10;
			int cy = PAD_SubStickY(controller) / 10;
			if(cx != 0 || cy != 0)
				listener->OnScrollView(cx, cy);	
			
		}
		 //*/
	}

	//Update cursor location
	if(x < 0) x = 0;
	if(x > SCREEN_WIDTH) x = SCREEN_WIDTH;
	if(y < 0) y = 0;
	if(y > SCREEN_HEIGHT) y = SCREEN_HEIGHT;
	
	ControllerListener *l = onKeyboard ? keyboard : listener;
	if(l != NULL && (x != previous_x || y != previous_y))
		l->OnCursorMove(x, y);

}

void Controller::SetListener(ControllerListener *listener)
{
	this->listener = listener;
}

void Controller::SetKeyboard(Keyboard *keyboard)
{
	this->keyboard = keyboard;
}

void Controller::Draw()
{
	cursor_texture->Draw(x, y);
}
