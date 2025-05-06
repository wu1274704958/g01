#include "fileLoader/DefFileLoaderWide.h"
#include <fstream>

std::span<uint8_t> eqd_mp::DefFileLoaderWide::loadFormMemory(const std::wstring& path)
{
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs.is_open()) {
        return {};
    }

    ifs.seekg(0, std::ios::end);
    std::streampos fileSize = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    _buffer.resize(fileSize);
    ifs.read(reinterpret_cast<char*>(_buffer.data()), fileSize);

    return {_buffer};
}
