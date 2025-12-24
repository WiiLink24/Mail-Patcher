#include "nwc24dl.h"

#include <limits>

#include "utils.h"
#include <string>
#include <cstring>
#include <format>
#include <iostream>
#include <bits/ostream.tcc>

constexpr char DL_LIST_PATH[] = "/shared2/wc24/nwc24dl.bin";
constexpr char ANNOUNCEMENT_URL[] = "http://mail.wiilink.ca/{}/announcement";

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
    for (u16 i = 8; i < MAX_ENTRIES; i++) {
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
    entry->record_flags = 0xc0;
    entry->high_title_id = 0x48414541;
    entry->low_title_id = 0x00010002;
    entry->unknown1 = 0x48414541;
    entry->group_id = 0x3031;
    entry->remaining_downloads = 0x5a0;
    entry->dl_margin = 240;
    entry->retry_frequency = 1440;

    // Allow for multiple languages!
    const std::string url = std::format(ANNOUNCEMENT_URL, CONF_GetLanguage());
    std::strncpy(entry->dl_url, url.data(), 236);

    DLListRecord* record = &m_data.records[record_idx];
    record->low_title_id = 0x48414541;
    record->flags = 0xc0;

    return WriteConfig();
}

bool NWC24DL::AnnouncementExists() const {
    const std::string url = std::format(ANNOUNCEMENT_URL, CONF_GetLanguage());

    for (const DLListEntry& entry : m_data.entries) {
        // Only compare the "http://mail.wiilink.ca" part
        if (std::strncmp(entry.dl_url, url.data(), 236) == 0) {
            return true;
        }
    }

    return false;
}
