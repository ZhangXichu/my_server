#include "file.hpp"

#include <system_error>

namespace my_server {

File::File(const std::string& filename){
    std::error_code ec;

    if (!std::filesystem::is_regular_file(filename, ec) || ec) {
        throw std::runtime_error("File does not exist or is not a regular file: " + filename);
    }

    auto file_size = std::filesystem::file_size(filename, ec);

    _size = static_cast<std::size_t>(file_size);

    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    _data.resize(_size);

    if (!file.read(reinterpret_cast<char*>(_data.data()), _size)) {
        throw std::runtime_error("Failed to read file: " + filename);
    }
}

}