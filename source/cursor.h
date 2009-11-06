#ifndef CursorH
#define CursorH

#include "gx.h"

class Cursor {
public:
	int x, y;
	Cursor();
	~Cursor();
	void Update();
	void Draw();
private:
	GX_Texture *texture;
};

#endif
