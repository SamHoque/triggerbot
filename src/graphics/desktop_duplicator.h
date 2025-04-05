/**
 * @file desktop_duplicator.h
 * @brief Desktop Duplication API wrapper header
 * @author Sam Hoque
 *
 * License: Do whatever the fuck you want
 */

#pragma once

#include <d3d11.h>
#include <dxgi1_2.h>
#include <vector>
#include <iostream>
#include <cmath>
#include <cstdint>
#include <windows.h>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include "common/com_resource.h"

/**
 * @struct Color
 * @brief Basic RGB color structure
 */
struct Color {
    uint8_t r, g, b;
};

/**
 * @class DesktopDuplicator
 * @brief Wrapper for Windows Desktop Duplication API
 *
 * Provides functionality for screen capture and pixel-level analysis
 */
class DesktopDuplicator {
public:
    /**
     * @brief Constructor
     */
    DesktopDuplicator();

    /**
     * @brief Destructor
     */
    ~DesktopDuplicator();

    /**
     * @brief Initialize desktop duplication resources
     * @return true if successful, false otherwise
     */
    bool initialize();

    /**
     * @brief Capture a reference frame for later comparison
     * @param scanAreaX Width of the scan area
     * @param scanAreaY Height of the scan area
     * @param saveScreenshot Whether to save the screenshot to file
     * @return true if successful, false otherwise
     */
    bool captureReferenceFrame(int scanAreaX, int scanAreaY, bool saveScreenshot = false);

    /**
     * @brief Check for significant changes compared to reference frame
     * @param scanAreaX Width of the scan area
     * @param scanAreaY Height of the scan area
     * @param changeSensitivity Threshold for color difference to count as changed
     * @param pixelChangeThreshold Number of changed pixels required to trigger
     * @param debug Enable debug output
     * @param saveScreenshot Whether to save the screenshot when changes are detected
     * @return true if significant changes detected, false otherwise
     */
    bool checkForChanges(int scanAreaX, int scanAreaY, double changeSensitivity, 
                         int pixelChangeThreshold, bool debug, bool saveScreenshot = false);

private:
    /**
     * @brief Capture a frame from the screen
     * @param scanAreaX Width of the scan area
     * @param scanAreaY Height of the scan area
     * @param frame Vector to store captured pixel data
     * @return true if successful, false otherwise
     */
    bool captureFrame(int scanAreaX, int scanAreaY, std::vector<Color>& frame) const;
    
    /**
     * @brief Save frame to a BMP file
     * @param frame Vector of pixel data to save
     * @param width Width of the image
     * @param height Height of the image
     * @param filename Filename to save as
     * @return true if successful, false otherwise
     */
    static bool saveFrameToBMP(const std::vector<Color>& frame, int width, int height, const std::string& filename);
    
    /**
     * @brief Generate a timestamped filename
     * @param prefix Prefix for the filename
     * @param extension File extension (without dot)
     * @return Timestamped filename
     */
    std::string generateTimestampedFilename(const std::string& prefix, const std::string& extension);

    ComResource<ID3D11Device> d3dDevice;
    ComResource<ID3D11DeviceContext> d3dContext;
    ComResource<IDXGIOutput1> output;
    ComResource<IDXGIOutputDuplication> deskDupl;
    int screenWidth;
    int screenHeight;
    int frameCount;

    // Reference frame for comparison
    std::vector<Color> referenceFrame;
    bool referenceFrameCaptured;
};