/**
 * @file desktop_duplicator.cpp
 * @brief Desktop Duplication API wrapper implementation
 * @author Sam Hoque
 *
 * License: Do whatever the fuck you want
 */

#include "desktop_duplicator.h"

DesktopDuplicator::DesktopDuplicator() :
    screenWidth(0),
    screenHeight(0),
    frameCount(0),
    referenceFrameCaptured(false) {
}

DesktopDuplicator::~DesktopDuplicator() = default;

bool DesktopDuplicator::initialize() {
    // Create device
    D3D_FEATURE_LEVEL featureLevel;
    HRESULT hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        d3dDevice.getAddressOf(),
        &featureLevel,
        d3dContext.getAddressOf()
    );

    if (FAILED(hr)) {
        std::cerr << "Failed to create D3D11 device. Error: " << std::hex << hr << std::endl;
        return false;
    }

    // Get DXGI device
    ComResource<IDXGIDevice> dxgiDevice;
    hr = d3dDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void **>(dxgiDevice.getAddressOf()));
    if (FAILED(hr)) {
        std::cerr << "Failed to get DXGI device. Error: " << std::hex << hr << std::endl;
        return false;
    }

    // Get DXGI adapter
    ComResource<IDXGIAdapter> dxgiAdapter;
    hr = dxgiDevice->GetAdapter(dxgiAdapter.getAddressOf());
    if (FAILED(hr)) {
        std::cerr << "Failed to get DXGI adapter. Error: " << std::hex << hr << std::endl;
        return false;
    }

    // Get output (monitor)
    hr = dxgiAdapter->EnumOutputs(0, reinterpret_cast<IDXGIOutput **>(output.getAddressOf()));
    if (FAILED(hr)) {
        std::cerr << "Failed to get DXGI output. Error: " << std::hex << hr << std::endl;
        return false;
    }

    // Get output description to find screen dimensions
    DXGI_OUTPUT_DESC outputDesc;
    hr = output->GetDesc(&outputDesc);
    if (FAILED(hr)) {
        std::cerr << "Failed to get output description. Error: " << std::hex << hr << std::endl;
        return false;
    }

    screenWidth = outputDesc.DesktopCoordinates.right - outputDesc.DesktopCoordinates.left;
    screenHeight = outputDesc.DesktopCoordinates.bottom - outputDesc.DesktopCoordinates.top;

    // Create desktop duplication interface
    hr = output->QueryInterface(__uuidof(IDXGIOutput1), reinterpret_cast<void **>(output.getAddressOf()));
    if (FAILED(hr)) {
        std::cerr << "Failed to query IDXGIOutput1 interface. Error: " << std::hex << hr << std::endl;
        return false;
    }

    hr = output->DuplicateOutput(d3dDevice.get(), deskDupl.getAddressOf());
    if (FAILED(hr)) {
        if (hr == DXGI_ERROR_NOT_CURRENTLY_AVAILABLE) {
            std::cerr << "Desktop duplication API is already in use." << std::endl;
        } else {
            std::cerr << "Failed to duplicate output. Error: " << std::hex << hr << std::endl;
        }
        return false;
    }

    std::cout << "Desktop Duplicator initialized: " << screenWidth << "x" << screenHeight << std::endl;
    return true;
}

bool DesktopDuplicator::captureReferenceFrame(int scanAreaX, int scanAreaY, bool saveScreenshot) {
    if (!captureFrame(scanAreaX, scanAreaY, referenceFrame)) {
        std::cerr << "Failed to capture reference frame." << std::endl;
        return false;
    }

    if (saveScreenshot) {
        std::string filename = generateTimestampedFilename("reference", "bmp");
        if (saveFrameToBMP(referenceFrame, scanAreaX, scanAreaY, filename)) {
            std::cout << "Reference frame saved as " << filename << std::endl;
        } else {
            std::cerr << "Failed to save reference frame" << std::endl;
        }
    }

    referenceFrameCaptured = true;
    return true;
}

bool DesktopDuplicator::checkForChanges(int scanAreaX, int scanAreaY, double changeSensitivity,
                                        int pixelChangeThreshold, bool debug, bool saveScreenshot) {
    if (!referenceFrameCaptured) {
        std::cerr << "No reference frame captured. Call captureReferenceFrame first." << std::endl;
        return false;
    }

    std::vector<Color> currentFrame;
    if (!captureFrame(scanAreaX, scanAreaY, currentFrame)) {
        std::cerr << "Failed to capture current frame for comparison." << std::endl;
        return false;
    }

    // Validate frame sizes match
    if (currentFrame.size() != referenceFrame.size()) {
        std::cerr << "Current frame size doesn't match reference frame size." << std::endl;
        return false;
    }

    int pixelChanges = 0;
    std::string triggerReason;
    
    // Check for significant changes using per-channel threshold comparison
    // Consider a pixel changed if any channel (R,G,B) difference exceeds the threshold
    // Use a percentage of 255 for the threshold (e.g., 40% = 0.4 * 255 = 102)
    int threshold = static_cast<int>(changeSensitivity * 0.01 * 255);
    int maxRDiff = 0, maxGDiff = 0, maxBDiff = 0;
    int maxDiffPixelIndex = -1;
    
    for (size_t i = 0; i < currentFrame.size(); i++) {
        int rDiff = std::abs(static_cast<int>(currentFrame[i].r) - static_cast<int>(referenceFrame[i].r));
        int gDiff = std::abs(static_cast<int>(currentFrame[i].g) - static_cast<int>(referenceFrame[i].g));
        int bDiff = std::abs(static_cast<int>(currentFrame[i].b) - static_cast<int>(referenceFrame[i].b));
        
        // Track max differences for logging
        if (rDiff > maxRDiff) {
            maxRDiff = rDiff;
            maxDiffPixelIndex = static_cast<int>(i);
        }
        if (gDiff > maxGDiff) {
            maxGDiff = gDiff;
            maxDiffPixelIndex = static_cast<int>(i);
        }
        if (bDiff > maxBDiff) {
            maxBDiff = bDiff;
            maxDiffPixelIndex = static_cast<int>(i);
        }
        
        // Consider pixel changed if any channel exceeds threshold
        if (rDiff > threshold || gDiff > threshold || bDiff > threshold) {
            pixelChanges++;
        }
    }

    // Debug output
    if (debug) {
        std::cout << "Changes detected: " << pixelChanges << " pixels (threshold: " << pixelChangeThreshold << ")" << std::endl;
        std::cout << "Max differences - R: " << maxRDiff << ", G: " << maxGDiff << ", B: " << maxBDiff << std::endl;
    }

    // Check if changes exceed threshold
    bool significantChanges = pixelChanges >= pixelChangeThreshold;
    
    // If significant changes detected and debug is enabled, provide detailed information
    if (significantChanges) {
        // Build reason string for logging
        std::ostringstream reason;
        reason << "Changed pixels: " << pixelChanges << " (threshold: " << pixelChangeThreshold << ")\n";
        reason << "Max channel differences - R: " << maxRDiff << ", G: " << maxGDiff << ", B: " << maxBDiff << "\n";
        
        if (maxDiffPixelIndex >= 0) {
            int x = maxDiffPixelIndex % scanAreaX;
            int y = maxDiffPixelIndex / scanAreaX;
            Color ref = referenceFrame[maxDiffPixelIndex];
            Color cur = currentFrame[maxDiffPixelIndex];
            
            reason << "Sample pixel at (" << x << "," << y << "):\n";
            reason << "  Reference RGB: (" << static_cast<int>(ref.r) << "," << static_cast<int>(ref.g) << "," << static_cast<int>(ref.b) << ")\n";
            reason << "  Current RGB: (" << static_cast<int>(cur.r) << "," << static_cast<int>(cur.g) << "," << static_cast<int>(cur.b) << ")\n";
            reason << "  Differences: R=" << (static_cast<int>(cur.r) - static_cast<int>(ref.r)) 
                   << ", G=" << (static_cast<int>(cur.g) - static_cast<int>(ref.g)) 
                   << ", B=" << (static_cast<int>(cur.b) - static_cast<int>(ref.b));
        }
        
        triggerReason = reason.str();
        
        if (debug) {
            std::cout << "Trigger reason:\n" << triggerReason << std::endl;
        }
    }
    
    // If significant changes detected and screenshot saving is enabled
    if (significantChanges && saveScreenshot) {
        std::string filename = generateTimestampedFilename("trigger", "bmp");
        
        // Also save trigger reason to a companion text file
        std::string reasonFilename = filename.substr(0, filename.length() - 4) + ".txt";
        std::ofstream reasonFile(reasonFilename);
        if (reasonFile) {
            reasonFile << "Trigger Reason: " << triggerReason << std::endl;
            // Store time_t in a variable first to avoid taking address of temporary
            std::time_t currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            reasonFile << "Time: " << std::put_time(std::localtime(&currentTime), "%Y-%m-%d %H:%M:%S") << std::endl;
            reasonFile.close();
        }
        
        if (saveFrameToBMP(currentFrame, scanAreaX, scanAreaY, filename)) {
            if (debug) {
                std::cout << "Trigger frame saved as " << filename << std::endl;
                std::cout << "Trigger reason saved to " << reasonFilename << std::endl;
            }
        } else {
            std::cerr << "Failed to save trigger frame" << std::endl;
        }
    }

    return significantChanges;
}

bool DesktopDuplicator::captureFrame(int scanAreaX, int scanAreaY, std::vector<Color>& frame) const {
    if (!deskDupl) {
        std::cerr << "Desktop duplication not initialized." << std::endl;
        return false;
    }

    DXGI_OUTDUPL_FRAME_INFO frameInfo;
    ComResource<IDXGIResource> desktopResource;
    HRESULT hr = deskDupl->AcquireNextFrame(500, &frameInfo, desktopResource.getAddressOf());

    if (FAILED(hr)) {
        if (hr == DXGI_ERROR_WAIT_TIMEOUT) {
            // Timeout is normal if there are no changes
            return false;
        }
        std::cerr << "Failed to acquire next frame. Error code: 0x" << std::hex << hr << std::dec << std::endl;
        return false;
    }

    // Get the desktop texture
    ComResource<ID3D11Texture2D> desktopTexture;
    hr = desktopResource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void **>(desktopTexture.getAddressOf()));
    if (FAILED(hr)) {
        std::cerr << "Failed to query interface on desktop resource. Error code: 0x" << std::hex << hr << std::dec << std::endl;
        hr = deskDupl->ReleaseFrame();
        if (FAILED(hr)) {
            std::cerr << "Failed to release frame. Error code: 0x" << std::hex << hr << std::dec << std::endl;
        }
        return false;
    }

    // Get texture description
    D3D11_TEXTURE2D_DESC desc;
    desktopTexture->GetDesc(&desc);

    // Create staging texture to copy from GPU to CPU memory
    ComResource<ID3D11Texture2D> stagingTexture;
    D3D11_TEXTURE2D_DESC stagingDesc = desc;
    stagingDesc.BindFlags = 0;
    stagingDesc.Usage = D3D11_USAGE_STAGING;
    stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    stagingDesc.MiscFlags = 0;

    hr = d3dDevice->CreateTexture2D(&stagingDesc, nullptr, stagingTexture.getAddressOf());
    if (FAILED(hr)) {
        std::cerr << "Failed to create staging texture. Error code: 0x" << std::hex << hr << std::dec << std::endl;
        hr = deskDupl->ReleaseFrame();
        if (FAILED(hr)) {
            std::cerr << "Failed to release frame. Error code: 0x" << std::hex << hr << std::dec << std::endl;
        }
        return false;
    }

    // Copy from desktop texture to staging texture
    d3dContext->CopyResource(stagingTexture.get(), desktopTexture.get());

    // Map staging texture to CPU accessible memory
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    hr = d3dContext->Map(stagingTexture.get(), 0, D3D11_MAP_READ, 0, &mappedResource);
    if (FAILED(hr)) {
        std::cerr << "Failed to map staging texture. Error code: 0x" << std::hex << hr << std::dec << std::endl;
        hr = deskDupl->ReleaseFrame();
        if (FAILED(hr)) {
            std::cerr << "Failed to release frame. Error code: 0x" << std::hex << hr << std::dec << std::endl;
        }
        return false;
    }

    // Get screen center
    int centerX = screenWidth / 2;
    int centerY = screenHeight / 2;

    // Calculate scan area rectangle (centered)
    int scanLeft = centerX - scanAreaX / 2;
    int scanTop = centerY - scanAreaY / 2;

    // Boundary checks
    if (scanLeft < 0) scanLeft = 0;
    if (scanTop < 0) scanTop = 0;
    if (scanLeft + scanAreaX > screenWidth) scanLeft = screenWidth - scanAreaX;
    if (scanTop + scanAreaY > screenHeight) scanTop = screenHeight - scanAreaY;

    // Calculate scan area end points
    int scanRight = scanLeft + scanAreaX;
    int scanBottom = scanTop + scanAreaY;

    // Resize frame vector to hold scan area pixels
    frame.resize(scanAreaX * scanAreaY);

    // Copy pixels from mapped resource to frame vector, but only for the scan area
    const auto srcPtr = static_cast<BYTE*>(mappedResource.pData);
    size_t pixelIndex = 0;

    for (int y = scanTop; y < scanBottom; y++) {
        for (int x = scanLeft; x < scanRight; x++) {
            // Calculate source index in the full texture
            const size_t srcIndex = y * mappedResource.RowPitch + x * 4; // 4 bytes per pixel (B,G,R,A)

            // Extract BGR color components (desktop duplication gives BGR)
            Color pixel {};
            pixel.b = srcPtr[srcIndex + 0]; // Blue
            pixel.g = srcPtr[srcIndex + 1]; // Green
            pixel.r = srcPtr[srcIndex + 2]; // Red

            // Store in frame
            frame[pixelIndex++] = pixel;
        }
    }

    // Unmap the resource
    d3dContext->Unmap(stagingTexture.get(), 0);

    // Release frame and check result
    hr = deskDupl->ReleaseFrame();
    if (FAILED(hr)) {
        std::cerr << "Failed to release frame. Error code: 0x" << std::hex << hr << std::dec << std::endl;
        return false;
    }

    return true;
}

bool DesktopDuplicator::saveFrameToBMP(const std::vector<Color>& frame, const int width, const int height, const std::string& filename) {
    // Make sure the screenshots directory exists
    if (!CreateDirectory("screenshots", nullptr) && GetLastError() != ERROR_ALREADY_EXISTS) {
        std::cerr << "Failed to create screenshots directory" << std::endl;
        return false;
    }
    
    // Full path including directory
    const std::string fullPath = "screenshots/" + filename;
    
    // Open file for binary writing
    std::ofstream file(fullPath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << fullPath << std::endl;
        return false;
    }
    
    // BMP file header (14 bytes)
    constexpr int fileHeaderSize = 0x0E;  // 14 bytes
    // DIB header (40 bytes for BITMAPINFOHEADER)
    constexpr int dibHeaderSize = 0x28;   // 40 bytes
    // No color palette for 24-bit BMP
    constexpr int colorPaletteSize = 0x00;

    // Row size must be a multiple of 4 bytes
    const int rowSize = ((width * 3) + 3) & ~3;
    const int dataSize = rowSize * height;

    // Total file size
    const int fileSize = fileHeaderSize + dibHeaderSize + colorPaletteSize + dataSize;

    // File offset to pixel data
    constexpr int pixelDataOffset = fileHeaderSize + dibHeaderSize + colorPaletteSize;

    // BMP file header (14 bytes) - Use uint8_t instead of char to avoid narrowing
    uint8_t fileHeader[fileHeaderSize] = {
        'B', 'M',                       // Signature
        0, 0, 0, 0,                     // File size (bytes)
        0, 0, 0, 0,                     // Reserved
        0, 0, 0, 0                      // Pixel data offset
    };

    // Set file size
    fileHeader[2] = static_cast<uint8_t>(fileSize & 0xFF);
    fileHeader[3] = static_cast<uint8_t>((fileSize >> 8) & 0xFF);
    fileHeader[4] = static_cast<uint8_t>((fileSize >> 16) & 0xFF);
    fileHeader[5] = static_cast<uint8_t>((fileSize >> 24) & 0xFF);

    // Set pixel data offset
    fileHeader[10] = static_cast<uint8_t>(pixelDataOffset & 0xFF);
    fileHeader[11] = static_cast<uint8_t>((pixelDataOffset >> 8) & 0xFF);
    fileHeader[12] = static_cast<uint8_t>((pixelDataOffset >> 16) & 0xFF);
    fileHeader[13] = static_cast<uint8_t>((pixelDataOffset >> 24) & 0xFF);

    // DIB header (BITMAPINFOHEADER, 40 bytes)
    uint8_t dibHeader[dibHeaderSize] = {
        0, 0, 0, 0,                     // Header size (40 bytes)
        0, 0, 0, 0,                     // Image width
        0, 0, 0, 0,                     // Image height (negative means top-down)
        0, 0,                           // Number of color planes (1)
        0, 0,                           // Bits per pixel (24)
        0, 0, 0, 0,                     // Compression method (0 = none)
        0, 0, 0, 0,                     // Image size (can be 0 for uncompressed)
        0, 0, 0, 0,                     // Horizontal resolution (pixels per meter)
        0, 0, 0, 0,                     // Vertical resolution (pixels per meter)
        0, 0, 0, 0,                     // Number of colors in palette (0 = 2^n)
        0, 0, 0, 0                      // Number of important colors (0 = all)
    };

    // Set header size
    dibHeader[0] = static_cast<uint8_t>(dibHeaderSize & 0xFF);
    dibHeader[1] = static_cast<uint8_t>((dibHeaderSize >> 8) & 0xFF);
    dibHeader[2] = static_cast<uint8_t>((dibHeaderSize >> 16) & 0xFF);
    dibHeader[3] = static_cast<uint8_t>((dibHeaderSize >> 24) & 0xFF);

    // Set image width
    dibHeader[4] = static_cast<uint8_t>(width & 0xFF);
    dibHeader[5] = static_cast<uint8_t>((width >> 8) & 0xFF);
    dibHeader[6] = static_cast<uint8_t>((width >> 16) & 0xFF);
    dibHeader[7] = static_cast<uint8_t>((width >> 24) & 0xFF);

    // Set image height (negative for top-down)
    int negHeight = -height;
    dibHeader[8] = static_cast<uint8_t>(negHeight & 0xFF);
    dibHeader[9] = static_cast<uint8_t>((negHeight >> 8) & 0xFF);
    dibHeader[10] = static_cast<uint8_t>((negHeight >> 16) & 0xFF);
    dibHeader[11] = static_cast<uint8_t>((negHeight >> 24) & 0xFF);

    // Set number of color planes
    dibHeader[12] = 0x01;  // 1 plane

    // Set bits per pixel
    dibHeader[14] = 0x18;  // 24 bits per pixel

    // Write file header
    file.write(reinterpret_cast<const char*>(fileHeader), fileHeaderSize);

    // Write DIB header
    file.write(reinterpret_cast<const char*>(dibHeader), dibHeaderSize);

    // Write pixel data
    // BMP format uses BGR order, so we need to swap R and B
    std::vector<uint8_t> rowBuffer(rowSize, 0);

    for (int y = 0; y < height; y++) {
        int rowPos = 0;
        for (int x = 0; x < width; x++) {
            const auto&[r, g, b] = frame[y * width + x];

            // BGR order for BMP
            rowBuffer[rowPos++] = static_cast<uint8_t>(b);
            rowBuffer[rowPos++] = static_cast<uint8_t>(g);
            rowBuffer[rowPos++] = static_cast<uint8_t>(r);
        }

        // Write row
        file.write(reinterpret_cast<const char*>(rowBuffer.data()), rowSize);
    }

    file.close();
    return true;
}

std::string DesktopDuplicator::generateTimestampedFilename(const std::string& prefix, const std::string& extension) {
    // Get current time
    const auto now = std::chrono::system_clock::now();
    const std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    
    // Convert to milliseconds
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                        now.time_since_epoch()) % 1000;
    
    // Format time as string
    std::stringstream ss;
    ss << prefix << "_";
    
    // Add date and time
    std::tm tm_buf {};
    localtime_s(&tm_buf, &now_time);
    
    ss << std::put_time(&tm_buf, "%Y%m%d_%H%M%S");
    
    // Add milliseconds
    ss << "_" << std::setfill('0') << std::setw(3) << ms.count();
    
    // Add frame count for uniqueness
    ss << "_" << std::setfill('0') << std::setw(4) << (++frameCount);
    
    // Add extension
    ss << "." << extension;
    
    return ss.str();
}