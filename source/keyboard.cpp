#include "keyboard.h"

Controller *Keyboard::controller = NULL;
bool Keyboard::keyboard_inited = false;


/*===========*\
      Key
\*===========*/

Key::Key(Keyboard *theKeyboard, int x, int y, int width) :
	x(x),
	y(y),
	width(width),
	click_fade(0),
	sticky(false),
	pressedByUSB(false),
	pressed(false),
	pressed_counter(0),
	keyboard(theKeyboard),
	grow(0)
{
	return;
}

void Key::Update()
{
	//Size & color animation
	if(keyboard->hoverButton == this && grow < BUTTON_GROW_MAX) grow += GROW_SPEED;
	else if(keyboard->hoverButton != this && grow > 0) grow -= GROW_SPEED;
	
	if(sticky) {
		//Fade in or out depending on if the key is pressed
		if(pressed && click_fade < 255) click_fade += CLICKFADE_SPEED;
		if(!pressed && click_fade > 0) click_fade -= CLICKFADE_SPEED;
		if(click_fade < 0) click_fade = 0;
		if(click_fade > 255) click_fade = 255;
		
	} else {		
		//Always fade away
		if(click_fade > 0) click_fade -= CLICKFADE_SPEED;
		else click_fade = 0;
		
		//Do the repeating behaviour if the key remains depressed
		if(pressed) {
			pressed_counter++;
		
			if(pressed_counter > PRESS_DELAY && pressed_counter % PRESS_REPEAT == 0)
				Trigger();
		}
	}
}

/**
 * Flash yellow and create a key event
 */
void Key::Trigger()
{
	click_fade = 255;
	if(keyboard->listener != NULL && GetKeyCode() != 0) {
		keyboard->listener->OnKey(GetKeyCode(), true);
		keyboard->listener->OnKey(GetKeyCode(), false);
	}
}


//Color animation functions
u8 tween(u8 v1, u8 v2, float progress) {
	return v1 + (v2 - v1) * progress;
}

u32 colortween(u32 from_color, u32 to_color, float progress) {
	return tween(from_color, to_color, progress) |
			(tween(from_color >> 8, to_color >> 8, progress) << 8) |
			(tween(from_color >> 16, to_color >> 16, progress) << 16) |
			(tween(from_color >> 24, to_color >> 24, progress) << 24);
}

/**
 * Draws the generic key background
 */
void Key::Draw()
{
	u32 base_color = keyboard->hoverButton == this ? HOVER_COLOR : this->GetBaseColor();
	u32 color = colortween(base_color, CLICK_COLOR, click_fade / (float)255);
	
	GX_Texture *texture = keyboard->buttonTexture;
	texture->Draw(keyboard->position_x + x - grow, keyboard->position_y + y - grow,
				  width + grow*2, texture->height + grow*2, keyboard->opacity, color);
}

/**
 * Determine if the mouse is hovering over this key
 */
bool Key::IsMouseOver(int mouse_x, int mouse_y) {
	GX_Texture *texture = keyboard->buttonTexture;
	
	return	keyboard->position_x + x <= mouse_x && mouse_x < keyboard->position_x + x + width &&
			keyboard->position_y + y <= mouse_y && mouse_y < keyboard->position_y + y + texture->height;
}

void Key::Press()
{
	if(pressed || pressedByUSB)
		return;
	
	keyboard->pressedButton = this;
	pressed = true;
	pressed_counter = 0;
	
	if(sticky) {
		if(keyboard->listener != NULL)
			keyboard->listener->OnKey(GetKeyCode(), true);

	} else {
		//Press normal keys like normal
		Trigger();
	}
}

void Key::Release()
{
	if(pressedByUSB)
		return;
	
	keyboard->pressedButton = NULL;
	pressed = false;

	//Send the key up event if this key was sticky
	if(sticky && keyboard->listener != NULL)
		keyboard->listener->OnKey(GetKeyCode(), false);
	
	//If a non-sticky button is released all sticky buttons also get released
	if(!sticky) {
		
		for(int i = 0; i < NUM_KEYS; i++)
		{
			Key *key = keyboard->Keys[i];
			if(key != NULL && key->sticky && key->pressed && !key->pressedByUSB && key != keyboard->capslockKey)//Don't release CapsLock
				key->Release();
		}
	}
}



/*===========*\
 Character Key
\*===========*/

CharacterKey::CharacterKey(Keyboard *keyboard, int x, int y, struct ch_key key)
		: Key(keyboard, x, y, keyboard->buttonTexture->width),
	key(key),
	lowercase_texture( GX_Text( (char[2]){key.ch, '\0'} , 20, 0) ),
	uppercase_texture( GX_Text( (char[2]){key.ucase_ch, '\0'} , 20, 0) )
{
}

void CharacterKey::Draw()
{
	Key::Draw();
	
	GX_Texture *text_texture = keyboard->IsUppercase() ? &uppercase_texture : &lowercase_texture;	
	text_texture->Draw(keyboard->position_x + x + (keyboard->buttonTexture->width/2 - 7) - grow, keyboard->position_y + y - grow,
				 text_texture->width + grow*30, text_texture->height + grow*3, keyboard->opacity);
}

u32 CharacterKey::GetBaseColor()
{
	return 0xFFFFFF;
}

u16 CharacterKey::GetKeyCode()
{
	return keyboard->IsUppercase() ? key.ucase_ch : key.ch;
}

bool CharacterKey::HasKeyCode(u16 keycode)
{
	return key.ch == keycode || key.ucase_ch == keycode;
}


/*===========*\
  Command Key
\*===========*/

CommandKey::CommandKey(Keyboard *keyboard, int x, int y, int width, const char* text, int key_code, bool sticky=false)
	: Key(keyboard, x, y, width),
	text_texture(GX_Text( text, 12, 0)),
	key_code(key_code)
{
	this->sticky = sticky;
}

void CommandKey::Draw()
{
	Key::Draw();
	
	text_texture.Draw(keyboard->position_x + x + 5 - grow, keyboard->position_y + y - grow,
						   text_texture.width + grow*30, text_texture.height + grow*2, keyboard->opacity);
}

u32 CommandKey::GetBaseColor()
{
	return COMMAND_COLOR;
}

u16 CommandKey::GetKeyCode()
{
	return key_code;				
}

bool CommandKey::HasKeyCode(u16 keycode)
{
	return this->key_code == keycode;
}



/*==============*\
 Keyboard Listener
\*==============*/

void KeyboardListener::OnKey(int keycode, bool isDown)
{
}

/*===========*\
   Keyboard
\*===========*/

Keyboard::Keyboard() :
	position_x(40),
	position_y(300),
	opacity(0),
	show(false),
	shiftKey(NULL),
	capslockKey(NULL),
	usbShiftPressed(false),
	pressedButton(NULL),
	hoverButton(NULL),
	listener(NULL),
	buttonTexture(GX_Texture::LoadFromPNG(button))
{
	//Initialize USB keyboard
	if(!keyboard_inited) {
		KEYBOARD_Init(NULL);
		keyboard_inited = true;
	}
	
	//Notify the controller that this is the latest keyboard
	//(used for ignoring mouse movements above keyboard)
	controller->SetKeyboard(this);
	
	for(int i = 0; i < NUM_KEYS; i++)
		Keys[i] = NULL;	

	//Initialize a US layout keyboard
	const int row_pos[][2] = {
		{0,0},
		{60, 32},
		{70, 64},
		{90, 96},
	};
	const struct ch_key keys[][13] = {{
		{'`','~'}, {'1','!'}, {'2','@'}, {'3','#'},	{'4','$'}, {'5','%'}, {'6','^'},
		{'7','&'}, {'8','*'}, {'9','('}, {'0',')'}, {'-','_'}, {'=','+'}
	}, {
		{'q','Q'}, {'w','W'}, {'e','E'}, {'r','R'}, {'t','T'}, {'y','Y'}, {'u','U'},
		{'i','I'}, {'o','O'}, {'p','P'}, {'[','{'}, {']','}'}, {'\\', '|'}
	}, {
		{'a', 'A'}, {'s', 'S'}, {'d', 'D'}, {'f', 'F'}, {'g', 'G'}, {'h', 'H'},
		{'j', 'J'},	{'k', 'K'}, {'l', 'L'}, {';', ':'},	{'\'', '"'}
	}, {
		{'z', 'Z'}, {'x', 'X'}, {'c', 'C'}, {'v', 'V'}, {'b', 'B'},
		{'n', 'N'}, {'m', 'M'}, {',', '<'}, {'.', '>'},
		{'/', '?'}
	}};
	
	int b = 0;
	for(unsigned int row = 0; row < sizeof(keys) / sizeof(keys[0]); row++) {
		int x = row_pos[row][0];
		int y = row_pos[row][1];
		
		//Create a row of buttons
		for (unsigned int i = 0; i < sizeof(keys[row]) / sizeof(keys[row][0]); ++i) {
			if(keys[row][i].ch != '\0')
			   Keys[b++] = new CharacterKey(this, x, y, keys[row][i]);
			
			x += buttonTexture->width;
		}
	}

	//Special extra wide space key
	Key *spaceKey = new CharacterKey(this, 170, 128, (struct ch_key){' ', ' '});
	spaceKey->width = 200;
	Keys[b++] = spaceKey;
	
	//Command Keys (left side)
	Keys[b++] = new CommandKey(this, 0, 32, 60, "Tab", VNC_TAB);
	Keys[b++] = shiftKey = new CommandKey(this, 0, 96, 90, "Shift", VNC_SHIFTLEFT, true);
	Keys[b++] = capslockKey = new CommandKey(this, 0, 64, 70, "CapsLock", VNC_SHIFTLEFT, true);
	Keys[b++] = new CommandKey(this, 0, 128, 60, "Ctrl", VNC_CTRLLEFT, true);
	Keys[b++] = new CommandKey(this, 60, 128, 60, "Alt", VNC_ALTLEFT, true);
	Keys[b++] = new CommandKey(this, 120, 128, 50, "Meta", VNC_METALEFT, true);
	
	//Command Keys (right side)
	Keys[b++] = new CommandKey(this, 370, 128, 40, "Del", VNC_DELETE);
	Keys[b++] = new CommandKey(this, 410, 128, 40, "ESC", VNC_ESCAPE);
	Keys[b++] = new CommandKey(this, 460, 128, 40, "Left", VNC_LEFT);
	Keys[b++] = new CommandKey(this, 500, 128, 40, "Down", VNC_DOWN);
	Keys[b++] = new CommandKey(this, 540, 128, 40, "Right", VNC_RIGHT);
	Keys[b++] = new CommandKey(this, 500, 96, 40, "Up", VNC_UP);
	
	Keys[b++] = new CommandKey(this, 510, 64, 70, "Return", VNC_ENTER);
	Keys[b++] = new CommandKey(this, 520, 0, 60, "Backspace", VNC_BACKSPACE);
}

Keyboard::~Keyboard() {
	
	for(int i = 0; i < NUM_KEYS; i++)
		if(Keys[i] != NULL)
			delete Keys[i];

	delete buttonTexture;
	controller->SetKeyboard(NULL);
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
		
		//US Localization (row 1)
		case KS_at:			return '`';
		case KS_section:	return '~';
		case KS_quotedbl:	return '@';
		case KS_ampersand:	return '^';
		case KS_underscore:	return '&';
		case KS_parenleft:	return '*';
		case KS_parenright:	return '(';
		case KS_apostrophe:	return ')';
		case KS_slash:		return '-';
		case KS_question:	return '_';
		case KS_degree:		return '=';
		
		//US Localization (row 2)
		case KS_asterisk:	return ']';
		case KS_bar:		return '}';
		case KS_less:		return '\\';
		case KS_greater:	return '|';
		
		//US Localization (row 3)
		case KS_plus:		return ';';
		case KS_plusminus:	return ':';	
			
		//US Localization (row 4)
		case KS_semicolon:	return '<';
		case KS_colon:		return '>';
		case KS_minus:		return '/';
		case KS_equal:		return '?';
	}
	return keycode;
}

bool Keyboard::Visible()
{
	return opacity > 0;
}

void Keyboard::Show()
{
	UpdateHoverButton();
	show = true;
}

void Keyboard::Hide()
{
	hoverButton = NULL;
	show = false;
}

void Keyboard::Update()
{
	//Fade keyboard in & out
	if(show) opacity += KEYBOARD_APPEAR_SPEED;
	else opacity -= KEYBOARD_DISAPPEAR_SPEED;
	
	if(opacity > 255) opacity = 255;
	if(opacity < 0) opacity = 0;	
	
	//Alow all keys to update their state
	for(int i = 0; i < NUM_KEYS; i++)
		if(Keys[i] != NULL)
			Keys[i]->Update();

	
	//Send keys pressed by a USB keyboard
	keyboard_event ke;
	while(KEYBOARD_GetEvent(&ke))
	{
		bool isDown;
		if(ke.type == KEYBOARD_PRESSED) {
			isDown = true;
		} else if(ke.type == KEYBOARD_RELEASED) {
			isDown = false;
		} else {
			continue;
		}

		u16 keycode = keycodeToVNC(ke.symbol);
		
		//Let onscreen shiftstate be the same as the usb keyboard shiftstate
		if(keycode == VNC_SHIFTLEFT || keycode == VNC_SHIFTRIGHT)
			shiftKey->pressed = usbShiftPressed = (ke.type == KEYBOARD_PRESSED);

		//Look for a key on the onscreen keyboard to press
		//(This takes care of the repeating behaviour and also does the yellow flashing)
		bool foundKey = false;
		for(int i = 0; i < NUM_KEYS; i++)
		{
			if(Keys[i] != NULL && Keys[i]->HasKeyCode(keycode)) {
				if(isDown) {
					Keys[i]->Press();
					Keys[i]->pressedByUSB = true;
				} else {
					Keys[i]->pressedByUSB = false;
					Keys[i]->Release();
				}
				foundKey = true;
			}
		}
		
		//In case the key isn't represented by the onscreen keyboard send it manually
		if(!foundKey && listener != NULL)
			listener->OnKey(keycode, isDown);

	}
}

void Keyboard::Draw()
{
	if(opacity == 0)
		return;
	
	//Draw all the keys
	for(int i = 0; i < NUM_KEYS; i++)
		if(Keys[i] != NULL)
			Keys[i]->Draw();

	//Draw the button the cursor is over a second time
	//to make sure it is not drawn below other buttons
	if(hoverButton && opacity == 255)
		hoverButton->Draw();
}

bool Keyboard::IsUppercase()
{
	return shiftKey->pressed || capslockKey->pressed || usbShiftPressed;
}

/**
 * Update which key has the mouse hovering above it
 */
void Keyboard::OnCursorMove(int x, int y)
{
	//Do not update hover information if the user is pressing a key
	if(pressedButton == NULL)
		UpdateHoverButton();
}

/**
 * Update hoverButton & all the Key::hover fields
 */
void Keyboard::UpdateHoverButton()
{
	hoverButton = NULL;
	
	//Update for all keys wether there is a pointer hovering above it
	for(int i = 0; i < NUM_KEYS; i++) {
		if(Keys[i] != NULL && Keys[i]->IsMouseOver(controller->GetX(), controller->GetY())) {
			hoverButton = Keys[i];
			return;
		}
	}
}

void Keyboard::SetListener(KeyboardListener *listener)
{
	this->listener = listener;
}

/**
 * User clicks with his pointer
 */
void Keyboard::OnButton(bool isDown)
{
	if(isDown) {
		//Start pressing the key that the user clicked on
		for(int i = 0; i < NUM_KEYS; i++) {
			if(Keys[i] != NULL && Keys[i]->IsMouseOver(controller->GetX(), controller->GetY())) {
				
				//Toggle sticky buttons
				if(Keys[i]->sticky) {
					if(Keys[i]->pressed)
						Keys[i]->Release();
					else
						Keys[i]->Press();
					
				//Press normal keys like normal
				} else
					Keys[i]->Press();
				
				return;
			}
		}
		
		//If there is an button up event and the user was pressing a key
		//release that key and update the hoverButton information
	} else if(pressedButton != NULL) {
		if(!pressedButton->sticky)
			pressedButton->Release();
		else
			pressedButton = NULL;

		UpdateHoverButton();
	}
}

/**
 * Returns if the coordinate is hovering over a key
 */
bool Keyboard::IsMouseOver(int x, int y)
{
	if(!IsVisible())
		return false;

	for(int i = 0; i < NUM_KEYS; i++)
		if(Keys[i] != NULL && Keys[i]->IsMouseOver(x, y))
			return true;

	return false;
}

bool Keyboard::IsVisible()
{
	return opacity >= 255;
}