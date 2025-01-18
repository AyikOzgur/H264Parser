#include <iostream>
#include <fstream>
#include "H264Parser.h"

int main() 
{
    // Open the test file
    std::ifstream file("../../test/test.h264", std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file" << std::endl;
        return -1;
    }

    // Get the file size
    file.seekg(0, std::ios::end);
    int fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    uint8_t* buffer = new uint8_t[fileSize];
    file.read(reinterpret_cast<char*>(buffer), fileSize);

    int width{0}, height{0};

    while(true)
    {

        H264Parser::NalType nalType;
        int nalSize;
        bool isLastNal{false};
        uint8_t* nal = H264Parser::getNal(buffer, fileSize, nalType, nalSize, isLastNal);
        
        if (isLastNal)
        {
            std::cout << "Last NAL" << std::endl;
            std::cout << "Width: " << width << " Height: " << height << std::endl;
            break;
        }
        else
        {
            std::cout << "NAL type: " << static_cast<int>(nalType) << " size: " << nalSize << std::endl;
        }

        if (nalType == H264Parser::NalType::SPS && width == 0 && height == 0)
        {
            H264Parser::parseSps(nal, nalSize, width, height);
        }

        // Move to the next NAL
        buffer = nal + nalSize;
        fileSize -= nalSize;

    }
}