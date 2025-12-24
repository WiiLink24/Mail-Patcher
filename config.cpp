#include "config.h"
#include <cstring>
#include <format>
#include <iostream>
#include "utils.h"

constexpr char CONFIG_PATH[] = "/shared2/wc24/nwc24msg.cfg";

bool NWC24Config::ReadConfig()
{
  File* file = ISFS_GetFile(CONFIG_PATH);
  if (file->error_code != 0) {
    m_error = std::format("Error opening mail config file. Error code: {}", file->error_code);
    return false;
  }

  m_data = *(static_cast<ConfigData*>(file->data));
  return true;
}

bool NWC24Config::WriteConfig()
{
  // Recalculate checksum
  m_data.checksum = CalculateNwc24ConfigChecksum();

  s32 fd = ISFS_Open(CONFIG_PATH, ISFS_OPEN_WRITE);
  if (fd < 0) {
    printf("Error opening file at %s\n", CONFIG_PATH);
    return false;
  }

  s32 ret = ISFS_Write(fd, &m_data, sizeof(m_data));
  if (ret < 0) {
    printf("Error writing file at %s\n", CONFIG_PATH);
    return false;
  }

  ret = ISFS_Close(fd);
  if (ret < 0) {
    printf("Error closing file at %s\n", CONFIG_PATH);
    return false;
  }

  return true;
}

u32 NWC24Config::CalculateNwc24ConfigChecksum() const
{
  const u32* ptr = reinterpret_cast<const u32*>(&m_data);
  u32 sum = 0;

  for (int i = 0; i < 0xFF; ++i)
  {
    sum += *ptr++;
  }

  return sum;
}

std::string_view NWC24Config::GetError() const {
  return m_error;
}


s32 NWC24Config::CheckNwc24Config() const
{
  // 'WcCf' magic
  if (m_data.magic != 0x57634366)
  {
    return -14;
  }

  const u32 checksum = CalculateNwc24ConfigChecksum();
  if (m_data.checksum != checksum)
  {
    return -14;
  }

  if (m_data.id_generation > 0x1F)
  {
    return -14;
  }

  return 0;
}


void NWC24Config::SetChecksum(u32 checksum)
{
  m_data.checksum = checksum;
}

NWC24CreationStage NWC24Config::CreationStage() const
{
  return NWC24CreationStage(u32(m_data.creation_stage));
}

void NWC24Config::SetCreationStage(NWC24CreationStage creation_stage)
{
  m_data.creation_stage = NWC24CreationStage(u32(creation_stage));
}

std::string NWC24Config::GetEmail() {
  return m_data.email;
}

void NWC24Config::SetEmail(std::string_view email)
{
  std::strncpy(m_data.email, email.data(), MAX_EMAIL_LENGTH);
  m_data.email[MAX_EMAIL_LENGTH - 1] = '\0';
}

void NWC24Config::SetAccountURL()
{
  std::strncpy(m_data.http_urls[0], "http://mtw.rc24.xyz/cgi-bin/account.cgi", MAX_URL_LENGTH);
  m_data.http_urls[0][MAX_URL_LENGTH - 1] = '\0';
}
void NWC24Config::SetURLs()
{
  std::strncpy(m_data.http_urls[0], "http://mtw.rc24.xyz/cgi-bin/account.cgi", MAX_URL_LENGTH);
  m_data.http_urls[0][MAX_URL_LENGTH - 1] = '\0';

  std::strncpy(m_data.http_urls[1], "http://mtw.rc24.xyz/cgi-bin/check.cgi", MAX_URL_LENGTH);
  m_data.http_urls[1][MAX_URL_LENGTH - 1] = '\0';

  std::strncpy(m_data.http_urls[2], "http://mtw.rc24.xyz/cgi-bin/receive.cgi", MAX_URL_LENGTH);
  m_data.http_urls[2][MAX_URL_LENGTH - 1] = '\0';

  std::strncpy(m_data.http_urls[3], "http://mtw.rc24.xyz/cgi-bin/delete.cgi", MAX_URL_LENGTH);
  m_data.http_urls[3][MAX_URL_LENGTH - 1] = '\0';

  std::strncpy(m_data.http_urls[4], "http://mtw.rc24.xyz/cgi-bin/send.cgi", MAX_URL_LENGTH);
  m_data.http_urls[4][MAX_URL_LENGTH - 1] = '\0';
}

u64 NWC24Config::GetFriendCode() const
{
  return m_data.nwc24_id;
}
