#include <iostream>
#include <gccore.h>
#include "config.h"

static int DisplayError(NWC24Config& config, std::string message, int error_code)
{
  // Reset back to registered or WiiConnect24 will completely break.
  config.SetCreationStage(NWC24CreationStage::Registered);
  config.WriteConfig();

  std::cout << message << std::endl;
  printf("Error Code: %d\n", error_code);
  std::cout << "Please doing the WiiLink Discord for support." << std::endl;
  std::cout << "Server Link: https://discord.gg/reqUMqxu8D" << std::endl;
  std::cout << std::endl << "Press the HOME Button to exit." << std::endl;
  return error_code;
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

  IOS_ReloadIOS(80);
  s32 fd = IOS_Open("/dev/net/kd/request", IPC_OPEN_READ);
  if (fd < 0)
    return DisplayError(config, "An Error has occurred while opening the WC24 device.", fd);

  void *io_buf = std::malloc(32);

  s32 ret = IOS_Ioctl(fd, 0x10, nullptr, 0, io_buf, 32);
  if (ret < 0)
    return DisplayError(config, "A fatal error has occurred in the WC24 device.", ret);

  const s32 response = reinterpret_cast<s32 *>(io_buf)[0];
  if (response != 0)
    return DisplayError(config, "An error has occurred in the patching process.", response);

  // Now that we successfully added to the server, update the URLs and email.
  // We have to reload the config as KD would have flushed the new mlchkid and password.
  config = NWC24Config();
  config.SetEmail("@mail.wiilink24.com");
  config.SetURLs();
  config.SetCreationStage(NWC24CreationStage::Registered);
  config.WriteConfig();
  std::cout << "Patching succeeded! You can now use the WiiLink Mail Service!" << std::endl;
  std::cout << "Thank you for installing WiiLink!" << std::endl;
  std::cout << std::endl << "Press the HOME Button to exit." << std::endl;

  return 0;
}
