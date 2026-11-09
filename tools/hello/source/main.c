#include <stdio.h>
#include <stdlib.h>
#include <gccore.h>

int main(int argc, char **argv)
{
	GXRModeObj *rmode;
	u32 *xfb;
	u32 i;

	VIDEO_Init();
	PAD_Init();

	rmode = VIDEO_GetPreferredMode(NULL);
	xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

	console_init(xfb, 20, 20, rmode->fbWidth, rmode->xfbHeight,
	             rmode->fbWidth * VI_DISPLAY_PIX_SZ);

	for (i = 0; i < VIDEO_GetFrameBufferSize(rmode) / sizeof(u32); i++)
		xfb[i] = 0x1D6B1DFF;

	printf("\nioQuake3-GC hello\n");

	VIDEO_Configure(rmode);
	VIDEO_SetNextFramebuffer(xfb);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if (rmode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync();

	while (1) {
		PAD_ScanPads();
		if (PAD_ButtonsDown(0) & PAD_BUTTON_START)
			exit(0);
	}

	return 0;
}
