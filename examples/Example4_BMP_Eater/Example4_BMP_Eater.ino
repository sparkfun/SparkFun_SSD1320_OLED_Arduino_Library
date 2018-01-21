/*
  Control a SSD1320 based flexible OLED display
  By: Nathan Seidle
  SparkFun Electronics
  Date: November 21st, 2017
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  This example takes in a full BMP file, parses it and displays it on the OLED. This is handy
  because you don't have to convert the BMP into a prog_mem array or anything else.
  
  This is a fun but tricky example.
  1) Use Teraterm or other terminal program capable of sending a binary file.
  2) Connect at 57600bps
  3) Upload che.bmp, monkey.bmp, or whitebox.bmp
  4) Should display on the screen

  Requirements:
  * The BMP should be 160x32 pixels
  * The BMP should be in grayscale
  * You can send non-grayscale bitmaps but this program will only display the blue channel
  * You can send taller or wider bitmaps but it will be displayed incorrectly 

  To connect the display to an Arduino:
  (Arduino pin) = (Display pin)
  Pin 13 = SCLK
  11 = SDIN
  10 = !CS
  9 = !RES

  The display is 160 pixels long and 32 pixels wide
  Each 4-bit nibble is the 4-bit grayscale for that pixel
  Therefore each byte of data written to the display paints two sequential pixels
  Loops that write to the display should be 80 iterations wide and 32 iterations tall
*/

#include <SSD1320_OLED.h>

//Initialize the display with the follow pin connections
SSD1320 flexibleOLED(10, 9); //10 = CS, 9 = RES

void setup()
{
  Serial.begin(57600);
  //For this example the 8MHz Arduino Pro Mini can't do the communication with LCD,
  //and the bitmap manipulations, and serial greater than 57600bps. 
  //Other platforms may be able to go faster.

  flexibleOLED.begin(160, 32); //Display is 160 wide, 32 high

  flexibleOLED.clearDisplay(); //Clear display and buffer
}

void loop()
{
  getBitmap();
}

void getBitmap()
{
  /*
    From: #https://github.com/yy502/ePaperDisplay
    ### Sample BMP header structure, total = 70 bytes
    ### !!! little-endian !!!

    Bitmap file header 14 bytes
    42 4D          "BM"
    C6 A9 03 00    FileSize = 240,070       <= dynamic value
    00 00          Reserved
    00 00          Reserved
    46 00 00 00    Offset = 70 = 14+56

    DIB header (bitmap information header)
    BITMAPV3INFOHEADER 56 bytes
    28 00 00 00    Size = 40
    20 03 00 00    Width = 800              <= dynamic value
    58 02 00 00    Height = 600             <= dynamic value
    01 00          Planes = 1
    04 00          BitCount = 4
    00 00 00 00    compression
    00 00 00 00    SizeImage
    00 00 00 00    XPerlPerMeter
    00 00 00 00    YPerlPerMeter
    04 00 00 00    Colors used = 4
    00 00 00 00    ColorImportant
    00 00 00 00    Color definition index 0
    55 55 55 00    Color definition index 1
    AA AA AA 00    Color definition index 2
    FF FF FF 00    Color definition index 3
  */

  //Begin scanning for incoming bytes
  Serial.flush(); //Flush anything in RX buffer
  Serial.println();
  Serial.println("Waiting for 160x32 sized grayscale bitmap to be sent");
  Serial.println("Note: File must be sent in binary");

  //Get BMP header
  unsigned long fileSize = 0;
  unsigned long offset = 0;
  for (int x = 0 ; x < 14 ; x++)
  {
    while (Serial.available() == false) delay(1); //Spin and do nothing

    byte incoming = Serial.read();

    //Error check the header bytes
    if (x == 0)
    {
      if (incoming != 'B')
      {
        Serial.println("Error: This does not look like a bitmap");
        while (Serial.available())
        {
          Serial.read(); //Wait for sender to stop
          delay(100);
        }
        return;
      }
    }
    if (x == 1)
    {
      if (incoming != 'M')
      {
        Serial.println("Error: This does not look like a bitmap");
        while (Serial.available())
        {
          Serial.read(); //Wait for sender to stop
          delay(100);
        }
        return;
      }
    }

    if (x == 2) fileSize |= ((long)incoming << (8 * 0));
    if (x == 3) fileSize |= ((long)incoming << (8 * 1));
    if (x == 4) fileSize |= ((long)incoming << (8 * 2));
    if (x == 5) fileSize |= ((long)incoming << (8 * 3));

    if (x == 10) offset |= ((long)incoming << (8 * 0));
    if (x == 11) offset |= ((long)incoming << (8 * 1));
    if (x == 12) offset |= ((long)incoming << (8 * 2));
    if (x == 13) offset |= ((long)incoming << (8 * 3));
  }

  //Get BMP size and number of colors used
  unsigned long colorsUsed = 0;
  for (int x = 0 ; x < 40 ; x++)
  {
    while (Serial.available() == false) delay(1); //Spin and do nothing

    byte incoming = Serial.read();

    if (x == 32) colorsUsed |= ((long)incoming << (8 * 0));
    if (x == 33) colorsUsed |= ((long)incoming << (8 * 1));
    if (x == 34) colorsUsed |= ((long)incoming << (8 * 2));
    if (x == 35) colorsUsed |= ((long)incoming << (8 * 3));
  }

  //Get look-up-table of colors
  byte colorTable[colorsUsed]; //Create array big enough to handle # of colors
  for (int x = 0 ; x < colorsUsed ; x++)
  {
    //Each color record is 4 bytes but we only need the first
    for (int j = 0 ; j < 4 ; j++)
    {
      while (Serial.available() == false) delay(1); //Spin and do nothing
      byte incoming = Serial.read();

      if (j == 0) colorTable[x] = incoming;
    }
  }

  //Make sure the display is ready to go
  flexibleOLED.setRowAddress(0);
  flexibleOLED.setColumnAddress(0);

  //Next we have pixel data. Using the look-up-table
  //convert the pixel's color code to the actual color,
  //then down sample each 8-bit byte to a 4-bit,
  //then combine two bytes at a time into one byte
  //then send to the display
  for (unsigned long i = 0 ; i < fileSize - offset - 1 ; i += 2)
  {
    //Get the two bytes
    while (Serial.available() == false) delay(1); //Spin and do nothing
    byte byte1 = Serial.read();

    while (Serial.available() == false) delay(1); //Spin and do nothing
    byte byte2 = Serial.read();

    //Covert color code to actual color
    byte1 = colorTable[byte1];
    byte2 = colorTable[byte2];

    //Covert 8-bit value to 4 bit by removing lower 4 bits
    byte2 &= 0xF0;
    byte1 >>= 4;

    byte combined = byte1 | byte2; //Combine two bytes into one

    flexibleOLED.data(combined); //Write byte directly to display
  }

  Serial.print("fileSize: ");
  Serial.println(fileSize);
  Serial.print("offset: ");
  Serial.println(offset);
  Serial.print("colorsUsed: ");
  Serial.println(colorsUsed);

  while (Serial.available())
  {
    delay(10);
    Serial.read(); //Wait for sender to stop sending
  }

  Serial.println("All done!");
}




