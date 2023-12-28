#include <iostream>
#include <vector>
#include <gccore.h>
#include "config.h"
#include "errors.h"

static int DisplayError(NWC24Config& config, std::string_view message, s32 error_code)
{
  // Reset back to registered or WiiConnect24 will completely break.
  config.SetCreationStage(NWC24CreationStage::Registered);
  config.WriteConfig();

  std::cout << message << std::endl;
  std::cout << "Error Code: " << error_code << std::endl;

  // Get a pre-made message if applicable
  s32 error_code_start = error_code;
  while (error_code_start < -9 || error_code_start > 9)
    error_code_start /= 10;

  if (error_code_start == -5)
    error_code = error_code_start;

  if (auto found = error_descriptions.find(error_code); found != error_descriptions.end())
    std::cout << found->second << std::endl;

  std::cout << "Wii Number: " << config.GetFriendCode() << std::endl << std::endl;
  std::cout << "Please join the RiiConnect24 Discord for support." << std::endl;
  std::cout << "Server Link: https://discord.gg/rc24" << std::endl << std::endl ;
  std::cout << "Press the HOME Button to exit." << std::endl;
  return error_code;
}

static s32 GetWC24Error(s32 wc24_fd, NWC24Config& config)
{
  void *io_buf = std::malloc(256);
  s32 ret = IOS_Ioctl(wc24_fd, 0x1E, nullptr, 0, io_buf, 256);
  if (ret < 0)
    return DisplayError(config, "A fatal error has occurred in the WC24 device.", ret);

  return reinterpret_cast<s32 *>(io_buf)[2];
}

static s32 GetSystemMenuIOS() {
  s32 ret;
  u32 view_size{};

  ret = ES_GetTMDViewSize(0x100000002LL, &view_size);
  if (ret < 0)
    return ret;

  u8* buffer = reinterpret_cast<u8*>(aligned_alloc(32, view_size));
  tmd_view* tmd = reinterpret_cast<tmd_view*>(buffer);
  ret = ES_GetTMDView(0x100000002LL, buffer, view_size);
  if (ret < 0)
    return ret;

  return static_cast<s32>(tmd->sys_version);

}

int Patcher()
{
  // This is hacky but the easiest way to go about patching.
  // RiiConnect24's patcher has to handle parsing everything which is a bit yuck.
  // We can utilize the Request Register User ID ioctl within KD.
  // Before doing that however, we must set the registration flag to `Generated` then reload IOS.
  // We must also set the account URL to ours.
  NWC24Config config = NWC24Config();
  config.SetCreationStage(NWC24CreationStage::Generated);
  config.SetAccountURL();
  config.WriteConfig();

  s32 IOS = GetSystemMenuIOS();
  if (IOS < 0)
    return DisplayError(config, "Unable to retrieve the Wii Menu's IOS.", IOS);

  IOS_ReloadIOS(IOS);

  s32 fd = IOS_Open("/dev/net/kd/request", IPC_OPEN_READ);
  if (fd < 0)
    return DisplayError(config, "An Error has occurred while opening the WC24 device.", fd);

  void *io_buf = std::malloc(32);

  s32 ret = IOS_Ioctl(fd, 0x10, nullptr, 0, io_buf, 32);
  if (ret < 0)
    return DisplayError(config, "A fatal error has occurred in the WC24 device.", ret);

  const s32 response = reinterpret_cast<s32 *>(io_buf)[0];
  if (response != 0)
  {
    const s32 wc24_error = GetWC24Error(fd, config);
    return DisplayError(config, "An error has occurred in the patching process.", wc24_error);
  }

  // Now that we successfully added to the server, update the URLs and email.
  // We have to reload the config as KD would have flushed the new mlchkid and password.
  config = NWC24Config();
  config.SetEmail("@rc24.xyz");
  config.SetURLs();
  config.SetCreationStage(NWC24CreationStage::Registered);
  config.WriteConfig();
  std::cout << "Patching succeeded! You can now use the RiiConnect24 Mail Service!" << std::endl;
  std::cout << "Thank you for installing RiiConnect24!" << std::endl;
  std::cout << std::endl << "Press the HOME Button to exit." << std::endl;

  return 0;
}
