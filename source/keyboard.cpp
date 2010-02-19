#include "keyboard.h"
#include "common.h"

//A character / uppercase character pair
struct ch_key {
	const char ch;
	const char ucase_ch;
};

/**
 * The keys are the clickable buttons that make up the keyboard
 */
class Key {
public:
	//Position & size of key
	int x, y, width;
	
	//How yellow the button is
	int click_fade; // 0 = no color, 255 = CLICK_COLOR
	
	//Sticky buttons have the property that the key-up and key-down
	//events can be sent seperately depending on how long the key is pressed
	bool sticky;
	
public:
	//Keeps track of if this key is pressed by the USB keyboard
	//Onscreen keyboard may not release keys that have been pressed by USB keyboard
	bool pressedByUSB;
	
	//Keep track of how long the button has been pressed
	//(used for repeat functionality when key is depressed longer than normal)
	bool pressed;
	
protected:
	int pressed_counter;
	
	//Reference to the keyboard to use for keyboard-wide variables
	Keyboard *keyboard;
	
	//How much bigger the button is than normal
	u8 grow;
	
public:
	Key(Keyboard *keyboard, int x, int y, int width);
	virtual void Update();
	virtual void Draw();
	
	virtual void Press();
	virtual void Release();
	
	virtual u16 GetKeyCode()=0;
	virtual bool HasKeyCode(u16 keycode)=0;
	bool IsMouseOver(int x, int y);
protected:
	virtual u32 GetBaseColor()=0;
private:
	virtual void Trigger();
};



/**
 * A key that can be pressed to print a character,
 * is used for all the dull grey keys.
 * Note that these keys can change character if shift is pressed
 */
class CharacterKey : public Key {
private:
	//Character & uppercase character this key represents
	struct ch_key key;
	
	//Texture chaches for the characters
	GX_Texture *lowercase_texture;
	GX_Texture *uppercase_texture;
	
public:
	CharacterKey(Keyboard *keyboard, int x, int y, struct ch_key key);
	~CharacterKey();
	void Draw();
	u16 GetKeyCode();
	bool HasKeyCode(u16 keycode);
protected:
	u32 GetBaseColor();
};

/**
 * Key that doesn't print a character, these are the lightblue keys with text on them.
 */
class CommandKey : public Key {
private:
	GX_Texture *text_texture;
	int key_code; //Keycode this key sends
	
public:
	CommandKey(Keyboard *keyboard, int x, int y, int width, const char* text, int key_code, bool sticky);
	~CommandKey();
	void Draw();
	bool HasKeyCode(u16 keycode);
	u16 GetKeyCode();
protected:
	u32 GetBaseColor();
};


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


/**
 * Draws the generic key background
 */
void Key::Draw()
{
	u32 base_color = keyboard->hoverButton == this ? HOVER_COLOR : this->GetBaseColor();
	u32 color = colortween(base_color, CLICK_COLOR, click_fade / (float)255);
	
	GX_Texture *texture = keyboard->buttonTexture;
	texture->Draw(keyboard->position_x + x - grow, keyboard->position_y + y - grow,
				  width + grow*2, texture->height + grow*2, keyboard->opacity, 0, color);
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

CharacterKey::~CharacterKey()
{
	delete lowercase_texture;
	delete uppercase_texture;
}

void CharacterKey::Draw()
{
	Key::Draw();
	
	GX_Texture *text_texture = keyboard->IsUppercase() ? uppercase_texture : lowercase_texture;	
	text_texture->Draw(keyboard->position_x + x + (keyboard->buttonTexture->width/2 - 7) - grow, keyboard->position_y + y - grow,
				 text_texture->width + grow, text_texture->height + grow*3, keyboard->opacity);
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

CommandKey::~CommandKey()
{
	delete text_texture;
}

void CommandKey::Draw()
{
	Key::Draw();
	
	text_texture->Draw(keyboard->position_x + x + 5 - grow, keyboard->position_y + y - grow,
						   text_texture->width + grow*2, text_texture->height + grow*2, keyboard->opacity);
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
	//Map both shifts onto one key
	if(keycode == KS_Shift_R && this->key_code == KS_Shift_L) 
		return true;
	
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

Keyboard::Keyboard(int position_x, int position_y, KeyboardType type) :
	position_x(position_x),
	position_y(position_y),
	opacity(0),
	show(false),
	shiftKey(NULL),
	capslockKey(NULL),
	pressedButton(NULL),
	hoverButton(NULL),
	listener(NULL),
	buttonTexture(GX_Texture::LoadFromPNG(key))
{
	//Initialize USB keyboard
	if(!keyboard_inited) {
		KEYBOARD_Init(NULL);
		keyboard_inited = true;
	}
	
	//Notify the controller that this is the latest keyboard
	//(used for ignoring mouse movements above keyboard)
	Controller::instance()->SetKeyboard(this);
	
	for(int i = 0; i < NUM_KEYS; i++)
		Keys[i] = NULL;	

	//Initialize a US layout keyboard
	const int row_pos[][2] = {
		{0,0},
		{60, 32},
		{70, 64},
		{90, 96},
	};
	const struct ch_key full_keys[4][13] = {{
		{'`','~'}, {'1','!'}, {'2','@'}, {'3','#'},	{'4','$'}, {'5','%'}, {'6','^'},
		{'7','&'}, {'8','*'}, {'9','('}, {'0',')'}, {'-','_'}, {'=','+'}
	}, {
		{'q','Q'}, {'w', 'W'}, {'e','E'}, {'r','R'}, {'t','T'}, {'y','Y'}, {'u','U'},
		{'i','I'}, {'o','O'}, {'p','P'}, {'[','{'}, {']','}'}, {'\\', '|'}
	}, {
		{'a', 'A'}, {'s', 'S'}, {'d', 'D'}, {'f', 'F'}, {'g', 'G'}, {'h', 'H'},
		{'j', 'J'},	{'k', 'K'}, {'l', 'L'}, {';', ':'},	{'\'', '"'}
	}, {
		{'z', 'Z'}, {'x', 'X'}, {'c', 'C'}, {'v', 'V'}, {'b', 'B'},
		{'n', 'N'}, {'m', 'M'}, {',', '<'}, {'.', '>'},
		{'/', '?'}
	}};
	
	const struct ch_key alpha_keys[4][13] = 
		{{{0, 0}, {'1',0}, {'2',0}, {'3',0}, {'4',0}, {'5',0}, {'6',0}, {'7',0}, {'8',0}, {'9',0}, {'0',0}},
		{{'q','Q'}, {'w', 'W'}, {'e','E'}, {'r','R'}, {'t','T'}, {'y','Y'}, {'u','U'}, {'i','I'}, {'o','O'}, {'p','P'}},
		{{'a', 'A'}, {'s', 'S'}, {'d', 'D'}, {'f', 'F'}, {'g', 'G'}, {'h', 'H'}, {'j', 'J'}, {'k', 'K'}, {'l', 'L'}},
		{{'z', 'Z'}, {'x', 'X'}, {'c', 'C'}, {'v', 'V'}, {'b', 'B'}, {'n', 'N'}, {'m', 'M'}, {'.', 0}}};
	
	const struct ch_key (&keys)[4][13] = type == ALPHA_NUMERIC? alpha_keys : full_keys;
	
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
	
	if(type == FULL || type == TEXT)
	{
		Keys[b++] = shiftKey = new CommandKey(this, 0, 96, 90, "Shift", KS_Shift_L, true);
		Keys[b++] = capslockKey = new CommandKey(this, 0, 64, 70, "CapsLock", KS_Shift_L, true);
	}
	
	if(type == FULL) {
		//Command Keys (left side)
		Keys[b++] = new CommandKey(this, 0, 32, 60, "Tab", KS_Tab);
		Keys[b++] = new CommandKey(this, 0, 128, 60, "Ctrl", KS_Control_L, true);
		Keys[b++] = new CommandKey(this, 60, 128, 60, "Alt", KS_Alt_L, true);
		Keys[b++] = new CommandKey(this, 120, 128, 50, "Meta", KS_Meta_L, true);
	
		//Command Keys (right side)
		Keys[b++] = new CommandKey(this, 410, 128, 40, "ESC", KS_Escape);
		Keys[b++] = new CommandKey(this, 500, 96, 40, "Up", KS_Up);
		Keys[b++] = new CommandKey(this, 500, 128, 40, "Down", KS_Down);
		Keys[b++] = new CommandKey(this, 510, 64, 70, "Return", KS_Return);
	}
	
	Keys[b++] = new CommandKey(this, type == ALPHA_NUMERIC ? 440 : 520, 0, 60, "Backspace", KS_BackSpace);
	Keys[b++] = new CommandKey(this, 370, 128, 40, "Del", KS_Delete);
	Keys[b++] = new CommandKey(this, type == FULL ? 460 : 90, 128, 40, "Left", KS_Left);
	Keys[b++] = new CommandKey(this, type == FULL ? 540 : 130, 128, 40, "Right", KS_Right);
}

Keyboard::~Keyboard() {
	
	for(int i = 0; i < NUM_KEYS; i++)
		if(Keys[i] != NULL)
			delete Keys[i];

	delete buttonTexture;
	Controller::instance()->SetKeyboard(NULL);
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

u16 localizeKeycode(u16 keycode)
{
	switch(keycode) {
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
		if(ke.type != KEYBOARD_PRESSED && ke.type != KEYBOARD_RELEASED)
			continue;
		
		bool isDown = ke.type == KEYBOARD_PRESSED;

		u16 keycode = localizeKeycode(ke.symbol);
		
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
				break;
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
	if(shiftKey == NULL || capslockKey == NULL)
		return false;
	
	return shiftKey->pressed || capslockKey->pressed;
}

/**
 * Update which key has the mouse hovering above it
 */
void Keyboard::OnMouseMove(int x, int y)
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
		if(Keys[i] != NULL && Keys[i]->IsMouseOver(Controller::GetX(), Controller::GetY())) {
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
			if(Keys[i] != NULL && Keys[i]->IsMouseOver(Controller::GetX(), Controller::GetY())) {
				
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