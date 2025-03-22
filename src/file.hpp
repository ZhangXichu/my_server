#ifndef _FILELS_H_
#define _FILELS_H_

#include <cstddef>
#include <memory>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <cstddef> 

namespace my_server {

class File {

public:
explicit File(const std::string& filename);

std::size_t size() const { return _size; }
const std::vector<char>& data() const { return _data; }

private:
std::size_t _size;
std::vector<char> _data;

};



}

#endif