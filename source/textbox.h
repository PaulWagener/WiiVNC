#ifndef TextboxH
#define TextboxH

#include "keyboard.h"
#include "gfx/textbox_back.h"

#define LEGAL_PASSWORD_CHARACTERS "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789`~!@#$%^&*()-=[]\\;',./_+{}|:\"<>? "
#define LEGAL_DOMAIN_CHARACTERS "abcdefghijklmnopqrstuvwxyz0123456789.-"
#define DIGIT_CHARACTERS "0123456789"

#define CURSOR_PERIOD 40

class Textbox : public KeyboardListener {
private:
	int x, y, width, height;
	
	//Current text
	char *text;
	
	//Rendered version of the text
	GX_Texture *textTexture;
	
	GX_Texture *backgroundTexture;
	
	//Position of blinking cursor, 0 means before first character
	int cursorPosition;
	
	//Position of the lightblue selected text. Are equal to eachother when there is no text selected
	int selStart;
	int selEnd;
	
	//Counter used to blink the cursor
	int cursorCount;
	
	//Used while dragging a selection
	bool dragging;
	int dragStart;
	
	//The i-th index of positions gives us the number of pixels up to and including the i-th character
	//positions[0] is always 0, positions[1] is the number of pixels after the first character, etc
	u16 *positions;

public:
	Textbox(int x, int y, int width, int height);
	~Textbox();
	void Update();
	void Draw();
	void OnKey(int keycode, bool isDown);
	void OnButton(bool isDown);
	void SetText(const char* text);
	const char* GetText();
	bool IsMouseOver(int x, int y);

	bool hasFocus;
private:
	bool IsLegalCharacter(u16 keycode);
	int GetPosition(int x);
	void RecalculatePositions();
	int CursorPositionFromCoordinate(int x);
	int GetTextX();
	void DeleteSelection();
};

#endif