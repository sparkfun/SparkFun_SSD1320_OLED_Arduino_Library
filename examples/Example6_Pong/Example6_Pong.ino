/*
  Control a SSD1320 based flexible OLED display
  By: Nathan Seidle
  SparkFun Electronics
  Date: November 21st, 2017
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  A silly pong example. Comes from Marshall's tutorial:
  https://learn.sparkfun.com/tutorials/teensyview-hookup-guide#example-prop-shield-compatible-connection

  On an 8MHz Arduino Pro Mini this is pretty painfully slow. 

  To connect the display to an Arduino:
  (Arduino pin) = (Display pin)
  Pin 13 = SCLK on display carrier
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
//Note that you should not change SCLK and MOSI because the 
//library uses hardware SPI
SSD1320 flexibleOLED(10, 9, 13, 11); //CS, RES, SCLK, MOSI(SDIN on OLED board)

void setup()
{
  Serial.begin(115200);

  flexibleOLED.begin(160, 32); //Display is 160 wide, 32 high
}

void loop()
{
  shapeExample();
}

void shapeExample()
{
  printTitle("Shapes!", 0);

  // Silly pong demo. It takes a lot of work to fake pong...
  int paddleW = 3;  // Paddle width
  int paddleH = 15;  // Paddle height
  // Paddle 0 (left) position coordinates
  int paddle0_Y = (flexibleOLED.getDisplayHeight() / 2) - (paddleH / 2);
  int paddle0_X = 2;
  // Paddle 1 (right) position coordinates
  int paddle1_Y = (flexibleOLED.getDisplayHeight() / 2) - (paddleH / 2);
  int paddle1_X = flexibleOLED.getDisplayWidth() - 3 - paddleW;
  int ball_rad = 2;  // Ball radius
  // Ball position coordinates
  int ball_X = paddle0_X + paddleW + ball_rad;
  int ball_Y = random(1 + ball_rad, flexibleOLED.getDisplayHeight() - ball_rad);//paddle0_Y + ball_rad;
  int ballVelocityX = 1;  // Ball left/right velocity
  int ballVelocityY = 1;  // Ball up/down velocity
  int paddle0Velocity = -1;  // Paddle 0 velocity
  int paddle1Velocity = 1;  // Paddle 1 velocity

  //while(ball_X >= paddle0_X + paddleW - 1)
  while ((ball_X - ball_rad > 1) &&
         (ball_X + ball_rad < flexibleOLED.getDisplayWidth() - 2))
  {
    // Increment ball's position
    ball_X += ballVelocityX;
    ball_Y += ballVelocityY;
    // Check if the ball is colliding with the left paddle
    if (ball_X - ball_rad < paddle0_X + paddleW)
    {
      // Check if ball is within paddle's height
      if ((ball_Y > paddle0_Y) && (ball_Y < paddle0_Y + paddleH))
      {
        ball_X++;  // Move ball over one to the right
        ballVelocityX = -ballVelocityX; // Change velocity
      }
    }
    // Check if the ball hit the right paddle
    if (ball_X + ball_rad > paddle1_X)
    {
      // Check if ball is within paddle's height
      if ((ball_Y > paddle1_Y) && (ball_Y < paddle1_Y + paddleH))
      {
        ball_X--;  // Move ball over one to the left
        ballVelocityX = -ballVelocityX; // change velocity
      }
    }
    // Check if the ball hit the top or bottom
    if ((ball_Y <= ball_rad) || (ball_Y >= (flexibleOLED.getDisplayHeight() - ball_rad - 1)))
    {
      // Change up/down velocity direction
      ballVelocityY = -ballVelocityY;
    }
    // Move the paddles up and down
    paddle0_Y += paddle0Velocity;
    paddle1_Y += paddle1Velocity;
    // Change paddle 0's direction if it hit top/bottom
    if ((paddle0_Y <= 1) || (paddle0_Y > flexibleOLED.getDisplayHeight() - 2 - paddleH))
    {
      paddle0Velocity = -paddle0Velocity;
    }
    // Change paddle 1's direction if it hit top/bottom
    if ((paddle1_Y <= 1) || (paddle1_Y > flexibleOLED.getDisplayHeight() - 2 - paddleH))
    {
      paddle1Velocity = -paddle1Velocity;
    }

    // Draw the Pong Field
    flexibleOLED.clearDisplay(CLEAR_BUFFER); //Save time. Only clear the local buffer.
    // Draw an outline of the screen:
    flexibleOLED.rect(0, 0, flexibleOLED.getDisplayWidth() - 1, flexibleOLED.getDisplayHeight());
    // Draw the center line
    flexibleOLED.rectFill(flexibleOLED.getDisplayWidth() / 2 - 1, 0, 2, flexibleOLED.getDisplayHeight());
    // Draw the Paddles:
    flexibleOLED.rectFill(paddle0_X, paddle0_Y, paddleW, paddleH);
    flexibleOLED.rectFill(paddle1_X, paddle1_Y, paddleW, paddleH);
    // Draw the ball:
    flexibleOLED.circle(ball_X, ball_Y, ball_rad);
    // Actually draw everything on the screen:
    flexibleOLED.display();

    //delay(25);  // Delay for visibility
  }
  
  delay(1000);
}

// Center and print a small title
// This function is quick and dirty. Only works for titles one line long.
void printTitle(String title, int font)
{
  int middleX = flexibleOLED.getDisplayWidth() / 2;
  int middleY = flexibleOLED.getDisplayHeight() / 2;

  flexibleOLED.clearDisplay();
  flexibleOLED.setFontType(font);

  // Set the cursor in the middle of the screen
  flexibleOLED.setCursor(middleX - (flexibleOLED.getFontWidth() * (title.length() / 2)),
                 middleY - (flexibleOLED.getFontHeight() / 2));

  // Print the title:
  flexibleOLED.print(title);
  flexibleOLED.display();

  delay(1500);

  flexibleOLED.clearDisplay(); //Clear everything
}
