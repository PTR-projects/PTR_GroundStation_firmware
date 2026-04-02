#pragma once

#include "Arduino.h"
#include "LORA_typedefs.h"
#include "TeleMetry.h"
#include "GNSS.h"
#include "PWR.h"
#include "lora.h"
#include <math.h>
#include "display.h"

// Abstract display interface with low-level primitives.
// High-level functions are implemented once using these primitives.
class IDisplay {
public:
    virtual ~IDisplay() = default;

    // Basic functions
    virtual void clear() = 0;
    virtual void drawString(uint16_t x, uint16_t y, const String &text, uint8_t size) = 0;
    virtual void drawSplash() = 0;
    virtual void flush() = 0;
    
    // Color management
    virtual void setColor(DisplayColor color) = 0;
    
    // Low-level primitives (must be implemented by each display)
    virtual void drawPixel(uint16_t x, uint16_t y) = 0;
    virtual void drawPixel(uint16_t x, uint16_t y, uint16_t color) = 0;  // RGB565 color
    virtual void drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) = 0;
    virtual void drawRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height) = 0;
    virtual void fillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height) = 0;
    virtual void drawCircle(uint16_t x, uint16_t y, uint16_t radius) = 0;
    virtual void fillCircle(uint16_t x, uint16_t y, uint16_t radius) = 0;
    virtual void drawBitmap(uint16_t x, uint16_t y, const uint8_t *bitmap, uint16_t width, uint16_t height) = 0;
    virtual uint16_t getWidth() = 0;
    virtual uint16_t getHeight() = 0;
    
    // Bulk buffer transfer for high performance
    virtual void drawBuffer(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint16_t* buffer) = 0;
    
    // High-level functions (implemented once using primitives)
    void drawProgressBar(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t progress);
    void drawRocketLaunch();
    void drawFinder();
    void drawCompass(int16_t x, int16_t y, float angle, float pitch, float roll);
};
