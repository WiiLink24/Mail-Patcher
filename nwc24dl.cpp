#include "nwc24dl.h"

#include <chrono>
#include <limits>
#include <unistd.h>

#include "utils.h"
#include <string>
#include <cstring>
#include <format>
#include <iostream>
#include <ogc/lwp_watchdog.h>


constexpr char DL_LIST_PATH[] = "/shared2/wc24/nwc24dl.bin";
constexpr char ANNOUNCEMENT_URL[] = "http://mail.wiilink.ca/{}/announcement";
// We doing 1 minute intervals lmao
constexpr u32 NEXT_DOWNLOAD_IN_SECONDS = 1;

bool NWC24DL::ReadConfig() {
    File* file = ISFS_GetFile(DL_LIST_PATH);
    if (file->error_code != 0) {
        m_error = std::format("Error opening mail config file. Error code: {}", file->error_code);
        return false;
    }

    m_data = *(static_cast<DLList*>(file->data));
    return true;
}

bool NWC24DL::WriteConfig() const {
    s32 fd = ISFS_Open(DL_LIST_PATH, ISFS_OPEN_WRITE);
    if (fd < 0) {
        printf("Error opening file at %s\n", DL_LIST_PATH);
        return false;
    }

    s32 ret = ISFS_Write(fd, &m_data, sizeof(m_data));
    if (ret < 0) {
        printf("Error writing file at %s\n", DL_LIST_PATH);
        return false;
    }

    ret = ISFS_Close(fd);
    if (ret < 0) {
        printf("Error closing file at %s\n", DL_LIST_PATH);
        return false;
    }

    return true;
}

std::string_view NWC24DL::GetError() const {
    return m_error;
}

bool NWC24DL::AddAnnouncementEntry() {
    // Find an empty record.
    u16 record_idx = MAX_ENTRIES;
    // We must start at 8 as the ones below that are observed to be reserved.
    for (u16 i = 0; i < MAX_ENTRIES; i++) {
        if (m_data.records[i].low_title_id == 0) {
            record_idx = i;
            break;
        }
    }

    // No free entry. This is really bad and the Wii Menu/any application that uses WC24 should fix this for us.
    if (record_idx == MAX_ENTRIES) {
        return false;
    }

    // Record index always corresponds to an entry index.
    DLListEntry* entry = &m_data.entries[record_idx];
    entry->type = MAIL;
    entry->flags = 1 << 2;
    entry->record_flags = 100;
    entry->high_title_id = 0x48414541;
    entry->low_title_id = 0x00010002;
    entry->unknown1 = 0x48414541;
    entry->group_id = 0x3031;
    entry->remaining_downloads = 100;
    entry->dl_margin = 120;
    entry->retry_frequency = 1440;

    // Allow for multiple languages!
    const std::string url = std::format(ANNOUNCEMENT_URL, CONF_GetLanguage());
    std::strncpy(entry->dl_url, url.data(), 236);

    DLListRecord* record = &m_data.records[record_idx];
    record->low_title_id = 0x48414541;
    record->flags = 100;

    // Get UTC from console.
    s32 fd = IOS_Open("/dev/net/kd/time", 0);
    if (fd < 0) {
        std::cout << "Error opening KD Time device" << std::endl;
        return false;
    }

    // We have to set RTC in the WC24 device before we can get Unix UTC
    std::array<u32, 2> rtc{};
    __SYS_GetRTC(&rtc[0]);

    s32 err{};
    s32 ipc_err = IOS_Ioctl(fd, 0x17, rtc.data(), 8, &err, 4);
    if (ipc_err < 0 || err < 0) {
        std::cout << "Error setting RTC" << std::endl;
        std::cout << "Error code: " << err << "and " << ipc_err << std::endl;
        return false;
    }

    void* time = std::malloc(12);
    std::memset(time, 0, 12);
    ipc_err = IOS_Ioctl(fd, 0x14, nullptr, 0, time, 12);
    err = *static_cast<s32*>(time);
    if (ipc_err < 0 || err < 0) {
        std::cout << "Error setting RTC" << std::endl;
        std::cout << "Error code: " << err << "and " << ipc_err << std::endl;
        return false;
    }

    IOS_Close(fd);

    record->last_modified_timestamp = *static_cast<u64*>(time+4) / 60;
    record->next_dl_timestamp = *static_cast<u64*>(time+4) / 60 + 5;
    return WriteConfig();
}

bool NWC24DL::AnnouncementExists() {
    const std::string url = std::format(ANNOUNCEMENT_URL, CONF_GetLanguage());

    for (u16 i = 0; i < MAX_ENTRIES; i++) {
        if (std::strncmp(m_data.entries[i].dl_url, url.data(), 236) == 0) {
            // There was an issue where KD would not process the task if the next download time is 0, it will
            // never download. Fix this now.
            if (m_data.records[i].flags != 100) {
                std::memset(&m_data.records[i], 0, sizeof(DLListRecord));
                std::memset(&m_data.entries[i], 0, sizeof(DLListEntry));
                m_data.entries[i].type = UNUSED;

                // Call add announcement to finalize.
                AddAnnouncementEntry();
                std::cout << "Successfully fixed announcement entry" << std::endl;
            }

            return true;
        }

    }

    return false;
}
