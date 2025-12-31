#include <iostream>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include "patcher.h"
#include "nwc24dl.h"

static void* xfb = nullptr;
static GXRModeObj* rmode = nullptr;

[[noreturn]] void poll_home_button() {
  std::cout << std::endl << "Press the HOME Button to exit." << std::endl;

  while (true) {
    WPAD_ScanPads();
    u32 pressed = WPAD_ButtonsDown(0);
    if (pressed & WPAD_BUTTON_HOME)
      exit(0);
    VIDEO_WaitVSync();
  }
}

int main() {
  VIDEO_Init();
  ISFS_Initialize();

  rmode = VIDEO_GetPreferredMode(nullptr);
  xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
  VIDEO_ClearFrameBuffer(rmode, xfb, COLOR_BLACK);
  VIDEO_Configure(rmode);
  VIDEO_SetNextFramebuffer(xfb);
  VIDEO_SetBlack(FALSE);
  VIDEO_Flush();
  VIDEO_WaitVSync();
  if (rmode->viTVMode & VI_NON_INTERLACE) VIDEO_WaitVSync();
  CON_InitEx(rmode, 0, 0, rmode->fbWidth, rmode->xfbHeight);

  std::cout << std::endl;
  std::cout << std::endl;
  std::cout << std::endl;
  std::cout << "WiiLink Mail Patcher - (c) 2025 WiiLink" << std::endl;
  std::cout << "v2.2.1" << std::endl;
  std::cout << std::endl;
  std::cout << "Patching..." << std::endl;
  std::cout << std::endl;
  int ret = Patcher();

  // We reloaded IOS in the Patcher function, we have to init the devices here.
  CONF_Init();
  WPAD_Init();
  if (ret != 0) {
    // Error has occurred, abort.
    poll_home_button();
  }

  // We can now have the user decide if they want to opt in to the WiiLink Announcement Service.
  auto list = NWC24DL();
  bool success = list.ReadConfig();
  if (!success) {
    std::cout << list.GetError() << std::endl;
    poll_home_button();
  }

  // Check if they already have the download task added.
  if (!list.AnnouncementExists()) {
    // Prompt the user to add.
    std::cout << std::endl << "You can also opt in to the WiiLink Announcement Service!" << std::endl;
    std::cout << "We will occasionally send announcements about the service" << std::endl << "via the Wii Message Board" << std::endl;
    std::cout << "Press A to opt in, or HOME to return to the Wii Menu." << std::endl;
    while (true) {
      WPAD_ScanPads();
      u32 pressed = WPAD_ButtonsDown(0);
      if (pressed & WPAD_BUTTON_HOME) {
        exit(0);
      }
      if (pressed & WPAD_BUTTON_A) {
        success = list.AddAnnouncementEntry();
        if (success) {
          std::cout << std::endl << "Successfully opted in to the WiiLink Announcement Service!" << std::endl;
        }

        break;
      }

      VIDEO_WaitVSync();
    }
  }

  poll_home_button();
  return 0;
}
