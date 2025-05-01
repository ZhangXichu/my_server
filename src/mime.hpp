#pragma once
#include <string>

namespace my_server {

/**
 * Return a MIME type for a given filename.
 * If the extension is unknown or missing, returns "application/octet-stream".
 */
std::string mime_type_get(const std::string &filename);

} // namespace my_server