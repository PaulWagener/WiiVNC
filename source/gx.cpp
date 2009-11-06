#include "gx.h"
	
void *xfb[2];
int fb = 0;
GXRModeObj *rmode;
int gxfoo = 0;
int GX_Initialize()
{
	Mtx GXmodelView2D;
	f32 yscale;
    u32 xfbHeight;
    Mtx44 perspective;
	static void  *gp_fifo = NULL;
	
    // Initialise the video subsystem
	VIDEO_Init();
    VIDEO_SetBlack(true);  // Disable video output during initialisation

    // Grab a pointer to the video mode attributes
    if ( !(rmode = VIDEO_GetPreferredMode(NULL)) )  return -1 ;

    // Video Mode Correction
    switch (rmode->viTVMode) {
        case VI_DEBUG_PAL:  // PAL 50hz 576i
            //rmode = &TVPal574IntDfScale;
            rmode = &TVPal528IntDf; // BC ...this is still wrong, but "less bad" for now
            break;
    }

    // 16:9 and 4:3 Screen Adjustment
    if (CONF_GetAspectRatio() == CONF_ASPECT_16_9) {
        rmode->viWidth = 678;
        rmode->viXOrigin = (VI_MAX_WIDTH_NTSC - 678)/2;  // This probably needs to consider PAL
    } else {    // 4:3
        rmode->viWidth = 672;
        rmode->viXOrigin = (VI_MAX_WIDTH_NTSC - 672)/2;
    }

    // --
    VIDEO_Configure(rmode);

    // Get some memory to use for a "double buffered" frame buffer
    if ( !(xfb[0] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode))) )  return -1 ;
    if ( !(xfb[1] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode))) )  return -1 ;

    VIDEO_SetNextFramebuffer(xfb[fb]);  // Choose a frame buffer to start with

    VIDEO_Flush();                      // flush the frame to the TV
    VIDEO_WaitVSync();                  // Wait for the TV to finish updating
    // If the TV image is interlaced it takes two passes to display the image
    if (rmode->viTVMode & VI_NON_INTERLACE)  VIDEO_WaitVSync() ;

	#define DEFAULT_FIFO_SIZE (256 * 1024) /**< GX fifo buffer size. */

    // The FIFO is the buffer the CPU uses to send commands to the GPU
    if ( !(gp_fifo = memalign(32, DEFAULT_FIFO_SIZE)) )  return -1 ;
    memset(gp_fifo, 0, DEFAULT_FIFO_SIZE);
    GX_Init(gp_fifo, DEFAULT_FIFO_SIZE);

    // Clear the background to opaque black and clears the z-buffer
    GX_SetCopyClear((GXColor){ 0, 0, 0, 0xff }, GX_MAX_Z24);

    // Other GX setup
    yscale    = GX_GetYScaleFactor(rmode->efbHeight, rmode->xfbHeight);
    xfbHeight = GX_SetDispCopyYScale(yscale);
    GX_SetDispCopySrc(0, 0, rmode->fbWidth, rmode->efbHeight);
    GX_SetDispCopyDst(rmode->fbWidth, xfbHeight);
    GX_SetCopyFilter(rmode->aa, rmode->sample_pattern, GX_TRUE, rmode->vfilter);
    GX_SetFieldMode(rmode->field_rendering, ((rmode->viHeight == 2 * rmode->xfbHeight) ? GX_ENABLE : GX_DISABLE));

    if (rmode->aa)  GX_SetPixelFmt(GX_PF_RGB565_Z16, GX_ZC_LINEAR) ;  // Set 16 bit RGB565
    else            GX_SetPixelFmt(GX_PF_RGB8_Z24  , GX_ZC_LINEAR) ;  // Set 24 bit Z24

    GX_SetDispCopyGamma(GX_GM_1_0);

    // Setup the vertex descriptor
    GX_ClearVtxDesc();      // clear all the vertex descriptors
    GX_InvVtxCache();       // Invalidate the vertex cache
    GX_InvalidateTexAll();  // Invalidate all textures

    // Tells the flipper to expect direct data
    GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);
    GX_SetVtxDesc(GX_VA_POS,  GX_DIRECT);
    GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);

    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS,  GX_POS_XYZ,  GX_F32, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST,   GX_F32, 0);
    // Colour 0 is 8bit RGBA format
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);

    GX_SetNumChans(1);    // colour is the same as vertex colour
    GX_SetNumTexGens(1);  // One texture exists
    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);

    guMtxIdentity(GXmodelView2D);
    guMtxTransApply(GXmodelView2D, GXmodelView2D, 0.0F, 0.0F, -100.0F);
    GX_LoadPosMtxImm(GXmodelView2D, GX_PNMTX0);

    guOrtho(perspective, 0, rmode->efbHeight, 0, rmode->fbWidth, 0, 1000.0f);
    GX_LoadProjectionMtx(perspective, GX_ORTHOGRAPHIC);

    GX_SetViewport(0, 0, rmode->fbWidth, rmode->efbHeight, 0, 1);
    GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
    GX_SetAlphaUpdate(GX_TRUE);
    GX_SetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GX_SetColorUpdate(GX_ENABLE);
    GX_SetCullMode(GX_CULL_NONE);
        GX_SetClipMode( GX_CLIP_ENABLE );
    GX_SetScissor( 0, 0, rmode->fbWidth, rmode->efbHeight );
    VIDEO_SetBlack(false);  // Enable video output
	return 0;
}

void GX_Render()
{
    GX_DrawDone();          // Tell the GX engine we are done drawing
    GX_InvalidateTexAll();

    GX_SetZMode      (GX_TRUE, GX_LEQUAL, GX_TRUE);
    GX_SetColorUpdate(GX_TRUE);
    GX_CopyDisp      (xfb[fb], GX_TRUE);

	//This code should be uncommented but somehow introduces all sorts of strange bugs.
    //fb ^= 1;  // Toggle framebuffer index

    VIDEO_SetNextFramebuffer(xfb[fb]);  // Select eXternal Frame Buffer
    VIDEO_Flush();                      // Flush video buffer to screen
    VIDEO_WaitVSync();                  // Wait for screen to update
    // Interlaced screens require two frames to update
    if (rmode->viTVMode &VI_NON_INTERLACE)  VIDEO_WaitVSync() ;
}

/**
 *	
 */
GX_Texture::GX_Texture(int newWidth, int newHeight, u8 newFormat, u8* newBuffer)
{
	width = newWidth;
	height = newHeight;
	format = newFormat;
	if(newBuffer == NULL)
		newBuffer = (u8*)memalign(32, width*height*4);

	buffer = newBuffer;
	
	GX_InitTexObj(&texObj, buffer, width, height, format, GX_CLAMP, GX_CLAMP, GX_FALSE);
}

GX_Texture::~GX_Texture()
{
	free(buffer);
}
//#include "libpng/png.h"
#include "libpng/pngu/pngu.h"
GX_Texture* GX_Texture::LoadFromPNG(const void* png) {
    PNGUPROP imgProp;
    IMGCTX ctx;

	u8* buffer = NULL;
    if(png != NULL) {
        ctx = PNGU_SelectImageFromBuffer(png);
        PNGU_GetImageProperties(ctx, &imgProp);
       	buffer = (u8*)memalign(32, imgProp.imgWidth * imgProp.imgHeight * 4);
        if(buffer != NULL) {
            PNGU_DecodeTo4x4RGBA8(ctx, imgProp.imgWidth, imgProp.imgHeight,buffer, 255);
            PNGU_ReleaseImageContext(ctx);
        }
    }
	
	GX_Texture *texture = new GX_Texture(imgProp.imgWidth, imgProp.imgHeight, GX_TF_RGBA8, buffer);
	texture->Flush();
	return texture;
}


/**
 * Draws the texture buffer on the screen
 * If no width and height are given the texture is drawn with its own width and height
 * Opacity is a value from 0 (transparent) to 255 (opaque), defaults to 255
 * TODO: implement rotation
 */
void GX_Texture::Draw(int x, int y, int drawWidth, int drawHeight, int opacity, int rotation)
{
	if(drawWidth==-1) drawWidth = width;
	if(drawHeight==-1) drawHeight = height;

	GX_LoadTexObj(&texObj, GX_TEXMAP0);
	GX_SetTevOp  (GX_TEVSTAGE0, GX_MODULATE);
	GX_SetVtxDesc(GX_VA_TEX0,   GX_DIRECT);
		
	u32 color = 0xFFFFFF00 | opacity;
	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
		GX_Position3f32(x, y, 0);
		GX_Color1u32   (color);
		GX_TexCoord2f32(0, 0);

        GX_Position3f32(x+drawWidth, y, 0);
        GX_Color1u32   (color);
        GX_TexCoord2f32(1, 0);

        GX_Position3f32(x+drawWidth, y+drawHeight, 0);
        GX_Color1u32   (color);
        GX_TexCoord2f32(1, 1);

        GX_Position3f32(x, y+drawHeight, 0);
        GX_Color1u32   (color);
        GX_TexCoord2f32(0, 1);
    GX_End();

    GX_SetTevOp  (GX_TEVSTAGE0, GX_PASSCLR);
    GX_SetVtxDesc(GX_VA_TEX0,   GX_NONE);
}

/**
 *
 */
void GX_Texture::Flush()
{
	DCFlushRange(buffer, width*height*4);
}
