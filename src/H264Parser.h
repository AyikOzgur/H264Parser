#pragma once
#include <cstdint>
#include <string>

namespace H264Parser
{

    enum class NalType
    {
        UNKNOWN = 0,
        NON_IDR = 1,
        IDR = 5,
        SEI = 6,
        SPS = 7,
        PPS = 8,
    };

    /**
     * @brief Get a nal from the input buffer
     * @param inputBuffer The input buffer
     * @param inputBufferSize The size of the input buffer
     * @param nalType Type of the found nal
     * @param nalSize Size of the found nal
     * @return Pointer to the found nal. It is just a pointer from the input buffer.
     */
    uint8_t* getNal(const uint8_t* inputBuffer, int inputBufferSize, NalType& nalType, int& nalSize);

    /**
     * @brief Parse the SPS buffer to get the width and height of the video
     * @param spsBuffer The SPS buffer
     * @param spsSize The size of the SPS buffer
     * @param width The width of the video
     * @param height The height of the video
     * @return True if the SPS buffer was parsed successfully, false otherwise
     */
    bool parseSps(const uint8_t* spsBuffer, int spsSize, int& width, int& height);

}