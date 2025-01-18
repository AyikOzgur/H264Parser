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

    std::cout << "File size: " << fileSize << std::endl;

    uint8_t* buffer = new uint8_t[fileSize];
    file.read(reinterpret_cast<char*>(buffer), fileSize);

    while(true)
    {

        H264Parser::NalType nalType;
        int nalSize;
        bool isLastNal{false};
        uint8_t* nal = H264Parser::getNal(buffer, fileSize, nalType, nalSize, isLastNal);
        
        if (isLastNal)
        {
            std::cout << "Last NAL" << std::endl;
            break;
        }
        else
        {
            std::cout << "NAL type: " << static_cast<int>(nalType) << " size: " << nalSize << std::endl;
        }

        if (nalType == H264Parser::NalType::SPS)
        {
            int width, height;
            if (H264Parser::parseSps(nal, nalSize, width, height))
            {
                std::cout << "Width: " << width << " Height: " << height << std::endl;
            }
            else
            {
                std::cerr << "Failed to parse SPS" << std::endl;
            }
        }

        // Move to the next NAL
        buffer = nal + nalSize;
        fileSize -= nalSize;

    }
}