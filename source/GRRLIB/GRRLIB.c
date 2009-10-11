/*===========================================
        GRRLIB (GX version) 3.0.1 alpha
        Code     : NoNameNo
        GX hints : RedShade
===========================================*/

#include "GRRLIB.h"
#include "../libpng/pngu/pngu.h"
#include "../libjpeg/jpgogc.h"
#define DEFAULT_FIFO_SIZE (256 * 1024)

const GRRLIB_texImg empty_texture = {0,0,NULL};

 u32 fb=0;
 static void *xfb[2] = { NULL, NULL};
 GXRModeObj *rmode;
 void *gp_fifo = NULL;


inline void GRRLIB_FillScreen(u32 color){
	GRRLIB_Rectangle(-40, -40, 680,520, color, 1);
}

inline void GRRLIB_Plot(f32 x,f32 y, u32 color){
   Vector  v[]={{x,y,0.0f}};
   GXColor c[]={GRRLIB_Splitu32(color)};
	
	GRRLIB_NPlot(v,c,1);
}
void GRRLIB_NPlot(Vector v[],GXColor c[],long n){
	GRRLIB_GXEngine(v,c,n,GX_POINTS);
}

inline void GRRLIB_Line(f32 x1, f32 y1, f32 x2, f32 y2, u32 color){
   Vector  v[]={{x1,y1,0.0f},{x2,y2,0.0f}};
   GXColor col = GRRLIB_Splitu32(color);
   GXColor c[]={col,col};

	GRRLIB_NGone(v,c,2);
}

inline void GRRLIB_Rectangle(f32 x, f32 y, f32 width, f32 height, u32 color, u8 filled){
   Vector  v[]={{x,y,0.0f},{x+width,y,0.0f},{x+width,y+height,0.0f},{x,y+height,0.0f},{x,y,0.0f}};
   GXColor col = GRRLIB_Splitu32(color);
   GXColor c[]={col,col,col,col,col};

	if(!filled){
		GRRLIB_NGone(v,c,5);
	}
	else{
		GRRLIB_NGoneFilled(v,c,4);
	}
}
void GRRLIB_NGone(Vector v[],GXColor c[],long n){
	GRRLIB_GXEngine(v,c,n,GX_LINESTRIP);
}
void GRRLIB_NGoneFilled(Vector v[],GXColor c[],long n){
	GRRLIB_GXEngine(v,c,n,GX_TRIANGLEFAN);
}

GRRLIB_texImg GRRLIB_LoadTexture(const unsigned char* my_png) {
   PNGUPROP imgProp;
   IMGCTX ctx;
   void *my_texture;

   	ctx = PNGU_SelectImageFromBuffer(my_png);
    PNGU_GetImageProperties (ctx, &imgProp);
    my_texture = memalign (32, imgProp.imgWidth * imgProp.imgHeight * 4);
	
	if(my_texture == NULL) {
		PNGU_ReleaseImageContext (ctx);
		return empty_texture;
	}
		
		
	GRRLIB_texImg texture;
	texture.w = imgProp.imgWidth;
	texture.h = imgProp.imgHeight;
	texture.data = my_texture;
    int pngreturncode = PNGU_DecodeTo4x4RGBA8 (ctx, imgProp.imgWidth, imgProp.imgHeight, my_texture, 255);
	PNGU_ReleaseImageContext (ctx);
												
	if(pngreturncode != PNGU_OK) {
		free(my_texture);
		return empty_texture;
	}
	
	
    DCFlushRange (my_texture, imgProp.imgWidth * imgProp.imgHeight * 4);
	return texture;
}

u8 to255(int value)
{
	if(value < 0)
		return 0;
	if(value > 255)
		return 255;
		
	return value;
}

GRRLIB_texImg GRRLIB_LoadTextureJPEG(const unsigned char* jpegdata, unsigned int jpegsize) {
	//Decompress JPEG
	JPEGIMG         jpeg;
    memset(&jpeg, 0, sizeof(JPEGIMG));

    jpeg.inbuffer = (char*)jpegdata;
    jpeg.inbufferlength = jpegsize;
	
	JPEG_Decompress(&jpeg);
	
	u8* input = (u8*)jpeg.outbuffer;
	
	
	GRRLIB_texImg texture;
	texture.w = jpeg.width;
	texture.h = jpeg.height;

	
	texture.data = memalign(32, texture.w * texture.h * 4);
	
	if(texture.data == NULL)
		return empty_texture;
		
	//Convert the YCbYCr jpeg outbutbuffer to 4x4 RGBA8
	u32 x, y, x1, y1, x2, y2, block = 0;
	
	//Iterate through all 4x4 blocks
	for(y1 = 0; y1 < texture.h; y1 += 4)
	{
		for(x1 = 0; x1 < texture.w; x1 += 4)
		{
			u8 *ar_block = texture.data + block * 64;
			u8 *gb_block = ar_block + 32;
			
			//Iterate through all pixelpairs within block
			for(y2 = 0; y2 < 4; y2++)
			{
				for(x2 = 0; x2 < 4; x2 += 2)
				{
					x = x1 + x2;
					y = y1 + y2;
					
					u8 *ar_blockpixel = ar_block + (y2 * 4 + x2) * 2;
					u8 *gb_blockpixel = gb_block + (y2 * 4 + x2) * 2;
					
					u8 *pixel = input + (y * texture.w + x) * 2;
					
					//YCbYCr to RGB
					int r = 1.371f * (pixel[3] - 128); 
					int g = -0.698f * (pixel[3] - 128) - 0.336f * (pixel[1] - 128);
					int b = 1.732f * (pixel[1] - 128);
					u8 r1 = to255(pixel[0] + r);
					u8 g1 = to255(pixel[0] + g);
					u8 b1 = to255(pixel[0] + b);
			
					u8 r2 = to255(pixel[2] + r);
					u8 g2 = to255(pixel[2] + g);
					u8 b2 = to255(pixel[2] + b);
					
					//Fill in buffer
					ar_blockpixel[0] = 255; //alpha
					ar_blockpixel[1] = r1;
					ar_blockpixel[2] = 255; //alpha
					ar_blockpixel[3] = r2;
					
					gb_blockpixel[0] = g1;
					gb_blockpixel[1] = b1;
					gb_blockpixel[2] = g2;
					gb_blockpixel[3] = b2;
				}
			}
			block++;
		}
	}
	
	free(jpeg.outbuffer);
	DCFlushRange (texture.data, texture.w * texture.h * 4);
	
	return texture;
}

inline void GRRLIB_DrawImg(f32 xpos, f32 ypos, GRRLIB_texImg texture, float degrees, float scaleX, f32 scaleY, u8 alpha ){
   GXTexObj texObj;

	int width = texture.w;
	int height = texture.h;
	u8 *data = texture.data;
	
	GX_InitTexObj(&texObj, data, width,height, GX_TF_RGBA8,GX_CLAMP, GX_CLAMP,GX_FALSE);
	//GX_InitTexObjLOD(&texObj, GX_NEAR, GX_NEAR, 0.0f, 0.0f, 0.0f, 0, 0, GX_ANISO_1);
	GX_LoadTexObj(&texObj, GX_TEXMAP0);

	GX_SetTevOp (GX_TEVSTAGE0, GX_MODULATE);
  	GX_SetVtxDesc (GX_VA_TEX0, GX_DIRECT);

	Mtx m,m1,m2, mv;
	width *=.5;
	height*=.5;
	guMtxIdentity (m1);
	guMtxScaleApply(m1,m1,scaleX,scaleY,1.0);
	Vector axis =(Vector) {0 , 0, 1 };
	guMtxRotAxisDeg (m2, &axis, degrees);
	guMtxConcat(m2,m1,m);

	guMtxTransApply(m,m, xpos+width,ypos+height,0);
	guMtxConcat (GXmodelView2D, m, mv);
	GX_LoadPosMtxImm (mv, GX_PNMTX0);
	
	GX_Begin(GX_QUADS, GX_VTXFMT0,4);
  	GX_Position3f32(-width, -height,  0);
  	GX_Color4u8(0xFF,0xFF,0xFF,alpha);
  	GX_TexCoord2f32(0, 0);
  
  	GX_Position3f32(width, -height,  0);
 	GX_Color4u8(0xFF,0xFF,0xFF,alpha);
  	GX_TexCoord2f32(1, 0);
  
  	GX_Position3f32(width, height,  0);
	GX_Color4u8(0xFF,0xFF,0xFF,alpha);
  	GX_TexCoord2f32(1, 1);
  
  	GX_Position3f32(-width, height,  0);
	GX_Color4u8(0xFF,0xFF,0xFF,alpha);
  	GX_TexCoord2f32(0, 1);
	GX_End();
	GX_LoadPosMtxImm (GXmodelView2D, GX_PNMTX0);

	GX_SetTevOp (GX_TEVSTAGE0, GX_PASSCLR);
  	GX_SetVtxDesc (GX_VA_TEX0, GX_NONE);

}

inline void GRRLIB_DrawTile(f32 xpos, f32 ypos, u16 width, u16 height, u8 data[], float degrees, float scaleX, f32 scaleY, u8 alpha, f32 frame,f32 maxframe ){
GXTexObj texObj;
f32 s1= frame/maxframe;
f32 s2= (frame+1)/maxframe;
f32 t1=0;
f32 t2=1;
	
	GX_InitTexObj(&texObj, data, width*maxframe,height, GX_TF_RGBA8,GX_CLAMP, GX_CLAMP,GX_FALSE);
	GX_InitTexObjLOD(&texObj, GX_NEAR, GX_NEAR, 0.0f, 0.0f, 0.0f, 0, 0, GX_ANISO_1);
	GX_LoadTexObj(&texObj, GX_TEXMAP0);

	GX_SetTevOp (GX_TEVSTAGE0, GX_MODULATE);
  	GX_SetVtxDesc (GX_VA_TEX0, GX_DIRECT);

	Mtx m,m1,m2, mv;
	width *=.5;
	height*=.5;
	guMtxIdentity (m1);
	guMtxScaleApply(m1,m1,scaleX,scaleY,1.0);
	Vector axis =(Vector) {0 , 0, 1 };
	guMtxRotAxisDeg (m2, &axis, degrees);
	guMtxConcat(m2,m1,m);
	guMtxTransApply(m,m, xpos+width,ypos+height,0);
	guMtxConcat (GXmodelView2D, m, mv);
	GX_LoadPosMtxImm (mv, GX_PNMTX0);
	GX_Begin(GX_QUADS, GX_VTXFMT0,4);
  	GX_Position3f32(-width, -height,  0);
  	GX_Color4u8(0xFF,0xFF,0xFF,alpha);
  	GX_TexCoord2f32(s1, t1);
  
  	GX_Position3f32(width, -height,  0);
 	GX_Color4u8(0xFF,0xFF,0xFF,alpha);
  	GX_TexCoord2f32(s2, t1);
  
  	GX_Position3f32(width, height,  0);
	GX_Color4u8(0xFF,0xFF,0xFF,alpha);
  	GX_TexCoord2f32(s2, t2);
  
  	GX_Position3f32(-width, height,  0);
	GX_Color4u8(0xFF,0xFF,0xFF,alpha);
  	GX_TexCoord2f32(s1, t2);
	GX_End();
	GX_LoadPosMtxImm (GXmodelView2D, GX_PNMTX0);

	GX_SetTevOp (GX_TEVSTAGE0, GX_PASSCLR);
  	GX_SetVtxDesc (GX_VA_TEX0, GX_NONE);

}

inline void GRRLIB_DrawChar(f32 xpos, f32 ypos, u16 width, u16 height, u8 data[], float degrees, float scaleX, f32 scaleY, f32 frame,f32 maxframe, GXColor c ){
GXTexObj texObj;
f32 s1= frame/maxframe;
f32 s2= (frame+1)/maxframe;
f32 t1=0;
f32 t2=1;
	
	GX_InitTexObj(&texObj, data, width*maxframe,height, GX_TF_RGBA8,GX_CLAMP, GX_CLAMP,GX_FALSE);
	GX_InitTexObjLOD(&texObj, GX_NEAR, GX_NEAR, 0.0f, 0.0f, 0.0f, 0, 0, GX_ANISO_1);
	GX_LoadTexObj(&texObj, GX_TEXMAP0);

	GX_SetTevOp (GX_TEVSTAGE0, GX_MODULATE);
  	GX_SetVtxDesc (GX_VA_TEX0, GX_DIRECT);

	Mtx m,m1,m2, mv;
	width *=.5;
	height*=.5;
	guMtxIdentity (m1);
	guMtxScaleApply(m1,m1,scaleX,scaleY,1.0);
	Vector axis =(Vector) {0 , 0, 1 };
	guMtxRotAxisDeg (m2, &axis, degrees);
	guMtxConcat(m2,m1,m);
	guMtxTransApply(m,m, xpos+width,ypos+height,0);
	guMtxConcat (GXmodelView2D, m, mv);
	GX_LoadPosMtxImm (mv, GX_PNMTX0);
	GX_Begin(GX_QUADS, GX_VTXFMT0,4);
  	GX_Position3f32(-width, -height,  0);
  	GX_Color4u8(c.r,c.g,c.b,c.a);
  	GX_TexCoord2f32(s1, t1);
  
  	GX_Position3f32(width, -height,  0);
 	GX_Color4u8(c.r,c.g,c.b,c.a);
  	GX_TexCoord2f32(s2, t1);
  
  	GX_Position3f32(width, height,  0);
	GX_Color4u8(c.r,c.g,c.b,c.a);
  	GX_TexCoord2f32(s2, t2);
  
  	GX_Position3f32(-width, height,  0);
	GX_Color4u8(c.r,c.g,c.b,c.a);
  	GX_TexCoord2f32(s1, t2);
	GX_End();
	GX_LoadPosMtxImm (GXmodelView2D, GX_PNMTX0);

	GX_SetTevOp (GX_TEVSTAGE0, GX_PASSCLR);
  	GX_SetVtxDesc (GX_VA_TEX0, GX_NONE);

}

void GRRLIB_Printf(f32 xpos, f32 ypos, u8 data[], u32 color, f32 zoom, char *text,...){
int i ;
char tmp[1024];
int size=0;

va_list argp;
va_start(argp, text);
vsprintf(tmp, text, argp);
va_end(argp);

size = strlen(tmp);
GXColor col = GRRLIB_Splitu32(color);
	for(i=0;i<strlen(tmp);i++){
		u8 c = tmp[i];
		GRRLIB_DrawChar(xpos+i*8*zoom, ypos, 8, 8, data, 0, zoom, zoom, c,128, col );
	}
}

void GRRLIB_GXEngine(Vector v[], GXColor c[], long n,u8 fmt){
   int i=0;	

	GX_Begin(fmt, GX_VTXFMT0,n);
	for(i=0;i<n;i++){
  		GX_Position3f32(v[i].x, v[i].y,  v[i].z);
  		GX_Color4u8(c[i].r, c[i].g, c[i].b, c[i].a);
  	}
	GX_End();
}

GXColor GRRLIB_Splitu32(u32 color){
   u8 a,r,g,b;

	a = (color >> 24) & 0xFF; 
	r = (color >> 16) & 0xFF; 
	g = (color >> 8) & 0xFF; 
	b = (color) & 0xFF; 

	return (GXColor){r,g,b,a};
}


void GRRLIB_Widescreen(bool wd_on) {
	// This is meant to fix widescreen TVs
	if (wd_on) {
		rmode->viWidth = 678; 
		rmode->viXOrigin = (VI_MAX_WIDTH_PAL - 678) / 2;
	}
	else {
		rmode->viWidth = 640; 
		rmode->viXOrigin = (VI_MAX_WIDTH_PAL - 640) / 2;
	}

    VIDEO_Configure (rmode);
}

//********************************************************************************************
void GRRLIB_InitVideo () {

	rmode = VIDEO_GetPreferredMode(NULL);
	
	if(CONF_GetAspectRatio())
		rmode->viWidth = 678;

	VIDEO_Configure (rmode);
	xfb[0] = (u32 *)MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	xfb[1] = (u32 *)MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

	VIDEO_SetNextFramebuffer(xfb[fb]);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();

	gp_fifo = (u8 *) memalign(32,DEFAULT_FIFO_SIZE);
}

void GRRLIB_Start(){
   
   f32 yscale;
   u32 xfbHeight;
   Mtx perspective;

	GX_Init (gp_fifo, DEFAULT_FIFO_SIZE);

	// clears the bg to color and clears the z buffer
	GXColor background = { 0, 0, 0, 0xff };
	GX_SetCopyClear (background, 0x00ffffff);

	// other gx setup
	yscale = GX_GetYScaleFactor(rmode->efbHeight,rmode->xfbHeight);
	xfbHeight = GX_SetDispCopyYScale(yscale);
	GX_SetScissor(0,0,rmode->fbWidth,rmode->efbHeight);
	GX_SetDispCopySrc(0,0,rmode->fbWidth,rmode->efbHeight);
	GX_SetDispCopyDst(rmode->fbWidth,xfbHeight);
	GX_SetCopyFilter(rmode->aa,rmode->sample_pattern,GX_TRUE,rmode->vfilter);
	GX_SetFieldMode(rmode->field_rendering,((rmode->viHeight==2*rmode->xfbHeight)?GX_ENABLE:GX_DISABLE));

	if (rmode->aa)
		GX_SetPixelFmt(GX_PF_RGB565_Z16, GX_ZC_LINEAR);
	else
		GX_SetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);

	GX_SetDispCopyGamma(GX_GM_1_0);
 

	// setup the vertex descriptor
	// tells the flipper to expect direct data
	GX_ClearVtxDesc();
		GX_InvVtxCache ();
		GX_InvalidateTexAll();

	GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc (GX_VA_CLR0, GX_DIRECT);


		GX_SetVtxAttrFmt (GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
		GX_SetVtxAttrFmt (GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
		GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
		GX_SetZMode (GX_FALSE, GX_LEQUAL, GX_TRUE);

	GX_SetNumChans(1);
	GX_SetNumTexGens(1);
	GX_SetTevOp (GX_TEVSTAGE0, GX_PASSCLR);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
	GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);

	guMtxIdentity(GXmodelView2D);
	guMtxTransApply (GXmodelView2D, GXmodelView2D, 0.0F, 0.0F, -50.0F);
	GX_LoadPosMtxImm(GXmodelView2D,GX_PNMTX0);

	if(CONF_GetAspectRatio())
		guOrtho(perspective,0,479,0,719,0,300);
	else
		guOrtho(perspective,0,479,0,639,0,300);

	GX_LoadProjectionMtx(perspective, GX_ORTHOGRAPHIC);

	GX_SetViewport(0,0,rmode->fbWidth,rmode->efbHeight,0,1);
	GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
	GX_SetAlphaUpdate(GX_TRUE);
	
	GX_SetCullMode(GX_CULL_NONE);
}

void GRRLIB_Render () {
        GX_DrawDone ();

	fb ^= 1;		// flip framebuffer
	GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
	GX_SetColorUpdate(GX_TRUE);
	GX_CopyDisp(xfb[fb],GX_TRUE);
	VIDEO_SetNextFramebuffer(xfb[fb]);
 	VIDEO_Flush();
 	VIDEO_WaitVSync();

}

