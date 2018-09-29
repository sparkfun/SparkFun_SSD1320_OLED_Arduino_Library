/*
  This is a basic driver for the SSD1320 OLED driver IC. It can initialize the display
  and do simple commands like setPixels, drawCharacters, and other simple commands.

  We can't use SPI library out of the box because the display
  requires 9-bit data in 3-wire SPI mode.
  So we bit-bang the first bit then use SPI hardware to send remaining 8 bits
*/

#include "SSD1320_OLED.h"

// Add header of the fonts here.  Remove as many as possible to conserve FLASH memory.
#include "util/font5x7.h"
#include "util/font8x16.h"
#include "util/7segment.h"
#include "util/fontlargenumber.h"

// Change the total fonts included
#define TOTALFONTS    4

// Add the font name as declared in the header file. Remove as many as possible to conserve FLASH memory.
const unsigned char *SSD1320::fontsPointer[] = {
  font5x7,
  font8x16,
  sevensegment,
  fontlargenumber
};

// Definition of D/C# command bits
#define SPI3_COMMAND LOW   // Command bit is LOW
#define SPI3_DATA    HIGH  // Data bit is HIGH

enum {
  OLED_INTERFACE_SPI3, // 3-wire SPI interface
  OLED_INTERFACE_I2C   // I2C interface
};


/** \brief Grayscale Flexible OLED screen buffer.
  Page buffer 80 x 32 = 2,560 bytes are needed for full 4-bit grayscale. We don't have that.
  So the buffer is 1 bit per pixel or 640 pixels

  TODO - Allow for turning on/off different buffers for different targets.
  For example, if target is Teensy, allow for larger buffer that can contain grayscale

  Page buffer is required because in SPI mode the host cannot read the SSD1320's GDRAM
  of the controller.  This page buffer serves as a scratch RAM for graphical functions.
  All drawing function will first be drawn on this page buffer, only upon calling
  display() function will transfer the page buffer to the actual LCD controller's memory.
*/
static uint8_t screenMemory [] = {
  //LCD Memory organized in 20 bytes (160 columns) and 32 rows = 640 bytes

  //SparkFun Electronics Logo in 8-bit glory!

  // ROW0, BYTE0 to BYTE20
  0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00,
  0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x73 , 0x9C , 0x71 , 0x0A , 0x30,
  0xA2 , 0x71 , 0x80 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x42,
  0x10 , 0x41 , 0x0E , 0x48 , 0xE2 , 0x40 , 0x80 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00,
  0x00 , 0x00 , 0x04 , 0x62 , 0x18 , 0x41 , 0x0A , 0x48 , 0xE2 , 0x41 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00,
  0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x0C , 0x72 , 0x1C , 0x73 , 0x8E , 0x30 , 0xA2 , 0x71 , 0x80 , 0x00,
  0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x0C , 0x00 , 0x00 , 0x00 , 0x00 , 0x00,
  0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x0C , 0x00,
  0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00,
  0x00 , 0x07 , 0xCF , 0xE3 , 0xFB , 0x0C , 0x66 , 0x3D , 0xB1 , 0xC0 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00,
  0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x0F , 0xEF , 0xF7 , 0x7B , 0x0C , 0xE6 , 0x7F , 0xB1 , 0xC0 , 0x00 , 0x00,
  0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x0C , 0x6E , 0x36 , 0x3B , 0x0D , 0xC6 , 0x73,
  0xB1 , 0xC0 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x01 , 0xEC , 0x37,
  0x3B , 0x0F , 0x86 , 0x71 , 0xB1 , 0xC0 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00,
  0x00 , 0x0F , 0xCC , 0x33 , 0xFB , 0x0F , 0x86 , 0x71 , 0xB1 , 0xC0 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00,
  0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x0E , 0x0E , 0x30 , 0x3B , 0x0F , 0x86 , 0x71 , 0xB1 , 0xC0 , 0x00 , 0x00,
  0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x0E , 0xEF , 0xF7 , 0x33 , 0xED , 0xC7 , 0x71,
  0xBF , 0x80 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x07 , 0xC7 , 0xE3,
  0xF3 , 0xEC , 0xEF , 0xF1 , 0xBF , 0x80 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00,
  0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x0C , 0x06 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00,
  0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x0C , 0x87 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00,
  0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x0C , 0xC3 , 0x80,
  0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00,
  0x00 , 0x00 , 0xC0 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00,
  0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0xC0 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00,
  0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0xF0 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00,
  0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0xFF , 0x00,
  0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00,
  0x00 , 0x00 , 0xFF , 0x80 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00,
  0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0xFF , 0x80 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00,
  0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0xFF , 0xC0 , 0x00 , 0x00 , 0x00 , 0x00,
  0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x47 , 0xC0,
  0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00,
  0x00 , 0x00 , 0x07 , 0xC0 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00,
  0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x0F , 0x80 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00,
  0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x1C , 0x00 , 0x00 , 0x00 , 0x00 , 0x00,
  0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x1C , 0x00,
  0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00,
  0x00 , 0x00 , 0x0F , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00,
  0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00
};

SSD1320::SSD1320(uint8_t csPin, uint8_t rstPin, uint8_t sclkPin, uint8_t sdoutPin, SPIClass *spiInterface) {
  _cs = csPin;
  _rst = rstPin;
  _sclk = sclkPin;
  _sd = sdoutPin;

  _interface = OLED_INTERFACE_SPI3;
  _spi = spiInterface;
}

void SSD1320::begin(uint16_t lcdWidth, uint16_t lcdHeight) {
  _displayWidth = lcdWidth;
  _displayHeight = lcdHeight;

  if (_interface == OLED_INTERFACE_SPI3) {
    pinMode(_cs, OUTPUT);
    digitalWrite(_cs, HIGH); // CS = OUTPUT, init HIGH
    pinMode(_rst, OUTPUT);
    digitalWrite(_rst, LOW); // RST = OUTPUT, init LOW
    pinMode(_sclk, OUTPUT);
    digitalWrite(_sclk, LOW);// SCLK = OUTPUT, init LOW
    pinMode(_sd, OUTPUT);
    digitalWrite(_sd, LOW);  // SDIN = OUTPUT, init LOW

  } else if (_interface == OLED_INTERFACE_I2C) {
    //! TODO
  }

  setFontType(0);
  setColor(WHITE);
  setDrawMode(NORM);
  setCursor(0, 0);

  powerUp();
}

///////////////////////
// Private Functions //
///////////////////////

/** \brief Send the display a command byte

  Send a command via SPI, I2C or parallel to SSD1320 controller.
  Display is configured for 3 wire SPI. Arduino SPI does not support 9-bit SPI.
  So we bit bang the first D/C# bit and then use hardware SPI for the command
*/
void SSD1320::command(uint8_t cmd) {

  if (_interface == OLED_INTERFACE_SPI3) {
    digitalWrite(_cs, LOW);  // CS LOW

    // Send D/C# bit
    digitalWrite(_sd, SPI3_COMMAND); // Send command bit
    digitalWrite(_sclk, HIGH); // SCLK HIGH - clock in on rising edge
    digitalWrite(_sclk, LOW);  // SCLK LOW

    // Send command byte
    _spi->beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0)); //Start up SPI again
    _spi->transfer(cmd);
    _spi->endTransaction();

    _spi->end(); //We have to stop the SPI hardware before we can bit bang the 9th bit

    digitalWrite(_cs, HIGH); // CS HIGH

  } else if (_interface == OLED_INTERFACE_I2C) {
    //! TODO
  }
}

/** \brief Send the display a data byte

  Send a command via SPI, I2C or parallel to SSD1320 controller.
  Display is configured for 3 wire SPI. Arduino SPI does not support 9-bit SPI.
  So we bit bang the first D/C# bit and then use hardware SPI for the data
*/
void SSD1320::data(uint8_t cmd) {

  if (_interface == OLED_INTERFACE_SPI3) {
    digitalWrite(_cs, LOW);  // CS LOW

    // Send D/C# bit
    digitalWrite(_sd, SPI3_DATA); // Send data bit
    digitalWrite(_sclk, HIGH); // SCLK HIGH - clock in on rising edge
    digitalWrite(_sclk, LOW);  // SCLK LOW

    // Send data byte
    _spi->beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0)); //Start up SPI again
    _spi->transfer(cmd);
    _spi->endTransaction();

    _spi->end(); //We have to stop the SPI hardware before we can bit bang the 9th bit

    digitalWrite(_cs, HIGH); // CS HIGH
  } else if (_interface == OLED_INTERFACE_I2C) {
    //! TODO
  }
}

/** \brief Set SSD1320 column address.
    Send page address command and address to the SSD1320 OLED controller.

    This triple byte command specifies column start address and end address
    of the display data RAM. This command also sets the column address
    pointer to row start address.
*/
void SSD1320::setColumnAddress(uint8_t address) {
  command(SETCOLUMN); // Set column address
  command(address); //Set start address
  command((_displayWidth / 2) - 1); //There are 160 pixels but each byte is 2 pixels. We want addresses 0 to 79.
  return;
}

/** \brief Set SSD1320 row address.
    Send page address command and address to the SSD1320 OLED controller.

    This triple byte command specifies row start address and end address
    of the display data RAM. This command also sets the row address
    pointer to row start address.
*/
void SSD1320::setRowAddress(uint8_t address) {
  command(SETROW); // Set row address
  command(address); //Set start address
  command(_displayHeight - 1); //Set end address: Display has 32 rows of pixels.
  return;
}

// Execute power up sequence as diagramed on page 11 of OLED datasheet
void SSD1320::powerUp() {
  digitalWrite(_rst, LOW); // Start with display off
  delay(1); // Wait for power to stabilize

  digitalWrite(_rst, HIGH); // Set reset line high
  delayMicroseconds(3);

  command(DISPLAYOFF);       // 0xAE - Display off

  command(SETDISPLAYCLOCKDIV); // 0xD5 - Clock divide ratio/osc. freq
  command(0xC2);                     // 0xC2 - osc clock=0xC divide ratio = 0x2

  command(SETMULTIPLEX); // 0xA8 - Multiplex ratio
  command(0x1F);                     // 0x1F - 31

  command(SETDISPLAYOFFSET); // 0xD3 - Display offset
  command(0x60);                          // 0x60 - 96

  command(SETSTARTLINE); // 0xA2 - Start line
  command(0x00);                      // 0x00 - Line 0

  command(SETSEGREMAP);  // 0xA0 - Segment re-map

  command(COMSCANINC); // 0xC0 - COM Output scan direction

  command(SETCOMPINS); // 0xDA - seg pins hardware config
  command(0x12);                       // 0x12 -

  command(SETCONTRAST);    // 0x81 - Contrast control
  command(0x5A);                       // 0x5A - value between 0x00 and 0xFF

  command(SETPHASELENGTH); // 0xD9 - Pre-charge period
  command(0x22);                       // 0x22

  command(SETVCOMDESELECT);   // 0xDB - VCOMH Deselect level
  command(0x30);                       // 0x30

  command(SELECTIREF);     // 0xAD - Internal IREF Enable
  command(0x10);                       // 0x10

  command(MEMORYMODE); // 0x20 - Memory addressing mode
  command(0x00);                        // 0x00 - Horizontal

  // disable internal charge pump
  command(SETCHARGEPMP1); // 0x8D - Internal charge pump
  command(0x01);                           // 0x01
  command(SETCHARGEPMP2); // 0xAC - Internal charge pump
  command(0x00);                           // 0x00

  // set entire display on/off
  command(RESETALLON);      // 0xA4 - Display on

  // set normal/inverse display
  command(RESETINVERT);  // 0xA6 - Normal display (not inverted)

  // display on
  command(DISPLAYON);         // 0xAF - Display on

  //Set the row and column limits for this display
  //These commands also set the RAM pointer on the display to 0,0
  setColumnAddress(0);
  setRowAddress(0);
}

/** \brief Invert display.
    The WHITE color of the display will turn to BLACK and the BLACK will turn to WHITE.
*/
void SSD1320::invert(boolean inv) {
  if (inv)
    command(INVERTDISPLAY);
  else
    command(RESETINVERT);
}

/** \brief Set contrast.
    OLED contract value from 0 to 255. Note: Contrast level is not very obvious.
*/
void SSD1320::setContrast(uint8_t contrast) {
  command(SETCONTRAST); //0x81
  command(contrast);
}

/** \brief Transfer display memory.
    Bulk move the screen buffer to the SSD1320 controller's memory so that images/graphics
    drawn on the screen buffer will be displayed on the OLED.
*/
void SSD1320::display(void) {

  //Return CGRAM pointer to 0,0
  setColumnAddress(0);
  setRowAddress(0);

  for (uint8_t rows = 0 ; rows < 32 ; rows++)
  {
    for (uint8_t columns = 0 ; columns < 20 ; columns++)
    {
      uint8_t originalByte = screenMemory[(int)rows * 20 + columns];
      for (uint8_t bitNumber = 8 ; bitNumber > 0 ; bitNumber -= 2)
      {
        uint8_t newByte = 0;
        //Because our scatch buffer is too small to contain 4-bit grayscale,
        //we extrapolate 1 bit onto 4 bits so we pull in two bits to make a byte.
        //We look at each bit in the byte and change it to 0 = 0x00 and 1 = 0x0F.
        if ( (originalByte & (1 << (bitNumber - 1))) != 0) newByte |= 0x0F;
        if ( (originalByte & (1 << (bitNumber - 2))) != 0) newByte |= 0xF0;

        data(newByte);
      }
    }
  }
}

/** \brief Override Arduino's Print.
  Arduino's print overridden so that we can use oled.print().
*/
size_t SSD1320::write(uint8_t c) {
  if (c == '\n')
  {
    cursorY += fontHeight;
    cursorX = 0;
  }
  else if (c == '\r')
  {
    // skip
  }
  else
  {
    drawChar(cursorX, cursorY, c, foreColor, drawMode);
    cursorX += fontWidth + 1;
    if ((cursorX > (_displayWidth - fontWidth)))
    {
      cursorY += fontHeight;
      cursorX = 0;
    }
  }

  return 1;
}

/** \brief Draw line.
  Draw line using current fore color and current draw mode from x0,y0 to x1,y1 of the screen buffer.
*/
void SSD1320::line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
  line(x0, y0, x1, y1, foreColor, drawMode);
}

/** \brief Draw line with color and mode.
  Draw line using color and mode from x0,y0 to x1,y1 of the screen buffer.
*/
void SSD1320::line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t color, uint8_t mode) {
  uint8_t steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    swap(x0, y0);
    swap(x1, y1);
  }

  if (x0 > x1) {
    swap(x0, x1);
    swap(y0, y1);
  }

  uint8_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int8_t err = dx / 2;
  int8_t ystep;

  if (y0 < y1) {
    ystep = 1;
  } else {
    ystep = -1;
  }

  for (; x0 < x1; x0++) {
    if (steep) {
      setPixel(y0, x0, color, mode);
    } else {
      setPixel(x0, y0, color, mode);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}

/** \brief Draw horizontal line.
  Draw horizontal line using current fore color and current draw mode from x,y to x+width,y of the screen buffer.
*/
void SSD1320::lineH(uint8_t x, uint8_t y, uint8_t width) {
  line(x, y, x + width, y, foreColor, drawMode);
}

/** \brief Draw horizontal line with color and mode.
  Draw horizontal line using color and mode from x,y to x+width,y of the screen buffer.
*/
void SSD1320::lineH(uint8_t x, uint8_t y, uint8_t width, uint8_t color, uint8_t mode) {
  line(x, y, x + width, y, color, mode);
}

/** \brief Draw vertical line.
  Draw vertical line using current fore color and current draw mode from x,y to x,y+height of the screen buffer.
*/
void SSD1320::lineV(uint8_t x, uint8_t y, uint8_t height) {
  line(x, y, x, y + height, foreColor, drawMode);
}

/** \brief Draw vertical line with color and mode.
  Draw vertical line using color and mode from x,y to x,y+height of the screen buffer.
*/
void SSD1320::lineV(uint8_t x, uint8_t y, uint8_t height, uint8_t color, uint8_t mode) {
  line(x, y, x, y + height, color, mode);
}

/** \brief Draw rectangle.
  Draw rectangle using current fore color and current draw mode from x,y to x+width,y+height of the screen buffer.
*/
void SSD1320::rect(uint8_t x, uint8_t y, uint8_t width, uint8_t height) {
  rect(x, y, width, height, foreColor, drawMode);
}

/** \brief Draw rectangle with color and mode.
  Draw rectangle using color and mode from x,y to x+width,y+height of the screen buffer.
*/
void SSD1320::rect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t color , uint8_t mode) {
  uint8_t tempHeight;

  lineH(x, y, width, color, mode);
  lineH(x, y + height - 1, width, color, mode);

  tempHeight = height - 2;

  // skip drawing vertical lines to avoid overlapping of pixel that will
  // affect XOR plot if no pixel in between horizontal lines
  if (tempHeight < 1) return;

  lineV(x, y + 1, tempHeight, color, mode);
  lineV(x + width - 1, y + 1, tempHeight, color, mode);
}

/** \brief Draw filled rectangle.
  Draw filled rectangle using current fore color and current draw mode from x,y to x+width,y+height of the screen buffer.
*/
void SSD1320::rectFill(uint8_t x, uint8_t y, uint8_t width, uint8_t height) {
  rectFill(x, y, width, height, foreColor, drawMode);
}

/** \brief Draw filled rectangle with color and mode.
  Draw filled rectangle using color and mode from x,y to x+width,y+height of the screen buffer.
*/
void SSD1320::rectFill(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t color , uint8_t mode) {
  // TODO - need to optimise the memory map draw so that this function will not call pixel one by one
  for (int i = x; i < x + width; i++) {
    lineV(i, y, height, color, mode);
  }
}

/** \brief Draw circle.
    Draw circle with radius using current fore color and current draw mode at x,y of the screen buffer.
*/
void SSD1320::circle(uint8_t x0, uint8_t y0, uint8_t radius) {
  circle(x0, y0, radius, foreColor, drawMode);
}

/** \brief Draw circle with color and mode.
  Draw circle with radius using color and mode at x,y of the screen buffer.
*/
void SSD1320::circle(uint8_t x0, uint8_t y0, uint8_t radius, uint8_t color, uint8_t mode) {
  //TODO - find a way to check for no overlapping of pixels so that XOR draw mode will work perfectly
  int8_t f = 1 - radius;
  int8_t ddF_x = 1;
  int8_t ddF_y = -2 * radius;
  int8_t x = 0;
  int8_t y = radius;

  setPixel(x0, y0 + radius, color, mode);
  setPixel(x0, y0 - radius, color, mode);
  setPixel(x0 + radius, y0, color, mode);
  setPixel(x0 - radius, y0, color, mode);

  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;

    setPixel(x0 + x, y0 + y, color, mode);
    setPixel(x0 - x, y0 + y, color, mode);
    setPixel(x0 + x, y0 - y, color, mode);
    setPixel(x0 - x, y0 - y, color, mode);

    setPixel(x0 + y, y0 + x, color, mode);
    setPixel(x0 - y, y0 + x, color, mode);
    setPixel(x0 + y, y0 - x, color, mode);
    setPixel(x0 - y, y0 - x, color, mode);
  }
}

/** \brief Draw filled circle.
    Draw filled circle with radius using current fore color and current draw mode at x,y of the screen buffer.
*/
void SSD1320::circleFill(uint8_t x0, uint8_t y0, uint8_t radius) {
  circleFill(x0, y0, radius, foreColor, drawMode);
}

/** \brief Draw filled circle with color and mode.
    Draw filled circle with radius using color and mode at x,y of the screen buffer.
*/
void SSD1320::circleFill(uint8_t x0, uint8_t y0, uint8_t radius, uint8_t color, uint8_t mode) {
  // TODO - - find a way to check for no overlapping of pixels so that XOR draw mode will work perfectly
  int8_t f = 1 - radius;
  int8_t ddF_x = 1;
  int8_t ddF_y = -2 * radius;
  int8_t x = 0;
  int8_t y = radius;

  // Temporary disable fill circle for XOR mode.
  if (mode == XOR) return;

  for (uint8_t i = y0 - radius ; i <= y0 + radius ; i++) {
    setPixel(x0, i, color, mode);
  }

  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;

    for (uint8_t i = y0 - y ; i <= y0 + y ; i++) {
      setPixel(x0 + x, i, color, mode);
      setPixel(x0 - x, i, color, mode);
    }
    for (uint8_t i = y0 - x ; i <= y0 + x ; i++) {
      setPixel(x0 + y, i, color, mode);
      setPixel(x0 - y, i, color, mode);
    }
  }
}

/** \brief Draw character.
    Draw character c using current color and current draw mode at x,y.
*/
void  SSD1320::drawChar(uint8_t x, uint8_t y, uint8_t c) {
  drawChar(x, y, c, foreColor, drawMode);
}

/** \brief Draw character with color and mode.
    Draw character c using color and draw mode at x,y.
*/
void  SSD1320::drawChar(uint8_t x, uint8_t y, uint8_t c, uint8_t color, uint8_t mode) {
  // TODO - New routine to take font of any height, at the moment limited to font height in multiple of 8 pixels

  uint8_t rowsToDraw, row, tempC;
  uint8_t i, j, temp;
  uint16_t charPerBitmapRow, charColPositionOnBitmap, charRowPositionOnBitmap, charBitmapStartPosition;

  if ((c < fontStartChar) || (c > (fontStartChar + fontTotalChar - 1))) // no bitmap available for the required c
    return;

  tempC = c - fontStartChar; //Turn user's character into a byte number

  // each row (in datasheet is called a page) is 8 bits high, 16 bit high character will have 2 rows to be drawn
  rowsToDraw = fontHeight / 8; // 8 is LCD's page size, see datasheet
  if (rowsToDraw < 1) rowsToDraw = 1;

  // The following draw function can draw anywhere on the screen, but SLOW pixel by pixel draw
  if (rowsToDraw == 1) {
    for  (i = 0 ; i < fontWidth + 1 ; i++)
    {
      if (i == fontWidth) // this is done in a weird way because for 5x7 font, there is no margin, this code add a margin after col 5
        temp = 0;
      else
        temp = pgm_read_byte(fontsPointer[fontType] + FONTHEADERSIZE + (tempC * fontWidth) + i);

      //0x7F is the first vertical line of the lowercase letter h
      //The fonts are coming in upside down?
      temp = flipByte(temp);

      //Step through this line of the character checking each bit and setting a pixel
      for (j = 0 ; j < 8 ; j++)
      {
        if (temp & 0x01) {
          setPixel(x + i, y + j, color, mode);
        }
        else {
          setPixel(x + i, y + j, !color, mode);
        }

        temp >>= 1;
      }
    }
    return;
  }

  // Font height over 8 bit
  // Take character "0" ASCII 48 as example
  charPerBitmapRow = fontMapWidth / fontWidth; // 256/8 = 32 char per row
  charColPositionOnBitmap = tempC % charPerBitmapRow; // = 16
  charRowPositionOnBitmap = int(tempC / charPerBitmapRow); // = 1
  charBitmapStartPosition = (charRowPositionOnBitmap * fontMapWidth * (fontHeight / 8)) + (charColPositionOnBitmap * fontWidth) ;

  for (row = 0 ; row < rowsToDraw ; row++) {
    for (i = 0 ; i < fontWidth ; i++) {
      temp = pgm_read_byte(fontsPointer[fontType] + FONTHEADERSIZE + (charBitmapStartPosition + i + (row * fontMapWidth)));

      //The fonts are coming in upside down
      //Additionally, the large font #1 has padding at the (now) bottom that causes problems
      //The fonts really need to be updated
      temp = flipByte(temp);

      for (j = 0 ; j < 8 ; j++) {
        if (temp & 0x01) {
          setPixel(x + i, y + j + ((rowsToDraw - 1 - row) * 8), color, mode);
        }
        else {
          setPixel(x + i, y + j + ((rowsToDraw - 1 - row) * 8), !color, mode);
        }
        temp >>= 1;
      }
    }
  }
}

/*
  Draw Bitmap image on screen. The array for the bitmap can be stored in the Arduino file, 
  so user don't have to mess with the library files.
  To use, create uint8_t array that is 160x32 pixels (640 bytes). Then call .drawBitmap and pass it the array.
*/
void SSD1320::drawBitmap(uint8_t * bitArray)
{
  for (int i = 0; i < (_displayWidth * _displayHeight / 8); i++)
    screenMemory[i] = bitArray[i];
}

/** \brief Draw pixel.
  Draw pixel using the current fore color and current draw mode in the screen buffer's x,y position.
*/
void SSD1320::setPixel(uint8_t x, uint8_t y) {
  setPixel(x, y, foreColor, drawMode);
}

/** \brief Draw pixel with color and mode.
  Draw color pixel in the screen buffer's x,y position with NORM or XOR draw mode.
*/
void SSD1320::setPixel(uint8_t x, uint8_t y, uint8_t color, uint8_t mode) {
  if ((x < 0) || (x >= _displayWidth) || (y < 0) || (y >= _displayHeight))
    return;

  int byteNumber = y * (_displayWidth / 8) + (x / 8);

  if (mode == XOR)
  {
    screenMemory[byteNumber] ^= (1 << (7 - (x % 8)));
  }
  else //mode = NORM
  {
    if (color == WHITE)
    {
      screenMemory[byteNumber] |= (1 << (7 - (x % 8)));
    }
    else
    {
      screenMemory[byteNumber] &= ~(1 << (7 - (x % 8)));
    }
  }
}

/** \brief Clear screen buffer or SSD1306's memory.
    To clear both RAM and local buffer use CLEAR_ALL
    To clear GDRAM inside the LCD controller use CLEAR_DISPLAY
    To clear local buffer use CLEAR_BUFFER
*/
void SSD1320::clearDisplay(uint8_t mode)
{
  if (mode == CLEAR_DISPLAY || mode == CLEAR_ALL) //Clear the RAM on the display
  {
    //Return CGRAM pointer to 0,0
    setColumnAddress(0);
    setRowAddress(0);

    //Display is 160 pixels long and 32 pixels wide
    //Each byte paints two sequential pixels
    //Each 4-bit nibble is the 4-bit grayscale for that pixel
    //There are only 80 columns because each byte has 2 pixels
    for (int rows = 0 ; rows < _displayHeight ; rows++)
      for (int columns = 0 ; columns < (_displayWidth / 2) ; columns++)
        data(0x00);

    if (mode == CLEAR_ALL) memset(screenMemory, 0, (_displayHeight * _displayWidth / 8)); //Clear the local buffer as well
  }
  else //Clear the local buffer
  {
    memset(screenMemory, 0, (_displayHeight * _displayWidth / 8));   // (32 x 160/8) = 640 bytes in the screenMemory buffer
  }
}

/** \brief Set cursor position.
  OLED's cursor position to x,y.
*/
void SSD1320::setCursor(uint8_t x, uint8_t y) {
  cursorX = x;
  cursorY = y;
}

/** \brief Set the display height.
    Set the height of the display. This will affect the setPixel function.
*/
void SSD1320::setDisplayHeight(uint16_t H) {
  _displayHeight = H;
}

/** \brief Set the display width.
    Set the width of the display. This will affect the setPixel function.
*/
void SSD1320::setDisplayWidth(uint16_t W) {
  _displayWidth = W;
}

/** \brief Get display height.
    The height of the display returned as int.
*/
uint16_t SSD1320::getDisplayHeight(void) {
  return _displayHeight;
}

/** \brief Get display width.
    The width of the display returned as int.
*/
uint16_t SSD1320::getDisplayWidth(void) {
  return _displayWidth;
}

/** \brief Set color.
    Set the current draw's color. Only WHITE and BLACK available.
*/
void SSD1320::setColor(uint8_t color) {
  foreColor = color;
}

/** \brief Set draw mode.
    Set current draw mode with NORM or XOR.
*/
void SSD1320::setDrawMode(uint8_t mode) {
  drawMode = mode;
}

//https://forum.arduino.cc/index.php?topic=117966.0
uint8_t SSD1320::flipByte(uint8_t c)
{
  c = ((c >> 1) & 0x55) | ((c << 1) & 0xAA);
  c = ((c >> 2) & 0x33) | ((c << 2) & 0xCC);
  c = (c >> 4) | (c << 4) ;

  return c;
}

/*
  Return a pointer to the start of the RAM screen buffer for direct access.
*/
uint8_t *SSD1320::getScreenBuffer(void) {
  return screenMemory;
}

/** \brief Get font width.
    The cucrrent font's width return as byte.
*/
uint8_t SSD1320::getFontWidth(void) {
  return fontWidth;
}

/** \brief Get font height.
    The current font's height return as byte.
*/
uint8_t SSD1320::getFontHeight(void) {
  return fontHeight;
}

/** \brief Get total fonts.
    Return the total number of fonts loaded into the MicroOLED's flash memory.
*/
uint8_t SSD1320::getTotalFonts(void) {
  return TOTALFONTS;
}

/** \brief Get font type.
    Return the font type number of the current font.
*/
uint8_t SSD1320::getFontType(void) {
  return fontType;
}

/** \brief Set font type.
    Set the current font type number, ie changing to different fonts base on the type provided.
*/
boolean SSD1320::setFontType(uint8_t type) {
  if ((type >= TOTALFONTS) || (type < 0))
    return false;

  fontType = type;
  fontWidth = pgm_read_byte(fontsPointer[fontType] + 0);
  fontHeight = pgm_read_byte(fontsPointer[fontType] + 1);
  fontStartChar = pgm_read_byte(fontsPointer[fontType] + 2);
  fontTotalChar = pgm_read_byte(fontsPointer[fontType] + 3);
  fontMapWidth = (pgm_read_byte(fontsPointer[fontType] + 4) * 100) + pgm_read_byte(fontsPointer[fontType] + 5); // two bytes values into integer 16
  return true;
}

/** \brief Get font starting character.
    Return the starting ASCII character of the currnet font, not all fonts start with ASCII character 0. Custom fonts can start from any ASCII character.
*/
uint8_t SSD1320::getFontStartChar(void) {
  return fontStartChar;
}

/** \brief Get font total characters.
    Return the total characters of the current font.
*/
uint8_t SSD1320::getFontTotalChar(void) {
  return fontTotalChar;
}

/** \brief Right scrolling.
  Set row start to row stop on the OLED to scroll right.

  Scrolling in not documented for the SSD1320 but I stumbled on
  some interesting behavior. scrollRight does nothing (but is documented
  in SSD1306). scrollLeft is the one that does weird stuff.
*/
void SSD1320::scrollRight(uint8_t start, uint8_t stop) {
  if (stop < start) // stop must be larger or equal to start
    return;
  scrollStop();   // need to disable scrolling before starting to avoid memory corruption
  command(RIGHTHORIZONTALSCROLL);
  command(0x00); //Byte A - Dummy 0x00
  command(start); //Byte B - Define start page address
  command(0x07); //Byte C - Set scroll speed to 2 frames (time interval in number of frames 5/64/128/256/3/4/25/2)
  command(stop); //Byte D - Define end page address
  command(0x00); //Byte E - Dummy 0x00
  command(0xFF); //Byte F - Dummy 0xFF
  command(ACTIVATESCROLL);
}

/** \brief Left scrolling.
  Set row start to row stop on the OLED to scroll left.
*/
void SSD1320::scrollLeft(uint8_t start, uint8_t stop) {
  if (stop < start) // Stop must be larger or equal to start
    return;

  scrollStop();   // Must disable scrolling before starting to avoid memory corruption

  command(LEFTHORIZONTALSCROLL);
  command(0x00); //Dummy byte
  command(0x00); //Dummy byte
  command(start); //Define starting page address

  command(32); //Number of rows to scroll. You scan scroll part of the display

  command(stop); //Define end page address
  command(0x00);
  command(0xFF); //Speed?
  command(ACTIVATESCROLL);
}

/** \brief Left scrolling.
  Set row start to row stop on the OLED to scroll left.

  Undocumented. Doesn't yet work.
*/
void SSD1320::scrollUp(uint8_t start, uint8_t stop) {
  if (stop < start) // Stop must be larger or equal to start
    return;

  scrollStop();   // Must disable scrolling before starting to avoid memory corruption

  command(SETVERTICALSCROLLAREA);
  command(0x00); //Byte A - Set number of rows in top fixed area
  command(64); //Byte B - Set number of rows in scroll area

  command(VERTICALRIGHTHORIZONTALSCROLL);
  command(0x00); //Byte A - Dummy byte
  //command(0x00); //Dummy byte
  command(start); //Byte B - Define starting page address

  command(0); //Byte C - Number of rows to scroll. You scan scroll part of the display

  command(stop); //Byte D - Define end page address
  command(0x01); //Byte E - Vertical scrolling offset
  command(ACTIVATESCROLL);
}
/** \brief Stop scrolling.
    Stop the scrolling of graphics on the OLED.
*/
void SSD1320::scrollStop(void) {
  command(DEACTIVATESCROLL);
}

/** \brief Vertical flip.
  Flip the graphics on the OLED vertically.
*/
void SSD1320::flipVertical(boolean flip) {
  if (flip) {
    command(COMSCANINC);
  }
  else {
    command(COMSCANDEC);
  }
}

/** \brief Horizontal flip.
    Flip the graphics on the OLED horizontally.
*/
void SSD1320::flipHorizontal(boolean flip) {
  if (flip) {
    command(SETSEGREMAP | 0x01); //Set the bit
  }
  else {
    command(SETSEGREMAP & ~0x01); //Clear the bit
  }
}
