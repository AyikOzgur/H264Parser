#include "H264Parser.h"
#include <iostream>
#include <fstream>
#include <vector>


uint8_t* H264Parser::getNal(uint8_t* inputBuffer, int inputBufferSize, NalType& nalType, int& nalSize, bool& isLastNal)
{
    const int maxStartCodeLength = 4;
    bool isStartFound = false, isEndFound = false;
    int i = 0, startCode = 0;

    uint8_t* nalStart = nullptr;

    // find first frame.
    for (i = 0; i < inputBufferSize - maxStartCodeLength; i++)
    {
        if (inputBuffer[i] == 0 && inputBuffer[i + 1] == 0 && inputBuffer[i + 2] == 1)
        {
            startCode = 3;
        }
        else if (inputBuffer[i] == 0 && inputBuffer[i + 1] == 0 && inputBuffer[i + 2] == 0 && inputBuffer[i + 3] == 1)
        {
            startCode = 4;
        }
        else
        {
            continue;
        }

        nalStart = (&inputBuffer[i + startCode]);
        nalType = static_cast<NalType>(nalStart[0] & 0x1F);
        // look for NALs.
        if (nalType == NalType::SPS || nalType == NalType::PPS || nalType == NalType::IDR || nalType == NalType::NON_IDR || nalType == NalType::SEI
            || nalType == NalType::AUD || nalType == NalType::END_OF_SEQ || nalType == NalType::END_OF_STREAM || nalType == NalType::FILLER)
        {
            isStartFound = true;
            i += 4; // skip NAL header
            break;
        }
    }

    // find next frame
    for (; i < inputBufferSize - maxStartCodeLength; i++)
    {
        if (inputBuffer[i] == 0 && inputBuffer[i + 1] == 0 && inputBuffer[i + 2] == 1)
        {
            startCode = 3;
        }
        else if (inputBuffer[i] == 0 && inputBuffer[i + 1] == 0 && inputBuffer[i + 2] == 0 && inputBuffer[i + 3] == 1)
        {
            startCode = 4;
        }
        else
        {
            continue;
        }

        uint8_t* nextNal = (&inputBuffer[i + startCode]);
        NalType nextNalType = static_cast<NalType>(nextNal[0] & 0x1F);

        // Check for any NAL
        if (nextNalType == NalType::SPS || nextNalType == NalType::PPS || nextNalType == NalType::IDR || nextNalType == NalType::NON_IDR || nextNalType == NalType::SEI
            || nextNalType == NalType::AUD || nextNalType == NalType::END_OF_SEQ || nextNalType == NalType::END_OF_STREAM || nextNalType == NalType::FILLER)
        {
            isEndFound = true;
            break;
        }
    }

    if (isStartFound && isEndFound) 
    {
        nalSize = i - (nalStart - inputBuffer);
        return nalStart;
    }
    else if (isStartFound && !isEndFound) 
    {
        nalSize = inputBufferSize - (nalStart - inputBuffer);
        isLastNal = true; // Because we could not find the next NAL which means this is the last NAL
        return nalStart;
    }

    // No NAL found
    nalSize = 0;
    nalType = NalType::UNKNOWN;
    isLastNal = true; // When there is no NAL, it may be better to also consider it as the last NAL
    return nullptr;
}

bool H264Parser::parseSps(uint8_t* data, int size, int& width, int& height)
{
}