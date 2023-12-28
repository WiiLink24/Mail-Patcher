#include <iostream>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include "patcher.h"

static void* xfb = nullptr;
static GXRModeObj* rmode = nullptr;

int main() {
  VIDEO_Init();
  ISFS_Initialize();

  rmode = VIDEO_GetPreferredMode(nullptr);
  xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
  console_init(xfb, 20, 20, rmode->fbWidth, rmode->xfbHeight, rmode->fbWidth * VI_DISPLAY_PIX_SZ);
  VIDEO_Configure(rmode);
  VIDEO_SetNextFramebuffer(xfb);
  VIDEO_SetBlack(FALSE);
  VIDEO_Flush();
  VIDEO_WaitVSync();
  if (rmode->viTVMode & VI_NON_INTERLACE) VIDEO_WaitVSync();

  std::cout << std::endl;
  std::cout << std::endl;
  std::cout << std::endl;
  std::cout << "RiiConnect24 Mail Patcher - (c) 2023 RiiConnect24" << std::endl;
  std::cout << "v2.0" << std::endl;
  std::cout << std::endl;
  std::cout << "Patching..." << std::endl;
  std::cout << std::endl;
  Patcher();

  CONF_Init();
  WPAD_Init();

  while (true) {
    WPAD_ScanPads();
    u32 pressed = WPAD_ButtonsDown(0);
    if (pressed & WPAD_BUTTON_HOME)
      exit(0);
    VIDEO_WaitVSync();
  }

  return 0;
}
