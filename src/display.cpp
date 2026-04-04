#include "Arduino.h"
#include <Wire.h>
#include <SPI.h>
#include "BOARD.h"
#include "IDisplay.h"
#include "LORA_typedefs.h"
#include "TeleMetry.h"
#include "GNSS.h"
#include "PWR.h"
#include "lora.h"
#include <math.h>
#include "Fonts.h"
#include "display.h"

// ---- Font constants ----
#define HEIGHT_POS        1
#define FIRST_CHAR_POS   2
#define CHAR_NUM_POS    3
#define JUMPTABLE_START 4
#define JUMPTABLE_BYTES 4
#define JUMPTABLE_MSB   0
#define JUMPTABLE_LSB   1
#define JUMPTABLE_SIZE  2
#define JUMPTABLE_WIDTH 3

#define pgm_read_byte(addr)   (*(const unsigned char *)(addr))

// ---- Common drawing layer ----
class DisplayRenderer {
private:
    IDisplay* _display;
    const uint8_t* currentFont = nullptr;
    uint16_t* _framebuffer;
    uint16_t _framebufferWidth;
    uint16_t _framebufferHeight;
    bool _dirty;
    DisplayColor _currentColor;
    TextAlignment _textAlignment;
    
    // Convert display color to hardware-specific color value
    uint16_t getHardwareColor(DisplayColor color) {
        switch (color) {
            case COLOR_BLACK:
                return 0x0000;  // Black
            case COLOR_WHITE:
                return 0xFFFF;  // White
            case COLOR_RED:
                return 0xF800;  // Red (RGB565)
            case COLOR_GREEN:
                return 0x07E0;  // Green (RGB565)
            case COLOR_BLUE:
                return 0x001F;  // Blue (RGB565)
            case COLOR_CYAN:
                return 0x07FF;  // Cyan (RGB565)
            case COLOR_MAGENTA:
                return 0xF81F;  // Magenta (RGB565)
            case COLOR_YELLOW:
                return 0xFFE0;  // Yellow (RGB565)
            case COLOR_ORANGE:
                return 0xFC00;  // Orange (RGB565)
            default:
                return 0xFFFF;  // Default to white
        }
    }
    
public:
    DisplayRenderer(IDisplay* display, uint16_t width, uint16_t height) 
        : _display(display), _framebufferWidth(width), _framebufferHeight(height), _dirty(false), _currentColor(COLOR_WHITE), _textAlignment(TEXT_ALIGN_LEFT) {
        _framebuffer = (uint16_t*)malloc(width * height * sizeof(uint16_t));
        if (_framebuffer) {
            clear();
        }
    }
    
    ~DisplayRenderer() {
        if (_framebuffer) {
            free(_framebuffer);
        }
    }
    
    void setFont(const uint8_t* font) {
        currentFont = font;
    }
    
    DisplayColor getColor() {
        return _currentColor;
    }

    void setTextAlignment(TextAlignment alignment) {
        _textAlignment = alignment;
    }

    void setColor(DisplayColor color) {
        _currentColor = color;
    }
    
    void setPixel(int16_t x, int16_t y) {
        setPixel(x, y, _currentColor);
    }
    
    void setPixel(int16_t x, int16_t y, DisplayColor color) {
        if (!_framebuffer) return;
        if (x >= 0 && x < _framebufferWidth && y >= 0 && y < _framebufferHeight) {
            _framebuffer[y * _framebufferWidth + x] = getHardwareColor(color);
            _dirty = true;
        }
    }
    
    void clear() {
        clear(COLOR_BLACK);
    }
    
    void clear(DisplayColor color) {
        if (_framebuffer) {
            uint16_t hardwareColor = getHardwareColor(color);
            // Fill entire framebuffer with the specified color
            for (uint32_t i = 0; i < _framebufferWidth * _framebufferHeight; i++) {
                _framebuffer[i] = hardwareColor;
            }
            _dirty = true;
        }
    }
    
    // Flush framebuffer to display
    void flush() {
        if (!_framebuffer || !_dirty) return;
        
        // Send entire framebuffer in single transaction for high performance
        _display->drawBuffer(0, 0, _framebufferWidth, _framebufferHeight, _framebuffer);
        
        _dirty = false;
    }
    
    // Get framebuffer dimensions
    uint16_t getWidth() { return _framebufferWidth; }
    uint16_t getHeight() { return _framebufferHeight; }
    
    // Template function to draw single character
    void drawInternal(int16_t xMove, int16_t yMove,
                  int16_t width, int16_t height,
                  const uint8_t *data,
                  uint16_t offset,
                  uint16_t bytesInData){
    if (width <= 0 || height <= 0) return;

    int16_t screenW = getWidth();
    int16_t screenH = getHeight();

    // Clipping (simple early reject)
    if (xMove >= screenW || yMove >= screenH) return;
    if (xMove + width  <= 0 || yMove + height <= 0) return;

    uint8_t rasterHeight = (height + 7) >> 3; // ceil(height / 8)

    if (bytesInData == 0) {
        bytesInData = width * rasterHeight;
    }

    for (int16_t x = 0; x < width; x++) {
        for (int16_t page = 0; page < rasterHeight; page++) {

            uint16_t i = x * rasterHeight + page;
            if (i >= bytesInData) return;

            uint8_t byte = pgm_read_byte(data + offset + i);

            for (int bit = 0; bit < 8; bit++) {
                int16_t y = page * 8 + bit;
                if (y >= height) break;

                if (byte & (1 << bit)) {
                    int16_t sx = xMove + x;
                    int16_t sy = yMove + y;

                    if (sx >= 0 && sx < screenW &&
                        sy >= 0 && sy < screenH) {

                        setPixel(sx, sy);
                    }
                }
            }
        }
    }
}
    
    // Draw a single character at position (x, y) — top-left corner
    // Returns width of character (for cursor advancement)
    uint8_t drawChar(int16_t x, int16_t y, unsigned char c) {
        if (!currentFont) return 0;

        uint8_t textHeight  = pgm_read_byte(currentFont + HEIGHT_POS);
        uint8_t firstChar   = pgm_read_byte(currentFont + FIRST_CHAR_POS);
        uint8_t charCount   = pgm_read_byte(currentFont + CHAR_NUM_POS);
        uint16_t sizeOfJumpTable = pgm_read_byte(currentFont + CHAR_NUM_POS)  * JUMPTABLE_BYTES;

        if (c < firstChar || c >= firstChar + charCount) {
            return 0; // character not in font
        }

        uint8_t charIndex = c - firstChar;

        // Read jump table entry
        uint16_t jumpOffset = JUMPTABLE_START + charIndex * JUMPTABLE_BYTES;

        uint8_t msb       = pgm_read_byte(currentFont + jumpOffset + JUMPTABLE_MSB);
        uint8_t lsb       = pgm_read_byte(currentFont + jumpOffset + JUMPTABLE_LSB);
        uint8_t dataSize  = pgm_read_byte(currentFont + jumpOffset + JUMPTABLE_SIZE);
        uint8_t charWidth = pgm_read_byte(currentFont + jumpOffset + JUMPTABLE_WIDTH);

        // 0xFFFF means "no glyph defined" (space or missing char)
        if (msb == 255 && lsb == 255) {
            return charWidth;   // still advance cursor
        }

        uint16_t charDataOffset = JUMPTABLE_START + sizeOfJumpTable + ((msb << 8) + lsb);

        drawInternal(x, y, charWidth, textHeight, currentFont, charDataOffset, dataSize);

        return charWidth;
    }
    
    uint16_t getTextWidth(const String &text, uint8_t size) {
        uint16_t totalWidth = 0;
        
        for (uint16_t i = 0; i < text.length(); i++) {
            char c = text.charAt(i);
            uint8_t charWidth = getCharWidth(c);
            
            if (size > 1) {
                totalWidth += charWidth * size;
            } else {
                totalWidth += charWidth;
            }
        }
        
        return totalWidth;
    }
    
    uint8_t getCharWidth(char c) {
        if (!currentFont) return 0;
        
        uint8_t firstChar = pgm_read_byte(currentFont + FIRST_CHAR_POS);
        uint8_t charCount = pgm_read_byte(currentFont + CHAR_NUM_POS);
        
        if (c < firstChar || c >= firstChar + charCount) {
            return 0;
        }
        
        uint8_t charIndex = c - firstChar;
        uint16_t jumpOffset = JUMPTABLE_START + charIndex * JUMPTABLE_BYTES;
        
        uint8_t charWidth = pgm_read_byte(currentFont + jumpOffset + JUMPTABLE_WIDTH);
        return charWidth;
    }
    
    void drawString(int16_t x, int16_t y, const String &text, uint8_t size) {
        int16_t cursorX = x;
        int16_t cursorY = y;
        
        // Handle text alignment
        if (_textAlignment == TEXT_ALIGN_RIGHT) {
            uint16_t textWidth = getTextWidth(text, size);
            cursorX = x - textWidth;
        }
        
        for (uint16_t i = 0; i < text.length(); i++) {
            char c = text.charAt(i);
            uint8_t charWidth = drawChar(cursorX, cursorY, c);
            
            if (size > 1) {
                // For size > 1, we need to scale the character
                // This is a simplified approach - could be optimized
                for (uint8_t sy = 1; sy < size; sy++) {
                    for (uint8_t sx = 1; sx < size; sx++) {
                        drawChar(cursorX + sx, cursorY + sy, c);
                    }
                }
                cursorX += charWidth * size;
            } else {
                cursorX += charWidth;
            }
            
            // Check if we've exceeded screen width
            if (cursorX >= _display->getWidth()) {
                break;
            }
        }
    }
    
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
        // Bresenham's line algorithm
        int16_t dx = abs(x1 - x0);
        int16_t dy = abs(y1 - y0);
        int16_t sx = (x0 < x1) ? 1 : -1;
        int16_t sy = (y0 < y1) ? 1 : -1;
        int16_t err = dx - dy;
        
        while (true) {
            setPixel(x0, y0);
            
            if (x0 == x1 && y0 == y1) break;
            
            int16_t e2 = 2 * err;
            if (e2 > -dy) {
                err -= dy;
                x0 += sx;
            }
            if (e2 < dx) {
                err += dx;
                y0 += sy;
            }
        }
    }
    
    void drawRect(int16_t x, int16_t y, int16_t width, int16_t height) {
        // Draw horizontal lines
        for (int16_t i = x; i < x + width; i++) {
            setPixel(i, y);
            setPixel(i, y + height - 1);
        }
        // Draw vertical lines
        for (int16_t i = y; i < y + height; i++) {
            setPixel(x, i);
            setPixel(x + width - 1, i);
        }
    }
    
    void fillRect(int16_t x, int16_t y, int16_t width, int16_t height) {
        for (int16_t j = y; j < (y + height); j++) {
            for (int16_t i = x; i < (x + width); i++) {
                setPixel(i, j);
            }
        }
    }
    
    void drawCircle(int16_t x, int16_t y, int16_t radius) {
        // Midpoint circle algorithm
        int16_t f = 1 - radius;
        int16_t ddF_x = 1;
        int16_t ddF_y = -2 * radius;
        int16_t x0 = 0;
        int16_t y0 = radius;
        
        while (x0 <= y0) {
            // Draw 8 octants
            setPixel(x + x0, y + y0);
            setPixel(x - x0, y + y0);
            setPixel(x + x0, y - y0);
            setPixel(x - x0, y - y0);
            setPixel(x + y0, y + x0);
            setPixel(x - y0, y + x0);
            setPixel(x + y0, y - x0);
            setPixel(x - y0, y - x0);
            
            if (f >= 0) {
                y0--;
                ddF_y += 2;
                f += ddF_y;
            }
            x0++;
            ddF_x += 2;
            f += ddF_x;
        }
    }
    
    void fillCircle(int16_t x, int16_t y, int16_t radius) {
        // Fill circle using scanlines
        for (int16_t j = -radius; j <= radius; j++) {
            for (int16_t i = -radius; i <= radius; i++) {
                if (i * i + j * j <= radius * radius) {
                    setPixel(x + i, y + j);
                }
            }
        }
    }
    
    void drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t width, int16_t height) {
        for (int16_t j = 0; j < height; j++) {
            for (int16_t i = 0; i < width; i++) {
                uint16_t byteIndex = (j * width + i) / 8;
                uint8_t bitIndex = 7 - ((j * width + i) % 8);
                if (bitmap[byteIndex] & (1 << bitIndex)) {
                    setPixel(x + i, y + j);
                }
            }
        }
    }
};

// ---- Concrete display drivers ----------------------------------------
// Each class owns its hardware object and encapsulates driver-specific
// details: color constants, flush semantics, init sequence, etc.
// None of this leaks into the public API.

#if DISP_ST7735_160_80
#include <Adafruit_ST7735.h>

class ST7735_Display : public IDisplay {
private:
    Adafruit_ST7735 *_drv;
    DisplayRenderer* _renderer;
    
    // Uniform display - no shift required
    uint16_t offsetY(uint16_t y) {
        return y;
    }
    
public:
    ST7735_Display() {
        pinMode(LCD_LED, OUTPUT);
        pinMode(VEXT_EN_PIN, OUTPUT);   // TODO: move to PWR module
        digitalWrite(VEXT_EN_PIN, HIGH);
        digitalWrite(LCD_LED, HIGH);
        delay(10);
        _drv = new Adafruit_ST7735(LCD_CS, LCD_RS, LCD_MOSI, LCD_SCLK, LCS_RES);
        _drv->initR(INITR_MINI160x80_PLUGIN);
        _drv->setRotation(1);
        
        _renderer = new DisplayRenderer(this, 160, 80);
        _renderer->setFont(ArialMT_Plain_10);
    }
    
    ~ST7735_Display() {
        delete _renderer;
        delete _drv;
    }
    
    void clear() override {
        _renderer->clear();
    }
    
    void drawString(uint16_t x, uint16_t y, const String &text, uint8_t size) override {
        _renderer->drawString(x, offsetY(y), text, size);
    }
    
    void setColor(DisplayColor color) override {
        _renderer->setColor(color);
    }
    
    void setTextAlignment(TextAlignment alignment) override {
        _renderer->setTextAlignment(alignment);
    }
    
    void drawSplash() override {
        // TODO: add colour splash bitmap for TFT
    }
    
    void flush() override {
        _renderer->flush();
    }
    
    // Low-level primitives - only pixel operations
    void drawPixel(uint16_t x, uint16_t y) override {
        _drv->drawPixel(x, offsetY(y), ST77XX_WHITE);
    }
    
    void drawPixel(uint16_t x, uint16_t y, uint16_t color) override {
        _drv->drawPixel(x, offsetY(y), ST77XX_WHITE);
    }
    
    void drawBuffer(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint16_t* buffer) override {
        // Use startWrite/endWrite for better performance
        _drv->startWrite();
        
        // Set address window for bulk transfer
        _drv->setAddrWindow(x, offsetY(y), width, height);
        
        // Send pixels one by one but with optimized SPI transaction
        for (uint32_t i = 0; i < width * height; i++) {
            _drv->spiWrite(buffer[i] >> 8);      // High byte
            _drv->spiWrite(buffer[i] & 0xFF);     // Low byte
        }
        
        _drv->endWrite();
    }
    
    void drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) override {
        _renderer->drawLine(x0, offsetY(y0), x1, offsetY(y1));
    }
    
    void drawRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height) override {
        _renderer->drawRect(x, offsetY(y), width, height);
    }
    
    void fillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height) override {
        _renderer->fillRect(x, offsetY(y), width, height);
    }
    
    void drawCircle(uint16_t x, uint16_t y, uint16_t radius) override {
        _renderer->drawCircle(x, offsetY(y), radius);
    }
    
    void fillCircle(uint16_t x, uint16_t y, uint16_t radius) override {
        _renderer->fillCircle(x, offsetY(y), radius);
    }
    
    void drawBitmap(uint16_t x, uint16_t y, const uint8_t *bitmap, uint16_t width, uint16_t height) override {
        _renderer->drawBitmap(x, offsetY(y), bitmap, width, height);
    }
    
    uint16_t getWidth() override {
        return 160;
    }
    
    uint16_t getHeight() override {
        return 80;
    }
};
#endif // DISP_ST7735_160_80


#if (DISP_SSD1306_128_64 || DISP_SH110X_128_64)
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_SH110X.h>
#include "OLED.h"   // splash bitmap

#if DISP_SSD1306_128_64
class SSD1306_Display : public IDisplay {
private:
    Adafruit_SSD1306 *_drv;
    DisplayRenderer* _renderer;
        
    // Apply Y offset for lines with Y > 10
    uint16_t offsetY(uint16_t y) {
        return (y > 10) ? y - 4 : y;
    }
        
public:
    SSD1306_Display() {
        _drv = new Adafruit_SSD1306(128, 64, &Wire, OLED_RST);
        _drv->begin(SSD1306_SWITCHCAPVCC, DISPLAY_ADDR);
        delay(100);
            
        _renderer = new DisplayRenderer(this, 128, 64);
        _renderer->setFont(ArialMT_Plain_10);
    }
        
    ~SSD1306_Display() {
        delete _renderer;
        delete _drv;
    }
        
    void clear() override {
        _renderer->clear();
    }
        
    void drawString(uint16_t x, uint16_t y, const String &text, uint8_t size) override {
        _renderer->drawString(x, offsetY(y), text, size);
    }
    
    void setColor(DisplayColor color) override {
        _renderer->setColor(color);
    }
    
    void setTextAlignment(TextAlignment alignment) override {
        _renderer->setTextAlignment(alignment);
    }
        
    void drawSplash() override {
        _drv->drawBitmap(0, 0, splash, 128, 64, SSD1306_WHITE);
        flush();
    }
        
    void flush() override {
        _renderer->flush();
        _drv->display();
    }
        
    // Low-level primitives - only pixel operations
    void drawPixel(uint16_t x, uint16_t y) override {
        _drv->drawPixel(x, offsetY(y), SSD1306_WHITE);
    }
    
    void drawPixel(uint16_t x, uint16_t y, uint16_t color) override {
        // Monochrome display - treat all non-black colors as white
        if (color != 0x0000) {
            _drv->drawPixel(x, offsetY(y), SSD1306_WHITE);
        }
    }
    
    void drawBuffer(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint16_t* buffer) override {
        // Convert RGB565 buffer to monochrome and send in pages
        uint8_t pageHeight = 8;  // OLED displays are organized in 8-pixel pages
        
        for (uint16_t page = 0; page < (height + pageHeight - 1) / pageHeight; page++) {
            uint16_t pageY = y + page * pageHeight;
            
            // Set page and column address
            _drv->setCursor(x, pageY / 8);
            
            // Process one row of pixels (8 pixels high)
            for (uint16_t col = 0; col < width; col++) {
                uint8_t pageByte = 0;
                
                // Process 8 pixels vertically
                for (uint8_t bit = 0; bit < pageHeight; bit++) {
                    uint16_t pixelY = page * pageHeight + bit;
                    if (pixelY < height) {
                        uint16_t pixelIndex = pixelY * width + col;
                        uint16_t pixelColor = buffer[pixelIndex];
                        
                        // Convert RGB565 to grayscale (simple method)
                        uint8_t r = (pixelColor >> 11) & 0x1F;
                        uint8_t g = (pixelColor >> 5) & 0x3F;
                        uint8_t b = pixelColor & 0x1F;
                        uint8_t gray = (r * 29 + g * 150 + b * 29) >> 8;  // Weighted grayscale
                        
                        // Set bit if pixel is bright enough
                        if (gray > 64) {  // Threshold for white/black
                            pageByte |= (1 << bit);
                        }
                    }
                }
                
                // Send the page byte
                _drv->write(pageByte);
            }
        }
    }
        
    void drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) override {
        _renderer->drawLine(x0, offsetY(y0), x1, offsetY(y1));
    }
        
    void drawRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height) override {
        _renderer->drawRect(x, offsetY(y), width, height);
    }
        
    void fillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height) override {
        _renderer->fillRect(x, offsetY(y), width, height);
    }
        
    void drawCircle(uint16_t x, uint16_t y, uint16_t radius) override {
        _renderer->drawCircle(x, offsetY(y), radius);
    }
        
    void fillCircle(uint16_t x, uint16_t y, uint16_t radius) override {
        _renderer->fillCircle(x, offsetY(y), radius);
    }
        
    void drawBitmap(uint16_t x, uint16_t y, const uint8_t *bitmap, uint16_t width, uint16_t height) override {
        _renderer->drawBitmap(x, offsetY(y), bitmap, width, height);
    }
        
    uint16_t getWidth() override {
        return 128;
    }
        
    uint16_t getHeight() override {
        return 64;
    }
};
#endif // DISP_SSD1306_128_64


#if DISP_SH110X_128_64
class SH110X_Display : public IDisplay {
private:
    Adafruit_SH1106G *_drv;
    DisplayRenderer* _renderer;
    
    // Uniform display - no shift required
    uint16_t offsetY(uint16_t y) {
        return y;
    }
    
public:
    SH110X_Display() {
        _drv = new Adafruit_SH1106G(128, 64, &Wire, OLED_RST);
        _drv->begin(DISPLAY_ADDR);
        delay(100);
        
        _renderer = new DisplayRenderer(this, 128, 64);
        _renderer->setFont(ArialMT_Plain_10);
    }
    
    ~SH110X_Display() {
        delete _renderer;
        delete _drv;
    }
    
    void clear() override {
        _renderer->clear();
    }
    
    void drawString(uint16_t x, uint16_t y, const String &text, uint8_t size) override {
        _renderer->drawString(x, offsetY(y), text, size);
    }
    
    void setColor(DisplayColor color) override {
        _renderer->setColor(color);
    }
    
    void setTextAlignment(TextAlignment alignment) override {
        _renderer->setTextAlignment(alignment);
    }
    
    void drawSplash() override {
        _drv->drawBitmap(0, 0, splash, 128, 64, SH110X_WHITE);
        flush();
    }
    
    void flush() override {
        _renderer->flush();
        _drv->display();
    }
    
    // Low-level primitives - only pixel operations
    void drawPixel(uint16_t x, uint16_t y) override {
        _drv->drawPixel(x, offsetY(y), SH110X_WHITE);
    }
    
    void drawPixel(uint16_t x, uint16_t y, uint16_t color) override {
        // Monochrome display - treat all non-black colors as white
        if (color != 0x0000) {
            _drv->drawPixel(x, offsetY(y), SH110X_WHITE);
        }
    }
    
    void drawBuffer(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint16_t* buffer) override {
        // Convert RGB565 buffer to monochrome and send in pages
        uint8_t pageHeight = 8;  // OLED displays are organized in 8-pixel pages
        
        for (uint16_t page = 0; page < (height + pageHeight - 1) / pageHeight; page++) {
            uint16_t pageY = y + page * pageHeight;
            
            // Set page and column address
            _drv->setCursor(x, pageY / 8);
            
            // Process one row of pixels (8 pixels high)
            for (uint16_t col = 0; col < width; col++) {
                uint8_t pageByte = 0;
                
                // Process 8 pixels vertically
                for (uint8_t bit = 0; bit < pageHeight; bit++) {
                    uint16_t pixelY = page * pageHeight + bit;
                    if (pixelY < height) {
                        uint16_t pixelIndex = pixelY * width + col;
                        uint16_t pixelColor = buffer[pixelIndex];
                        
                        // Convert RGB565 to grayscale (simple method)
                        uint8_t r = (pixelColor >> 11) & 0x1F;
                        uint8_t g = (pixelColor >> 5) & 0x3F;
                        uint8_t b = pixelColor & 0x1F;
                        uint8_t gray = (r * 29 + g * 150 + b * 29) >> 8;  // Weighted grayscale
                        
                        // Set bit if pixel is bright enough
                        if (gray > 64) {  // Threshold for white/black
                            pageByte |= (1 << bit);
                        }
                    }
                }
                
                // Send the page byte
                _drv->write(pageByte);
            }
        }
    }
    
    void drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) override {
        _renderer->drawLine(x0, offsetY(y0), x1, offsetY(y1));
    }
    
    void drawRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height) override {
        _renderer->drawRect(x, offsetY(y), width, height);
    }
    
    void fillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height) override {
        _renderer->fillRect(x, offsetY(y), width, height);
    }
    
    void drawCircle(uint16_t x, uint16_t y, uint16_t radius) override {
        _renderer->drawCircle(x, offsetY(y), radius);
    }
    
    void fillCircle(uint16_t x, uint16_t y, uint16_t radius) override {
        _renderer->fillCircle(x, offsetY(y), radius);
    }
    
    void drawBitmap(uint16_t x, uint16_t y, const uint8_t *bitmap, uint16_t width, uint16_t height) override {
        _renderer->drawBitmap(x, offsetY(y), bitmap, width, height);
    }
    
    uint16_t getWidth() override {
        return 128;
    }
    
    uint16_t getHeight() override {
        return 64;
    }
};
#endif // DISP_SH110X_128_64
#endif // (DISP_SSD1306_128_64 || DISP_SH110X_128_64)


// ---- C API -----------------------------------------------------------
// Thin wrappers. All driver-specific logic lives in the classes above.

static IDisplay *g_display = nullptr;
static unsigned long Display_previousMillis = 0;
static const unsigned long Display_interval = 1000;

void Display_init(String model) {
    Serial.print("Display init start...  ");
    Serial.print("Type: ");
    Serial.println(model);

#if DISP_ST7735_160_80
    if (model == "ST7735") {
        g_display = new ST7735_Display();
        g_display->clear();
        g_display->flush();  // Clear framebuffer and send to display
        Serial.println("Display init done!");
        return;
    }
#endif

#if DISP_SSD1306_128_64
    if (model == "SSD1306") {
        g_display = new SSD1306_Display();
        g_display->clear();
        g_display->flush();  // Clear framebuffer and send to display
        Serial.println("Display init done!");
        return;
    }
#endif

#if DISP_SH110X_128_64
    if (model == "SH110X") {
        g_display = new SH110X_Display();
        g_display->clear();
        g_display->flush();  // Clear framebuffer and send to display
        Serial.println("Display init done!");
        return;
    }
#endif

    Serial.println("Display init failed!!!");
}

void Display_clear() {
    if (g_display) g_display->clear();
}

void Display_drawString(uint16_t x, uint16_t y, const String &text) {
    if (g_display) g_display->drawString(x, y, text, 1);
}

void Display_drawLargeString(uint16_t x, uint16_t y, const String &text) {
    if (g_display) g_display->drawString(x, y, text, 2);
}

void Display_drawSplash() {
    if (g_display) g_display->drawSplash();
}

void Display_refresh() {
    unsigned long now = millis();
    if ((now - Display_previousMillis) > Display_interval) {
        Display_previousMillis = now;
        if (g_display) {
            g_display->clear();
            g_display->drawRocketLaunch();
            g_display->flush();
        }
    }
}

void Display_drawProgressBar(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t progress) {
    if (g_display) g_display->drawProgressBar(x, y, width, height, progress);
}

void Display_drawRocketLaunch() {
    if (g_display) g_display->drawRocketLaunch();
}

void Display_drawFinder() {
    if (g_display) g_display->drawFinder();
}

void Display_drawCompass(int16_t x, int16_t y, float angle, float pitch, float roll) {
    if (g_display) g_display->drawCompass(x, y, angle, pitch, roll);
}

void Display_fillCircle(uint16_t x, uint16_t y, uint16_t radius) {
    if (g_display) g_display->fillCircle(x, y, radius);
}

// Color functions
void Display_setColor(DisplayColor color) {
    if (g_display) g_display->setColor(color);
}

DisplayColor Display_getColor() {
    // Note: This would require access to renderer state
    // For now, return default color
    return COLOR_WHITE;
}

// Text functions
void Display_setTextAlignment(TextAlignment alignment) {
    if (g_display) g_display->setTextAlignment(alignment);
}

void Display_setPixel(uint16_t x, uint16_t y) {
    if (g_display) g_display->drawPixel(x, y);
}

void Display_setPixel(uint16_t x, uint16_t y, DisplayColor color) {
    if (!g_display) return;
    
    // Convert DisplayColor to RGB565 value
    uint16_t rgb565Color;
    switch (color) {
        case COLOR_BLACK:
            rgb565Color = 0x0000;
            break;
        case COLOR_WHITE:
            rgb565Color = 0xFFFF;
            break;
        case COLOR_RED:
            rgb565Color = 0xF800;
            break;
        case COLOR_GREEN:
            rgb565Color = 0x07E0;
            break;
        case COLOR_BLUE:
            rgb565Color = 0x001F;
            break;
        default:
            rgb565Color = 0xFFFF;
            break;
    }
    
    g_display->drawPixel(x, y, rgb565Color);
}

void Display_clear(DisplayColor color) {
    if (!g_display) return;
    
    // For monochrome displays, only black and white matter
    if (color == COLOR_BLACK) {
        g_display->clear();
    } else {
        // For colored clear, we'd need to implement this in the display drivers
        // For now, fall back to regular clear
        g_display->clear();
    }
}
