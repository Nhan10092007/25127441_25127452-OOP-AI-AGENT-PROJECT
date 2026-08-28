#include "image_info.h"
#include <fstream>
#include <cstdint>

bool readPngSize(const std::string& filePath, int& width, int& height) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    unsigned char header[24];
    file.read(reinterpret_cast<char*>(header), sizeof(header));
    if (file.gcount() != static_cast<std::streamsize>(sizeof(header))) {
        return false;
    }

    static const unsigned char pngSignature[8] = {0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A};
    for (int i = 0; i < 8; ++i) {
        if (header[i] != pngSignature[i]) {
            return false;
        }
    }
    if (header[12] != 'I' || header[13] != 'H' || header[14] != 'D' || header[15] != 'R') {
        return false;
    }

    // Width và height là số nguyên 4 byte big-endian, nằm ngay sau "IHDR"
    width  = (header[16] << 24) | (header[17] << 16) | (header[18] << 8) | header[19];
    height = (header[20] << 24) | (header[21] << 16) | (header[22] << 8) | header[23];
    return (width > 0 && height > 0);
}
