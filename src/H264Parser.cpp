#include "H264Parser.h"
#include <iostream>
#include <fstream>
#include <vector>

uint8_t *H264Parser::getNal(uint8_t *inputBuffer, int inputBufferSize, NalType &nalType, int &nalSize, bool &isLastNal)
{
    const int maxStartCodeLength = 4;
    bool isStartFound = false, isEndFound = false;
    int i = 0, startCode = 0;

    uint8_t *nalStart = nullptr;

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
        if (nalType == NalType::SPS || nalType == NalType::PPS || nalType == NalType::IDR || nalType == NalType::NON_IDR || nalType == NalType::SEI || nalType == NalType::AUD || nalType == NalType::END_OF_SEQ || nalType == NalType::END_OF_STREAM || nalType == NalType::FILLER)
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

        uint8_t *nextNal = (&inputBuffer[i + startCode]);
        NalType nextNalType = static_cast<NalType>(nextNal[0] & 0x1F);

        // Check for any NAL
        if (nextNalType == NalType::SPS || nextNalType == NalType::PPS || nextNalType == NalType::IDR || nextNalType == NalType::NON_IDR || nextNalType == NalType::SEI || nextNalType == NalType::AUD || nextNalType == NalType::END_OF_SEQ || nextNalType == NalType::END_OF_STREAM || nextNalType == NalType::FILLER)
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

namespace
{

    uint32_t readBits(uint8_t *data, uint32_t count, uint32_t &offset)
    {
        uint32_t result = 0;
        for (uint32_t i = 0; i < count; i++)
        {
            result <<= 1; // Make room for the new bit
            result |= (data[offset / 8] >> (7 - (offset % 8))) & 1;
            offset++;
        }
        return result;
    }

    uint32_t readUeg(uint8_t *data, uint32_t &offset)
    {
        uint32_t leadingZeros = 0;
        while (readBits(data, 1, offset) == 0)
        {
            leadingZeros++;
        }
        return (1 << leadingZeros) - 1 + readBits(data, leadingZeros, offset);
    }

    int32_t readEg(uint8_t *data, uint32_t &offset)
    {
        uint32_t value = readUeg(data, offset);
        if (value % 2 == 0)
        {
            return -1 * (value / 2);
        }
        else
        {
            return (value + 1) / 2;
        }
    }

    void skipScalingList(uint8_t *data, uint32_t size, uint32_t &offset)
    {
        uint32_t lastScale = 8;
        uint32_t nextScale = 8;
        for (uint32_t i = 0; i < size; i++)
        {
            if (nextScale != 0)
            {
                int32_t deltaScale = readEg(data, offset);
                nextScale = (lastScale + deltaScale + 256) % 256;
            }
            lastScale = (nextScale == 0) ? lastScale : nextScale;
        }
    }
}

bool H264Parser::parseSps(uint8_t *data, int size, int &width, int &height)
{
    uint32_t offset = 0;
    uint8_t profileIdc = 0;
    uint32_t pict_order_cnt_type = 0;
    uint32_t picWidthInMbsMinus1 = 0;
    uint32_t picHeightInMapUnitsMinus1 = 0;
    uint8_t frameMbsOnlyFlag = 0;
    uint32_t frameCropLeftOffset = 0;
    uint32_t frameCropRightOffset = 0;
    uint32_t frameCropTopOffset = 0;
    uint32_t frameCropBottomOffset = 0;

    readBits(data, 8, offset);
    profileIdc = readBits(data, 8, offset);
    readBits(data, 16, offset);
    readUeg(data, offset);

    if (profileIdc == 100 || profileIdc == 110 || profileIdc == 122 ||
        profileIdc == 244 || profileIdc == 44 || profileIdc == 83 ||
        profileIdc == 86 || profileIdc == 118 || profileIdc == 128)
    {
        uint32_t chromaFormatIdc = readUeg(data, offset);

        if (chromaFormatIdc == 3)
            readBits(data, 1, offset);

        readUeg(data, offset);
        readUeg(data, offset);
        readBits(data, 1, offset);
        
        if (readBits(data, 1, offset))
        {
            for (uint8_t i = 0; i < (chromaFormatIdc != 3) ? 8 : 12; i++)
            {
                if (readBits(data, 1, offset))
                {
                    if (i < 6)
                        skipScalingList(data, 16, offset);
                    else
                        skipScalingList(data, 64, offset);
                }
            }
        }
    }

    readUeg(data, offset);
    pict_order_cnt_type = readUeg(data, offset);

    if (pict_order_cnt_type == 0)
    {
        readUeg(data, offset);
    }
    else if (pict_order_cnt_type == 1)
    {
        readBits(data, 1, offset);
        readEg(data, offset);
        readEg(data, offset);
        for (uint32_t i = 0; i < readUeg(data, offset); i++)
        {
            readEg(data, offset);
        }
    }

    readUeg(data, offset);
    readBits(data, 1, offset);
    picWidthInMbsMinus1 = readUeg(data, offset);
    picHeightInMapUnitsMinus1 = readUeg(data, offset);
    frameMbsOnlyFlag = readBits(data, 1, offset);
    if (!frameMbsOnlyFlag)
        readBits(data, 1, offset);

    readBits(data, 1, offset);

    if (readBits(data, 1, offset))
    {
        frameCropLeftOffset = readUeg(data, offset);
        frameCropRightOffset = readUeg(data, offset);
        frameCropTopOffset = readUeg(data, offset);
        frameCropBottomOffset = readUeg(data, offset);
    }

    width = ((picWidthInMbsMinus1 + 1) * 16) - frameCropLeftOffset * 2 - frameCropRightOffset * 2;
    height = ((2 - frameMbsOnlyFlag) * (picHeightInMapUnitsMinus1 + 1) * 16) - ((frameMbsOnlyFlag ? 2 : 4) * (frameCropTopOffset + frameCropBottomOffset));

    return true;
}