/*
  This is a basic driver for the SSD1320 OLED driver IC. It can initialize the display
  and do simple commands like setPixels, drawCharacters, and other simple commands.

  We can't use SPI library out of the box because the display
  requires 9-bit data in 3-wire SPI mode.
  So we bit-bang the first bit then use SPI hardware to send remaining 8 bits
*/

#include <Arduino.h>
#include <SPI.h>

#define SCLK_PIN_DEFAULT 13
#define SDOUT_PIN_DEFAULT 11
#define CS_PIN_DEFAULT 10
#define RST_PIN_DEFAULT 8

#define swap(a, b) { uint8_t t = a; a = b; b = t; }

#define BLACK 0
#define WHITE 1

#define FONTHEADERSIZE    6

#define NORM        0
#define XOR         1

#define CLEAR_ALL         0
#define CLEAR_DISPLAY     1
#define CLEAR_BUFFER      2

#define MEMORYMODE          0x20
#define SETCOLUMN           0x21
#define SETROW              0x22
#define SETPORTRAIT         0x25
#define SETCONTRAST         0x81
#define SETCHARGEPMP1       0x8D
#define SETSEGREMAP         0xA0
#define SETSTARTLINE        0xA2
#define RESETALLON          0xA4
#define DISPLAYALLON        0xA5
#define RESETINVERT         0xA6
#define INVERTDISPLAY       0xA7
#define SETMULTIPLEX        0xA8
#define SETCHARGEPMP2       0xAC
#define SELECTIREF          0xAD
#define DISPLAYOFF          0xAE
#define DISPLAYON           0xAF
#define SETPRECHARGE        0xBC
#define SETGSTABLE          0xBE
#define SETDEFAULTTABLE     0xBF
#define COMSCANINC          0xC0
#define COMSCANDEC          0xC8
#define SETDISPLAYOFFSET    0xD3
#define SETDISPLAYCLOCKDIV  0xD5
#define SETPHASELENGTH      0xD9
#define SETCOMPINS          0xDA
#define SETVCOMDESELECT     0xDB
#define SETCOMMANDLOCK      0xFD

// Scroll - It's not documented in the SSD1320 doc but we
// guessed at it from the SSD1306 doc (see MicroOLED product).
#define ACTIVATESCROLL                0x2F
#define DEACTIVATESCROLL              0x2E
#define SETVERTICALSCROLLAREA         0xA3
#define RIGHTHORIZONTALSCROLL         0x26
#define LEFTHORIZONTALSCROLL          0x27
#define VERTICALRIGHTHORIZONTALSCROLL 0x29
#define VERTICALLEFTHORIZONTALSCROLL  0x2A

typedef enum CMD {
  CMD_CLEAR,      //0
  CMD_INVERT,     //1
  CMD_CONTRAST,   //2
  CMD_DISPLAY,    //3
  CMD_SETCURSOR,    //4
  CMD_PIXEL,      //5
  CMD_LINE,     //6
  CMD_LINEH,      //7
  CMD_LINEV,      //8
  CMD_RECT,     //9
  CMD_RECTFILL,   //10
  CMD_CIRCLE,     //11
  CMD_CIRCLEFILL,   //12
  CMD_DRAWCHAR,   //13
  CMD_DRAWBITMAP,   //14
  CMD_GETLCDWIDTH,  //15
  CMD_GETLCDHEIGHT, //16
  CMD_SETCOLOR,   //17
  CMD_SETDRAWMODE   //18
} commCommand_t;

class SSD1320 : public Print {
  public:
    SSD1320(uint8_t csPin = CS_PIN_DEFAULT,
            uint8_t rstPin = RST_PIN_DEFAULT,
            uint8_t sclkPin = SCLK_PIN_DEFAULT,
            uint8_t sdoutPin = SDOUT_PIN_DEFAULT,
            SPIClass *spiInterface = &SPI);
    void begin(uint16_t, uint16_t);
    virtual size_t write(uint8_t);

    // RAW LCD functions
    void command(uint8_t cmd);
    void data(uint8_t d);
    void setColumnAddress(uint8_t address);
    void setRowAddress(uint8_t address);

    // LCD Draw functions
    void clearDisplay(uint8_t mode = CLEAR_ALL);
    void display(void);
    void setCursor(uint8_t x, uint8_t y);

    void invert(boolean inv);
    void setContrast(uint8_t contrast);
    void flipVertical(boolean flip);
    void flipHorizontal(boolean flip);

    void setPixel(uint8_t x, uint8_t y);
    void setPixel(uint8_t x, uint8_t y, uint8_t color, uint8_t mode);

    void line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
    void line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t color, uint8_t mode);
    void lineH(uint8_t x, uint8_t y, uint8_t width);
    void lineH(uint8_t x, uint8_t y, uint8_t width, uint8_t color, uint8_t mode);
    void lineV(uint8_t x, uint8_t y, uint8_t height);
    void lineV(uint8_t x, uint8_t y, uint8_t height, uint8_t color, uint8_t mode);

    void rect(uint8_t x, uint8_t y, uint8_t width, uint8_t height);
    void rect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t color , uint8_t mode);
    void rectFill(uint8_t x, uint8_t y, uint8_t width, uint8_t height);
    void rectFill(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t color , uint8_t mode);

    void circle(uint8_t x, uint8_t y, uint8_t radius);
    void circle(uint8_t x, uint8_t y, uint8_t radius, uint8_t color, uint8_t mode);
    void circleFill(uint8_t x0, uint8_t y0, uint8_t radius);
    void circleFill(uint8_t x0, uint8_t y0, uint8_t radius, uint8_t color, uint8_t mode);

    void drawChar(uint8_t x, uint8_t y, uint8_t c);
    void drawChar(uint8_t x, uint8_t y, uint8_t c, uint8_t color, uint8_t mode);

    void drawBitmap(uint8_t *bitArray);

    uint16_t getDisplayWidth(void);
    uint16_t getDisplayHeight(void);
    void setDisplayWidth(uint16_t);
    void setDisplayHeight(uint16_t);
    void setColor(uint8_t color);
    void setDrawMode(uint8_t mode);
    uint8_t *getScreenBuffer(void);

    //Font functions
    uint8_t getFontWidth(void);
    uint8_t getFontHeight(void);
    uint8_t getTotalFonts(void);
    uint8_t getFontType(void);
    boolean setFontType(uint8_t type);
    uint8_t getFontStartChar(void);
    uint8_t getFontTotalChar(void);

    // LCD Rotate Scroll functions
    void scrollRight(uint8_t start, uint8_t stop);
    void scrollLeft(uint8_t start, uint8_t stop);

    //TODO Add 0x29/0x2A vertical scrolling commands
    void scrollUp(uint8_t start, uint8_t stop);
    //void scrollVertLeft(uint8_t start, uint8_t stop);

    void scrollStop(void);

  private:
    uint8_t _sclk, _sd, _cs, _rst;
    uint8_t _interface;
    SPIClass *_spi;

    uint16_t _displayWidth, _displayHeight;

    void powerUp();
    static const unsigned char *fontsPointer[];

    uint8_t foreColor, drawMode, fontWidth, fontHeight, fontType, fontStartChar, fontTotalChar, cursorX, cursorY;
    uint16_t fontMapWidth;

    uint8_t flipByte(uint8_t thing);
};
