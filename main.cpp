#include <iostream>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <wiisocket.h>
#include "patcher.h"
#include <unistd.h>

static void* xfb = NULL;
static GXRModeObj* rmode = NULL;

int main() {
  VIDEO_Init();

  rmode = VIDEO_GetPreferredMode(NULL);
  xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
  console_init(xfb, 20, 20, rmode->fbWidth, rmode->xfbHeight, rmode->fbWidth * VI_DISPLAY_PIX_SZ);
  VIDEO_Configure(rmode);
  VIDEO_SetNextFramebuffer(xfb);
  VIDEO_SetBlack(FALSE);
  VIDEO_Flush();
  VIDEO_WaitVSync();
  if (rmode->viTVMode & VI_NON_INTERLACE) VIDEO_WaitVSync();

  WPAD_Init();
  ISFS_Initialize();
  CONF_Init();

  std::cout << std::endl;
  std::cout << std::endl;
  std::cout << std::endl;
  std::cout << "WiiLink Mail Patcher - (c) 2023 WiiLink" << std::endl;
  std::cout << "v0.1" << std::endl;
  std::cout << std::endl;
  std::cout << "Patching..." << std::endl;
  std::cout << std::endl;
  Patcher();

  while (1) {
    WPAD_ScanPads();
    u32 pressed = WPAD_ButtonsDown(0);

    if (pressed & WPAD_BUTTON_HOME) exit(0);

    VIDEO_WaitVSync();
  }

  return 0;
}
