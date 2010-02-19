#include "textbox.h"

Textbox::Textbox(int x, int y, int width, int height) :
	x(x),
	y(y),
	width(width),
	height(height),
	text((char*)malloc(1)),
	textTexture(GX_Text(" ", 18, 0)),
	backgroundTexture(GX_Texture::LoadFromPNG(textbox_back)),
	cursorPosition(0),
	selStart(0),
	selEnd(0),
	cursorCount(0),
	dragging(false),
	dragStart(0),
	positions((u16*)malloc(sizeof(u16))),
	hasFocus(false)
{
	text[0] = '\0';
}

Textbox::~Textbox()
{
	delete textTexture;
	delete backgroundTexture;
	delete text;
	delete positions;	
}

void Textbox::Update() {
	cursorCount++;
	if(cursorCount > CURSOR_PERIOD * 2)
		cursorCount = 0;
	
	//Update selection
	if(dragging) {
		int position = CursorPositionFromCoordinate(Controller::GetX());
		if(position < dragStart)
		{
			selStart = position;
			selEnd = dragStart;
		} else {
			selStart = dragStart;
			selEnd = position;
		}
		
	}
}

void Textbox::Draw() {	
	//Draw background
	backgroundTexture->Draw(x, y, width, height);

	//Draw selection
	int offsetSel = GetTextX()+positions[selStart];
	GX_DrawRectangle(offsetSel, y+10, (GetTextX()+positions[selEnd])-offsetSel, height-20, 0xb5d5ff);
	
	//Draw text
	textTexture->Draw(GetTextX(), y + 10);

	//Draw cursor
	if(cursorCount < CURSOR_PERIOD && selEnd == selStart && hasFocus)
		GX_DrawRectangle(GetTextX() + positions[cursorPosition] - 2, y+10, 2, height-20, 0x000000);
}

bool Textbox::IsLegalCharacter(u16 keycode) {
	if(keycode > 255)
		return false;
	
	for(unsigned int i = 0; i < strlen(LEGAL_PASSWORD_CHARACTERS); i++)
		if(LEGAL_PASSWORD_CHARACTERS[i] == keycode)
			return true;
	
	return false;
}

void Textbox::SetText(const char* text)
{
	delete this->text;
	this->text = strdup(text);
	
	delete textTexture;
	textTexture = GX_Text(text, height/2 - (height/2)%2, 0);
	
	cursorPosition = selStart = selEnd = 0;
	RecalculatePositions();
}

const char* Textbox::GetText()
{
	return this->text;
}

void Textbox::DeleteSelection()
{
	if(selStart == selEnd)
		return;
	
	int numChars = strlen(text);
	
	//Copy all characters to the front (including \0)
	for(int i = selEnd; i <= numChars; i++)
		text[selStart + (i - selEnd)] = text[i];
	
	realloc(text, strlen(text)+1);
	
	//Replace textTexture
	delete textTexture;
	textTexture = GX_Text(text, height/2 - (height/2)%2, 0);
	
	cursorPosition = selStart;
	selEnd = selStart;
	RecalculatePositions();
}

void Textbox::OnKey(int keycode, bool isDown) {	
	if(selStart != selEnd && (IsLegalCharacter(keycode) || keycode == KS_BackSpace || keycode == KS_Delete)) {
		//Remove a selection if a key is pressed
		DeleteSelection();
		
	} else if(isDown && keycode == KS_BackSpace && cursorPosition > 0) {
		//Do a backspace
		selStart = cursorPosition - 1;
		selEnd = cursorPosition;
		DeleteSelection();
		
	} else if(isDown && keycode == KS_Delete && cursorPosition < (int)strlen(text)) {
		//Do a delete
		selStart = cursorPosition;
		selEnd = cursorPosition + 1;
		DeleteSelection();
	}
	
	//Start a selection drag from pressing shift
	if(keycode == KS_Shift_L || keycode == KS_Shift_R) {
		dragging = isDown;
		if(isDown)
			dragStart = selStart = selEnd = cursorPosition;
	}
	
	//Update cursor & selection with keyboard control
	if(isDown && keycode == KS_Right && cursorPosition < (int)strlen(text)) {
		if(!dragging) {
			if(selStart != selEnd) {
				//Cancel a selection
				cursorPosition = selEnd;
				selStart = selEnd;
			} else
				cursorPosition++;
		} else {
			if(selStart < dragStart && selStart < (int)strlen(text)) selStart++;
			else if(selEnd < (int)strlen(text))selEnd++;
		}
	}
		
	if(isDown && keycode == KS_Left && cursorPosition > 0) {
		if(!dragging) {
			if(selStart != selEnd) {
				cursorPosition = selStart;
				selStart = selEnd;
			} else
				cursorPosition--;			
		} else {
			if(selEnd > dragStart && selEnd > 0) selEnd--;
			else if(selStart > 0) selStart--;
		}
	}
	 
	
	//Insert character at cursorPosition
	if(IsLegalCharacter(keycode) && isDown) {		
		int numChars = strlen(text);
		text = (char*)realloc(text, numChars+2);
		
		for(int i = numChars; i >= cursorPosition; i--)
			text[i+1] = text[i];
		
		text[cursorPosition] = keycode;
		cursorPosition++;
		
		//Replace the text texture cache
		delete textTexture;
		textTexture = GX_Text(text, height/2 - (height/2)%2, 0);
		
		RecalculatePositions();
	}
	cursorCount = 0;
}

bool Textbox::IsMouseOver(int x, int y)
{
	return this->x < x && x < this->x + width &&
	this->y < y && y < this->y + height;
}

/**
 * Find out before what character the user clicked
 * x argument is an absolute screen position
 */
int Textbox::CursorPositionFromCoordinate(int x)
{
	const int textOffset = Controller::GetX() - GetTextX();
	int numChars = strlen(text);
	for(int i = 0; i < numChars; i++)
	{
		if((positions[i] + positions[i+1]) / 2 > textOffset)
			return i;
	}
	return numChars;
}

void Textbox::OnButton(bool isDown)
{
	if(!isDown)
		dragging = false;

	if(isDown) {
		hasFocus = IsMouseOver(Controller::GetX(), Controller::GetY());
		if(!hasFocus)
			selStart = selEnd = 0;			
	}
	
	//Place cursor at mouse click
	if(isDown && IsMouseOver(Controller::GetX(), Controller::GetY())) {
		cursorPosition = CursorPositionFromCoordinate(Controller::GetX());
		hasFocus = true;
		cursorCount = 0;
		
		//Reset dragging
		selStart = selEnd = cursorPosition;
		dragging = true;		
		dragStart = cursorPosition;		
	}	
}

/**
 * Recalculate every value in the 'positions' array
 */
void Textbox::RecalculatePositions()
{
	//Resize the positions array
	int num_chars = strlen(text);
	positions = (u16*)realloc(positions, (num_chars+1) * sizeof(positions[0]));
	
	positions[0] = 0;
	
	for(int i = 1; i < num_chars+1; i++)
	{
		char temp = text[i];
		text[i] = '\0';
		GX_Texture *tempText = GX_Text(text, height/2 - (height/2)%2, 0);
		positions[i] = tempText->width;
		delete tempText;
		text[i] = temp;
	}
}

int Textbox::GetTextX()
{
	return (x + x + width)/2 - (textTexture->width/2);
}
