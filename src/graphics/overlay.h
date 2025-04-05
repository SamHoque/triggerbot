/**
 * @file overlay.h
 * @brief Window overlay implementation for visualizing scan areas
 * @author Sam Hoque
 *
 * License: Do whatever the fuck you want
 */

#pragma once

#include <Windows.h>
#include <iostream>
#include <string>

/**
 * @class Overlay
 * @brief Implements a Windows-native transparent overlay to visualize scan areas
 */
class Overlay {
public:
    /**
     * @brief Constructor
     */
    Overlay();
    
    /**
     * @brief Destructor
     */
    ~Overlay();
    
    /**
     * @brief Initialize the overlay window
     * @param screenWidth Width of the screen
     * @param screenHeight Height of the screen
     * @return true if successful, false otherwise
     */
    bool initialize(int screenWidth, int screenHeight);
    
    /**
     * @brief Update the overlay with current scan position and radius
     * @param centerX X-coordinate of scan center
     * @param centerY Y-coordinate of scan center
     * @param scanAreaX Width of scan area
     * @param scanAreaY Height of scan area
     */
    void update(int centerX, int centerY, int scanAreaX, int scanAreaY);
    
    /**
     * @brief Show or hide the overlay
     * @param visible true to show, false to hide
     */
    void setVisible(bool visible);
    
    /**
     * @brief Check if overlay is currently visible
     * @return true if visible, false otherwise
     */
    [[nodiscard]] bool isVisible() const;
    
private:
    /**
     * @brief Register window class
     * @return true if successful, false otherwise
     */
    bool registerWindowClass();
    
    /**
     * @brief Create overlay window
     * @return true if successful, false otherwise
     */
    bool createOverlayWindow(int screenWidth, int screenHeight);
    
    /**
     * @brief Render the overlay
     */
    void render() const;
    
    /**
     * @brief Static window procedure
     */
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    
    // Window properties
    HWND hwnd;
    WNDCLASSEX wc{};
    int width;
    int height;
    bool visible;
    
    // Scan area properties
    int centerX;
    int centerY;
    int scanAreaX;
    int scanAreaY;
};