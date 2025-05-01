#include "mime.hpp"

#include <algorithm>
#include <cctype>
#include <unordered_map>

namespace my_server {

static constexpr char DEFAULT_MIME_TYPE[] = "application/octet-stream";

static const std::unordered_map<std::string, std::string> mime_map = {
    {"html", "text/html"},
    {"htm",  "text/html"},
    {"jpeg", "image/jpg"},
    {"jpg",  "image/jpg"},
    {"css",  "text/css"},
    {"js",   "application/javascript"},
    {"json", "application/json"},
    {"txt",  "text/plain"},
    {"gif",  "image/gif"},
    {"png",  "image/png"}
};

std::string mime_type_get(const std::string &filename) {
    auto pos = filename.rfind('.');
    if (pos == std::string::npos) {
        return DEFAULT_MIME_TYPE;
    }

    std::string ext = filename.substr(pos + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c){ return static_cast<char>(std::tolower(c)); });

    auto it = mime_map.find(ext);
    if (it != mime_map.end()) {
        return it->second;
    }
    return DEFAULT_MIME_TYPE;
}

} // namespace my_server