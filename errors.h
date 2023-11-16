#pragma once

#include <map>
#include <string_view>
#include <ogcsys.h>

static const std::map<s32, std::string_view> error_descriptions = {
        {-101001, "Fatal error during setup."},
        {-101002, "Fatal error during cleanup."},
        {-110211, "Duplicate registration. If you are already registered you do not need to rerun this app."},
        {-110310, "Illegal request."},
        {-101232, "HTTP error."},
        {-101234, "Wii Number was not generated."},
        {-101239, "WiiConnect24 is disabled."},
        // Catch all for client side internet errors
        {-5, "Internet connection error. Make sure your network settings are valid by running a connection test."}
};