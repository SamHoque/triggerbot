/**
 * @file overlay.cpp
 * @brief Window overlay implementation for visualizing scan areas
 * @author Sam Hoque
 *
 * License: Do whatever the fuck you want
 */

#include "overlay.h"

// Window class name for the overlay
auto OVERLAY_WINDOW_CLASS = "TriggerBotOverlayClass";

Overlay::Overlay() : 
    hwnd(nullptr),
    width(0),
    height(0),
    visible(false),
    centerX(0),
    centerY(0),
    scanAreaX(0),
    scanAreaY(0) {
}

Overlay::~Overlay() {
    if (hwnd) {
        DestroyWindow(hwnd);
    }
    
    // Unregister window class
    UnregisterClass(OVERLAY_WINDOW_CLASS, GetModuleHandle(nullptr));
}

bool Overlay::initialize(const int screenWidth, const int screenHeight) {
    width = screenWidth;
    height = screenHeight;
    
    // Register window class
    if (!registerWindowClass()) {
        std::cerr << "Failed to register overlay window class" << std::endl;
        return false;
    }
    
    // Create overlay window
    if (!createOverlayWindow(screenWidth, screenHeight)) {
        std::cerr << "Failed to create overlay window" << std::endl;
        return false;
    }
    
    std::cout << "Overlay initialized successfully" << std::endl;
    return true;
}

void Overlay::update(int centerX, int centerY, int scanAreaX, int scanAreaY) {
    // Check if position actually changed to avoid unnecessary redraws
    bool needsRedraw = (this->centerX != centerX || 
                        this->centerY != centerY || 
                        this->scanAreaX != scanAreaX || 
                        this->scanAreaY != scanAreaY);
    
    // Update stored values
    this->centerX = centerX;
    this->centerY = centerY;
    this->scanAreaX = scanAreaX;
    this->scanAreaY = scanAreaY;
    
    // Only redraw if visible and something changed, or on a timer
    static DWORD lastDraw = GetTickCount();
    DWORD now = GetTickCount();
    
    // Throttle updates to max 30 FPS (33ms) to reduce overhead
    if (visible && (needsRedraw || now - lastDraw > 33)) {
        render();
        lastDraw = now;
    }
}

void Overlay::setVisible(bool visible) {
    this->visible = visible;
    
    if (hwnd) {
        if (visible) {
            // Use SW_SHOWNOACTIVATE to show the window without activating it
            ShowWindow(hwnd, SW_SHOWNOACTIVATE);
            
            // Make sure it's topmost and doesn't activate
            SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, 
                          SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
            
            // Double-check that window is still transparent to mouse events
            LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
            if (!(exStyle & WS_EX_TRANSPARENT)) {
                SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle | WS_EX_TRANSPARENT);
                std::cout << "Re-applying WS_EX_TRANSPARENT style" << std::endl;
            }
            
            std::cout << "Overlay shown" << std::endl;
            render(); // Immediately render when shown
        } else {
            ShowWindow(hwnd, SW_HIDE);
            std::cout << "Overlay hidden" << std::endl;
        }
    }
}

bool Overlay::isVisible() const {
    return visible;
}

bool Overlay::registerWindowClass() {
    ZeroMemory(&wc, sizeof(WNDCLASSEX));
    
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    wc.lpszClassName = OVERLAY_WINDOW_CLASS;
    
    if (!RegisterClassEx(&wc)) {
        std::cerr << "RegisterClassEx failed, error: " << GetLastError() << std::endl;
        return false;
    }
    
    return true;
}

bool Overlay::createOverlayWindow(int screenWidth, int screenHeight) {
    // Create an always-on-top, transparent, click-through window
    // WS_EX_TRANSPARENT makes the window transparent to mouse events
    // WS_EX_LAYERED allows for transparency
    // WS_EX_TOPMOST keeps it above all other windows
    // WS_EX_NOACTIVATE prevents the window from activating and stealing focus
    hwnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_NOACTIVATE,
        OVERLAY_WINDOW_CLASS,
        "TriggerBot Overlay",
        WS_POPUP,
        0, 0,
        screenWidth, screenHeight,
        nullptr,
        nullptr,
        GetModuleHandle(nullptr),
        this
    );
    
    if (!hwnd) {
        std::cerr << "Failed to create overlay window. Error: " << GetLastError() << std::endl;
        return false;
    }
    
    // Make the window click-through and transparent
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
    
    // Extra step to ensure the window is truly click-through
    // This sets the window to be transparent to hit testing, passing clicks through
    LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle | WS_EX_TRANSPARENT);
    
    // Enable the window to receive messages
    UpdateWindow(hwnd);
    
    std::cout << "Overlay window created successfully. Size: " << screenWidth << "x" << screenHeight << std::endl;
    return true;
}

void Overlay::render() const {
    if (!hwnd) {
        return;
    }

    // Get the window device context
    HDC hdc = GetDC(hwnd);
    if (!hdc) {
        return;
    }
    
    // Get window dimensions
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    
    // Simple direct drawing without double buffering
    // Clear the entire window to black (which will be transparent)
    const auto transparentBrush = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    FillRect(hdc, &clientRect, transparentBrush);
    
    // Create a thin red pen for the circle
    HPEN pen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
    const auto oldPen = static_cast<HPEN>(SelectObject(hdc, pen));
    
    // Draw a circle around the scan area
    const int radius = std::max(scanAreaX, scanAreaY) * 2; // Scale it for visibility
    
    // Create a hollow brush to ensure only the outline is drawn
    const auto hollowBrush = static_cast<HBRUSH>(GetStockObject(HOLLOW_BRUSH));
    const auto oldBrush = static_cast<HBRUSH>(SelectObject(hdc, hollowBrush));
    
    // Use Ellipse function to draw the circle outline
    Ellipse(hdc, 
            centerX - radius, 
            centerY - radius, 
            centerX + radius, 
            centerY + radius);
            
    // Restore the original brush
    SelectObject(hdc, oldBrush);
    
    // Clean up
    SelectObject(hdc, oldPen);
    DeleteObject(pen);
    ReleaseDC(hwnd, hdc);

    // Force window to stay on top
    static DWORD lastTopmost = GetTickCount();

    // Only set topmost periodically to reduce overhead
    if (const DWORD now = GetTickCount(); now - lastTopmost > 1000) {
        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, 
                    SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        lastTopmost = now;
    }
}

LRESULT CALLBACK Overlay::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
            
        // Explicitly ignore all mouse input messages
        case WM_MOUSEACTIVATE:
            return MA_NOACTIVATE; // Prevent activation on mouse click
            
        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_MOUSEWHEEL:
            return 0; // Ignore all mouse events
            
        // Prevent activation through keyboard
        case WM_ACTIVATE:
            if (LOWORD(wParam) != WA_INACTIVE) {
                // If window is being activated, immediately make it inactive
                SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, 
                             SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
                return 0;
            }
            break;
            
        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}