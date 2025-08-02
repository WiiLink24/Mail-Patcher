#pragma once

#include <string>
#include <gccore.h>

enum class NWC24CreationStage : u32
{
    Initial = 0,
    Generated = 1,
    Registered = 2
};

class NWC24Config final
{
public:
    explicit NWC24Config();
    void ReadConfig();
    void WriteConfig();
    void WriteConfigToPath(const std::string& filepath);
    void ResetConfig();

    u32 CalculateNwc24ConfigChecksum() const;
    s32 CheckNwc24Config() const;

    u64 GetFriendCode() const;

    u32 IdGen() const;
    void SetIdGen(u32 id_generation);
    void IncrementIdGen();

    u32 Checksum() const;
    void SetChecksum(u32 checksum);

    NWC24CreationStage CreationStage() const;
    void SetCreationStage(NWC24CreationStage creation_stage);

    bool IsCreated() const { return CreationStage() == NWC24CreationStage::Initial; }
    bool IsGenerated() const { return CreationStage() == NWC24CreationStage::Generated; }
    bool IsRegistered() const { return CreationStage() == NWC24CreationStage::Registered; }

    std::string GetEmail();
    void SetEmail(std::string_view email);
    void SetAccountURL();
    void SetURLs();

private:
    enum
    {
        URL_COUNT = 0x05,
        MAX_URL_LENGTH = 0x80,
        MAX_EMAIL_LENGTH = 0x40,
        MAX_PASSWORD_LENGTH = 0x20,
        MAX_MLCHKID_LENGTH = 0x24,
    };

#pragma pack(push, 1)
    struct ConfigData final
    {
        u32 magic;    // 'WcCf' 0x57634366
        u32 version;  // must be 8
        u64 nwc24_id;
        u32 id_generation;
        NWC24CreationStage creation_stage;
        char email[MAX_EMAIL_LENGTH];
        char paswd[MAX_PASSWORD_LENGTH];
        char mlchkid[MAX_MLCHKID_LENGTH];
        char http_urls[URL_COUNT][MAX_URL_LENGTH];
        u8 reserved[0xDC];
        u32 enable_booting;
        u32 checksum;
    };
#pragma pack(pop)

    ConfigData m_data;
};
