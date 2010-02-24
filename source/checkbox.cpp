#include "checkbox.h"
#include "gfx/checkbox_unchecked.h"
#include "gfx/checkbox_checked.h"
#include "controller.h"

Checkbox::Checkbox(int x, int y, int width, int height, bool checked) :
	x(x),
	y(y),
	width(width),
	height(height),
	checked(checked),
	prevChecked(checked)
{
	checkboxTexture = GX_Texture::LoadFromPNG(checkbox_unchecked);
	checkboxTextureChecked = GX_Texture::LoadFromPNG(checkbox_checked);
}

Checkbox::~Checkbox()
{
	delete checkboxTexture;
	delete checkboxTextureChecked;
}

void Checkbox::Draw()
{
	GX_Texture *texture = checked ? checkboxTextureChecked : checkboxTexture;
	texture->Draw(x, y, width, height);
}

void Checkbox::OnButton(bool isDown)
{
	int cx = Controller::GetX();
	int cy = Controller::GetY();
	
	if(isDown && x < cx && cx < x + width && y < cy && cy < y + height)
		checked = !checked;
}