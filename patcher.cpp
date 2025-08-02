#include <iostream>
#include <format>
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

  s64 wiino = config.GetFriendCode();
  // s64 wiino = 595'0972'0461'0244;

  std::cout << "Wii Number: " << std::format("{:04}-{:04}-{:04}-{:04}", (wiino / (u64)1e+12) % 10000, (wiino / (u64)1e+8) % 10000, (wiino / (u64)1e+4) % 10000, wiino % 10000) << std::endl << std::endl;
  std::cout << "Please join the WiiLink Discord for support." << std::endl;
  std::cout << "Server Link: https://discord.gg/wiilink" << std::endl << std::endl ;
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

  tmd_view* view = reinterpret_cast<tmd_view*>(aligned_alloc(32, view_size));
  ret = ES_GetTMDView(0x100000002LL, view, view_size);
  if (ret < 0)
    return ret;

  return static_cast<s32>(view->sys_version);

}

static int TestRegistration(NWC24Config& config)
{
  s32 fd = IOS_Open("/dev/net/kd/request", IPC_OPEN_READ);
  if (fd < 0)
    return DisplayError(config, "An error has occurred while opening the WC24 device.", fd);

  s32 ret;
  int tries = 5;
  do
  {
    s32 outbuf[4] { /* ret, isNewMail, mailSpan, socketError */ };
    ret = IOS_Ioctl(fd, 10, nullptr, 0, outbuf, 0x10);
    if (ret < 0)
    {
      DisplayError(config, "A fatal error has occurred in the WC24 device.", ret);
      break;
    }

    // std::cout << std::format("KD_CheckMail() ret={} soError={}", outbuf[0], outbuf[3]) << std::endl;

    if (outbuf[0] == -33)
    {
      // std::cout << std::format("Network not available, retrying... ({}) *", outbuf[3]) << std::endl;
      if (!tries--)
      {
        ret = DisplayError(config, "An error has occured during the patching process.", GetWC24Error(fd, config));
        break;
      }
      continue;
    }
    else
    {
      if (outbuf[0] == 0) // OK!
      {
        ret = 1;
      }
    }
  } while (0);

  IOS_Close(fd);
  return ret;
}

int Patcher()
{
  // This is hacky but the easiest way to go about patching.
  // We can utilize the Request Register User ID ioctl within KD.
  // Before doing that however, we must set the registration flag to `Generated` then reload IOS.
  // We must also set the account URL to ours.
  NWC24Config config = NWC24Config();
  if (config.GetEmail() == "@rc24.xyz")
  {
    // std::cout << "Email is already rc24.xyz, check mail now...." << std::endl;

    int ret = TestRegistration(config);
    if (ret == 1)
    {
      std::cout << "Your Wii is already registered for WiiLink Mail." << std::endl << std::endl;
      std::cout << "Press the HOME Button to exit." << std::endl;
      return 0;
    }
    else if (ret < 0)
    {
      return ret;
    }
  }

  config.SetCreationStage(NWC24CreationStage::Generated);
  config.SetAccountURL();
  config.WriteConfig();

  s32 IOS = GetSystemMenuIOS();
  if (IOS < 0)
    return DisplayError(config, "Unable to retrieve the Wii Menu's IOS.", IOS);

  IOS_ReloadIOS(IOS);

  s32 fd = IOS_Open("/dev/net/kd/request", IPC_OPEN_READ);
  if (fd < 0)
    return DisplayError(config, "An error has occurred while opening the WC24 device.", fd);

  s32 ret;
  int tries = 5;
  do
  {
    s32 outbuf[2];
    ret = IOS_Ioctl(fd, 0x10, nullptr, 0, outbuf, 0x8);
    if (ret < 0)
      return DisplayError(config, "A fatal error has occurred in the WC24 device.", ret);

    // std::cout << std::format("KD_CreateAccount() ret={} soError={}", outbuf[0], outbuf[1]) << std::endl;

    ret = outbuf[0];
    if (ret == -33)
    {
      if (!tries--)
        break;

      // std::cout << std::format("Network not available, retrying... ({})", outbuf[1]) << std::endl;
      continue;
    }
  } while (0);

  if (ret != 0)
    return DisplayError(config, "An error has occurred in the patching process.", GetWC24Error(fd, config));

  // Now that we successfully added to the server, update the URLs and email.
  // We have to reload the config as KD would have flushed the new mlchkid and password.
  config = NWC24Config();
  config.SetEmail("@rc24.xyz");
  config.SetURLs();
  config.SetCreationStage(NWC24CreationStage::Registered);
  config.WriteConfig();
  std::cout << "Patching succeeded! You can now use the WiiLink Mail Service!" << std::endl;
  std::cout << "Thank you for installing WiiLink!" << std::endl;
  std::cout << std::endl << "Press the HOME Button to exit." << std::endl;

  return 0;
}
