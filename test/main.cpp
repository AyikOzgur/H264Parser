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
            switch (nalType)
            {
                case H264Parser::NalType::SPS:
                    std::cout << "SPS NAL size: " << nalSize << std::endl;
                    break;
                case H264Parser::NalType::PPS:
                    std::cout << "PPS NAL size: " << nalSize << std::endl;
                    break;
                case H264Parser::NalType::IDR:
                    std::cout << "IDR NAL size: " << nalSize << std::endl;
                    break;
                case H264Parser::NalType::NON_IDR:
                    std::cout << "NON_IDR NAL size: " << nalSize << std::endl;
                    break;
                case H264Parser::NalType::SEI:
                    std::cout << "SEI NAL size: " << nalSize << std::endl;
                    break;
                case H264Parser::NalType::AUD:
                    std::cout << "AUD NAL size: " << nalSize << std::endl;
                    break;
                case H264Parser::NalType::END_OF_SEQ:
                    std::cout << "END_OF_SEQ NAL size: " << nalSize << std::endl;
                    break;
                case H264Parser::NalType::END_OF_STREAM:
                    std::cout << "END_OF_STREAM NAL size: " << nalSize << std::endl;
                    break;
                case H264Parser::NalType::FILLER:
                    std::cout << "FILLER NAL size: " << nalSize << std::endl;
                    break;
                default:
                    std::cout << "Unknown NAL size: " << nalSize << std::endl;
                    break;
            }
        }

        if (nalType == H264Parser::NalType::SPS && width == 0 && height == 0)
            H264Parser::parseSps(nal, nalSize, width, height);

        // Move to the next NAL
        buffer = nal + nalSize;
        fileSize -= nalSize;

    }
}