#ifndef CheckboxH
#define CheckboxH

#include "gx.h"

class Checkbox {
private:
	int x;
	int y;
	int width;
	int height;
	
public:
	bool checked;
	bool prevChecked;
	
private:
	GX_Texture *checkboxTexture;
	GX_Texture *checkboxTextureChecked;
	
public:
	Checkbox(int x, int y, int width, int height, bool checked=false);
	~Checkbox();
	void Update();
	void Draw();
	void OnButton(bool isDown);
};

#endif