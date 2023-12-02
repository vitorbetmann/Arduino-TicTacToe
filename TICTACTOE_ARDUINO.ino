//missing features
//to ask if the player wants to play again
//to play preventing player win, and OTHER plays
//well played or bummer messages when the game ends
//maybe have the player pick a game for 1 or 2 and also have messages for when p1 wins or p2 wins

/* AllProtocolsOnLCD.cpp
 *
 * Modified ReceiveDemo.cpp with additional 1602 LCD output.
 * If debug button is pressed (pin connected to ground) a long output is generated.
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 ************************************************************************************
 * MIT License
 *
 * Copyright (c) 2022-2023 Armin Joachimsmeyer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 ************************************************************************************
 */

/***************************************************
  This is a library for the genric 1.8" SPI display.

This library works with the Adafruit 1.8" TFT Breakout w/SD card
  ----> http://www.adafruit.com/products/358
The 1.8" TFT shield
  ----> https://www.adafruit.com/product/802
The 1.44" TFT breakout
  ----> https://www.adafruit.com/product/2088
as well as Adafruit raw 1.8" TFT display
  ----> http://www.adafruit.com/products/618

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/
 
 /***************************************************
  This is our GFX example for the Adafruit ILI9341 Breakout and Shield
  ----> http://www.adafruit.com/products/1651

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/

#include <LiquidCrystal.h>
#include <IRremote.hpp>
#include <Adafruit_GFX.h>      // Core graphics library
#include <XTronical_ST7735.h>  // Hardware-specific library
#include <SPI.h>

// set up pins we are going to use to talk to the screen
#define TFT_SCLK 13  // SPI clock
#define TFT_MOSI 11  // SPI Data
#define TFT_CS 9     // Display enable (Chip select), if not enabled will not talk on SPI bus
#define TFT_RST -1   // Display reset pin, you can also connect this to the Arduino reset \
                     // in which case, set this #define pin to -1!
#define TFT_DC 10    // register select (stands for Data Control perhaps!)

// initialise the routine to talk to this display with these pin connections (as we've missed off
// TFT_SCLK and TFT_MOSI the routine presumes we are using hardware SPI and internally uses 13 and 11
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(3, 4, 5, 6, 7, 8);

int receiver = 2;  // Signal Pin of IR receiver to Arduino Digital Pin 5
char symb1 = 'X', symb2 = 'O';
static int posx = 0, posy = 0;

//variables
int turn = 1;
bool draw = false, assigned = false;
int aiPlay[2];
long playerNum;
char blank = ' ';

// board array
char board[3][3] = { { ' ', ' ', ' ' }, { ' ', ' ', ' ' }, { ' ', ' ', ' ' } };

/*-----( Declare objects )-----*/
IRrecv irrecv(receiver);  // create instance of 'irrecv'
//vairable uses to store the last decodedRawData
uint32_t last_decodedRawData = 0;
/*-----( Function )-----*/
void translateIR()  // takes action based on IR code received
{
  // Check if it is a repeat IR code
  if (irrecv.decodedIRData.flags) {
    //set the current decodedRawData to the last decodedRawData
    irrecv.decodedIRData.decodedRawData = last_decodedRawData;
    Serial.println("REPEAT!");
  } else {
    //output the IR code on the serial monitor
    Serial.print("IR code:0x");
    Serial.println(irrecv.decodedIRData.decodedRawData, HEX);
  }
  //map the IR code to the remote key
  switch (irrecv.decodedIRData.decodedRawData) {
    //case 0xBA45FF00: Serial.println("POWER"); break;
    case 0xB946FF00: posy -= 1; break;        // vol+
    case 0xBB44FF00: posx -= 1; break;        //fast back
    case 0xBF40FF00: assigned = true; break;  // play/pause button
    case 0xBC43FF00: posx += 1; break;        //fast forward
    case 0xF807FF00: symb1 = 'O'; break;      //donw arrow
    case 0xEA15FF00: posy += 1; break;        // vol-
    case 0xF609FF00: symb1 = 'X'; break;      //up arrow
    default:
      Serial.println(" other button   ");
  }  // End Case
  //store the last decodedRawData
  last_decodedRawData = irrecv.decodedIRData.decodedRawData;
  delay(500);  // Do not get immediate repeat
}  //END translateIR

void setup() {
  // put your setup code here, to run once:

  // Start the receiver
  irrecv.enableIRIn();

  //square lcd test
  tft.init();  // initialize a ST7735S chip,

  //print board on lcd square screen
  tft.fillScreen(ST7735_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(ST7735_WHITE);
  tft.setRotation(2);

  tft.setCursor(12, 12);
  tft.print(board[0][0]);
  tft.print(" | ");
  tft.print(board[0][1]);
  tft.print(" | ");
  tft.print(board[0][2]);

  tft.setCursor(12, 24);
  tft.print("--|---|--");

  tft.setCursor(12, 36);
  tft.print(board[1][0]);
  tft.print(" | ");
  tft.print(board[1][1]);
  tft.print(" | ");
  tft.print(board[1][2]);

  tft.setCursor(12, 48);
  tft.print("--|---|--");

  tft.setCursor(12, 60);
  tft.print(board[2][0]);
  tft.print(" | ");
  tft.print(board[2][1]);
  tft.print(" | ");
  tft.print(board[2][2]);

  //welcome message
  lcd.begin(16, 2);
  lcd.print("   Welcome to   ");
  lcd.setCursor(0, 1);
  lcd.print("  TIC-TAC-TOE!");
  delay(3000);
  lcd.clear();

  while (true) {
    // pick symbol
    lcd.setCursor(0, 0);
    lcd.print("Press UP for X");
    lcd.setCursor(0, 1);
    lcd.print(" Or DOWN for O");

    if (irrecv.decode())  // have we received an IR signal?
    {
      translateIR();
      irrecv.resume();  // receive the next value
      break;
    }
  }
  lcd.clear();


  // determines first player
  randomSeed(analogRead(A0));
  playerNum = random(1, 3);

  if (playerNum == 2) {
    if (symb1 == 'X') symb1 = 'O';
    else if (symb1 == 'O') symb1 = 'X';
    lcd.setCursor(0, 0);
    lcd.print("You play SECOND");
    lcd.setCursor(0, 1);
    lcd.print("Good Luck!");
    delay(2500);

  } else {
    lcd.setCursor(0, 0);
    lcd.print("You play FIRST");
    lcd.setCursor(0, 1);
    lcd.print("Good Luck!");
    delay(2500);
  }
}

void loop() {  // put your main code here, to run repeatedly:

  lcd.clear();

  //print board on square lcd
  tft.fillScreen(ST7735_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(ST7735_WHITE);
  tft.setRotation(2);

  tft.setCursor(12, 12);
  tft.print(board[0][0]);
  tft.print(" |");
  tft.print(board[0][1]);
  tft.print("  |");
  tft.print(board[0][2]);

  tft.setCursor(12, 24);
  tft.print("--|---|--");

  tft.setCursor(12, 36);
  tft.print(board[1][0]);
  tft.print(" |");
  tft.print(board[1][1]);
  tft.print("  |");
  tft.print(board[1][2]);

  tft.setCursor(12, 48);
  tft.print("--|---|--");

  tft.setCursor(12, 60);
  tft.print(board[2][0]);
  tft.print(" |");
  tft.print(board[2][1]);
  tft.print("  |");
  tft.print(board[2][2]);

  // victory conditions
  // horizontal
  if (board[0][0] != ' ' && board[0][0] == board[0][1] && board[0][1] == board[0][2]) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(symb1);
    lcd.print("  Won the game!");
    lcd.setCursor(0, 1);
    lcd.print("See you around");
    delay(100000);  ///////////////////////////fix those
  }
  if ((board[1][0] == 'X' || board[1][0] == 'O') && board[1][0] == board[1][1] && board[1][1] == board[1][2]) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(symb1);
    lcd.print("  Won the game!");
    lcd.setCursor(0, 1);
    lcd.print("See you around");
    delay(100000);  ///////////////////////////fix those
  }
  if (board[2][1] != ' ' && board[2][0] == board[2][1] && board[2][1] == board[2][2]) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(symb1);
    lcd.print("  Won the game!");
    lcd.setCursor(0, 1);
    lcd.print("See you around");
    delay(100000);  ///////////////////////////fix those
  }
  // vertical
  if (board[0][0] != ' ' && board[0][0] == board[1][0] && board[1][0] == board[2][0]) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(symb1);
    lcd.print("  Won the game!");
    lcd.setCursor(0, 1);
    lcd.print("See you around");
    delay(100000);  ///////////////////////////fix those
  }
  if (board[0][1] != ' ' && board[0][1] == board[1][1] && board[1][1] == board[2][1]) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(symb1);
    lcd.print("  Won the game!");
    lcd.setCursor(0, 1);
    lcd.print("See you around");
    delay(100000);  ///////////////////////////fix those
  }
  if ((board[0][2] == 'X' || board[0][2] == 'O') && board[0][2] == board[1][2] && board[1][2] == board[2][2]) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(symb1);
    lcd.print("  Won the game!");
    lcd.setCursor(0, 1);
    lcd.print("See you around");
    delay(100000);  ///////////////////////////fix those
  }
  // diagonal
  if (board[0][0] != ' ' && board[0][0] == board[1][1] && board[1][1] == board[2][2]) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(symb1);
    lcd.print("  Won the game!");
    lcd.setCursor(0, 1);
    lcd.print("See you around");
    delay(100000);  ///////////////////////////fix those
  }
  if (board[2][0] != ' ' && board[2][0] == board[1][1] && board[1][1] == board[0][2]) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(symb1);
    lcd.print("  Won the game!");
    lcd.setCursor(0, 1);
    lcd.print("See you around");
    delay(100000);  ///////////////////////////fix those
  };

  // draw condition
  if (turn == 10) {
    draw = true;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Ops, it's a draw");
    lcd.setCursor(0, 1);
    lcd.print("See you around");
    delay(100000);  ///////////////////////////fix those
  }


  if (turn != 1) {
    if (symb1 == 'X') {
      symb2 = symb1;
      symb1 = 'O';
    } else {
      symb2 = symb1;
      symb1 = 'X';
    }
  }

  // if player turn
  if ((playerNum == 1 && turn % 2 != 0) || (playerNum == 2 && turn % 2 == 0)) {
    lcd.clear();
    lcd.print(" Make your play");
    lcd.setCursor(0, 1);
    lcd.print("(Pos = RED symb)");

    //print cursor
    tft.setTextColor(ST7735_RED);
    tft.setCursor(24 + (posx * 42), 12 + (posy * 24));
    tft.print(symb1);

    assigned = false;
    // check for valid play
    while (!assigned) {

      if (irrecv.decode())  // have we received an IR signal?
      {
        translateIR();
        irrecv.resume();  // receive the next value
        if (posx > 2) posx = 0;
        if (posx < 0) posx = 2;
        if (posy > 2) posy = 0;
        if (posy < 0) posy = 2;

        //print board on square lcd
        tft.fillScreen(ST7735_BLACK);
        tft.setTextSize(2);
        tft.setTextColor(ST7735_WHITE);
        tft.setRotation(2);

        tft.setCursor(12, 12);
        tft.print(board[0][0]);
        tft.print(" |");
        tft.print(board[0][1]);
        tft.print("  |");
        tft.print(board[0][2]);

        tft.setCursor(12, 24);
        tft.print("--|---|--");

        tft.setCursor(12, 36);
        tft.print(board[1][0]);
        tft.print(" |");
        tft.print(board[1][1]);
        tft.print("  |");
        tft.print(board[1][2]);

        tft.setCursor(12, 48);
        tft.print("--|---|--");

        tft.setCursor(12, 60);
        tft.print(board[2][0]);
        tft.print(" |");
        tft.print(board[2][1]);
        tft.print("  |");
        tft.print(board[2][2]);

        //print cursor
        tft.setTextColor(ST7735_RED);
        tft.setCursor(24 + (posx * 42), 12 + (posy * 24));
        tft.print(symb1);
      }

      if (assigned == true && board[posy][posx] != ' ') {
        assigned = false;
        lcd.clear();
        lcd.print(" Not Valid play");
        lcd.setCursor(0, 1);
        lcd.print("Make another one");

      } else if (assigned == true) {
        board[posy][posx] = symb1;
      }
    }
    lcd.clear();
  }

  // if ai turn
  else {
    //variables
    bool found = false;

    //ai personality
    if (turn <= 5) {
      lcd.clear();
      lcd.print("Thinking...");
      delay(2000 / turn);
    } else if (turn <= 7) {
      lcd.clear();
      lcd.print("Not bad...");
      delay(1500);
    } else if (turn == 8) {
      lcd.clear();
      lcd.print("I won't lose");
      delay(2000);
    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Let's end this");
      lcd.setCursor(0, 1);
      delay(2000);
    }

    while (!found) {
      // try to win horizontal
      if (board[0][0] == board[0][1] && board[0][0] == symb1 && board[0][2] == ' ') {
        board[0][2] = symb1;
        break;
      } else if (board[0][0] == board[0][2] && board[0][0] == symb1 && board[0][1] == ' ') {
        board[0][1] = symb1;
        break;
      } else if (board[0][1] == board[0][2] && board[0][1] == symb1 && board[0][0] == ' ') {
        board[0][0] = symb1;
        break;
      } else if (board[1][0] == board[1][1] && board[1][0] == symb1 && board[1][2] == ' ') {
        board[1][2] = symb1;
        break;
      } else if (board[1][0] == board[1][2] && board[1][0] == symb1 && board[1][1] == ' ') {
        board[1][1] = symb1;
        break;
      } else if (board[1][1] == board[1][2] && board[1][1] == symb1 && board[1][0] == ' ') {
        board[1][0] = symb1;
        break;
      } else if (board[2][0] == board[2][1] && board[2][0] == symb1 && board[2][2] == ' ') {
        board[2][2] = symb1;
        break;
      } else if (board[2][0] == board[2][2] && board[2][0] == symb1 && board[2][1] == ' ') {
        board[2][1] = symb1;
        break;
      } else if (board[2][1] == board[2][2] && board[2][1] == symb1 && board[2][0] == ' ') {
        board[2][0] = symb1;
        break;
      }

      // try to win vertical
      else if (board[0][0] == board[1][0] && board[0][0] == symb1 && board[2][0] == ' ') {
        board[2][0] = symb1;
        break;
      } else if (board[0][0] == board[2][0] && board[0][0] == symb1 && board[1][0] == ' ') {
        board[1][0] = symb1;
        break;
      } else if (board[1][0] == board[2][0] && board[1][0] == symb1 && board[0][0] == ' ') {
        board[0][0] = symb1;
        break;
      } else if (board[0][1] == board[1][1] && board[0][1] == symb1 && board[2][1] == ' ') {
        board[2][1] = symb1;
        break;
      } else if (board[0][1] == board[2][1] && board[0][1] == symb1 && board[1][1] == ' ') {
        board[1][1] = symb1;
        break;
      } else if (board[1][1] == board[2][1] && board[1][1] == symb1 && board[0][1] == ' ') {
        board[0][1] = symb1;
        break;
      } else if (board[0][2] == board[1][2] && board[0][2] == symb1 && board[2][2] == ' ') {
        board[2][2] = symb1;
        break;
      } else if (board[0][2] == board[2][2] && board[0][2] == symb1 && board[1][2] == ' ') {
        board[1][2] = symb1;
        break;
      } else if (board[1][2] == board[2][2] && board[1][2] == symb1 && board[0][2] == ' ') {
        board[0][2] = symb1;
        break;
      }

      // try to win diagonal
      // downward
      else if (board[0][0] == board[1][1] && board[0][0] == symb1 && board[2][2] == ' ') {
        board[2][2] = symb1;
        break;
      } else if (board[0][0] == board[2][2] && board[0][0] == symb1 && board[1][1] == ' ') {
        board[1][1] = symb1;
        break;
      } else if (board[1][1] == board[2][2] && board[1][1] == symb1 && board[0][0] == ' ') {
        board[0][0] = symb1;
        break;
      }

      // upward
      else if (board[0][2] == board[1][1] && board[0][2] == symb1 && board[2][0] == ' ') {
        board[2][0] = symb1;
        break;
      } else if (board[0][2] == board[2][0] && board[0][2] == symb1 && board[1][1] == ' ') {
        board[1][1] = symb1;
        break;
      } else if (board[1][1] == board[2][0] && board[1][1] == symb1 && board[0][2] == ' ') {
        board[0][2] = symb1;
        break;
      }

      // prevent player win horizontal check this
      else if (board[0][0] == board[0][1] && board[0][0] == symb2 && board[0][2] == ' ') {
        board[0][2] = symb1;
        break;
      } else if (board[0][0] == board[0][2] && board[0][0] == symb2 && board[0][1] == ' ') {
        board[0][1] = symb1;
        break;
      } else if (board[0][1] == board[0][2] && board[0][1] == symb2 && board[0][0] == ' ') {
        board[0][0] = symb1;
        break;
      } else if (board[1][0] == board[1][1] && board[1][0] == symb2 && board[1][2] == ' ') {
        board[1][2] = symb1;
        break;
      } else if (board[1][0] == board[1][2] && board[1][0] == symb2 && board[1][1] == ' ') {
        board[1][1] = symb1;
        break;
      } else if (board[1][1] == board[1][2] && board[1][1] == symb2 && board[1][0] == ' ') {
        board[1][0] = symb1;
        break;
      } else if (board[2][0] == board[2][1] && board[2][0] == symb2 && board[2][2] == ' ') {
        board[2][2] = symb1;
        break;
      } else if (board[2][0] == board[2][2] && board[2][0] == symb2 && board[2][1] == ' ') {
        board[2][1] = symb1;
        break;
      } else if (board[2][1] == board[2][2] && board[2][1] == symb1 && board[2][0] == ' ') {
        board[2][0] = symb1;
        break;
      }

      // prevent player win vertical check this
      else if (board[0][0] == board[1][0] && board[0][0] == symb2 && board[2][0] == ' ') {
        board[2][0] = symb1;
        break;
      } else if (board[0][0] == board[2][0] && board[0][0] == symb2 && board[1][0] == ' ') {
        board[1][0] = symb1;
        break;
      } else if (board[1][0] == board[2][0] && board[1][0] == symb2 && board[0][0] == ' ') {
        board[0][0] = symb1;
        break;
      } else if (board[0][1] == board[1][1] && board[0][1] == symb2 && board[2][1] == ' ') {
        board[2][1] = symb1;
        break;
      } else if (board[0][1] == board[2][1] && board[0][1] == symb2 && board[1][1] == ' ') {
        board[1][1] = symb1;
        break;
      } else if (board[1][1] == board[2][1] && board[1][1] == symb2 && board[0][1] == ' ') {
        board[0][1] = symb1;
        break;
      } else if (board[0][2] == board[1][2] && board[0][2] == symb2 && board[2][2] == ' ') {
        board[2][2] = symb1;
        break;
      } else if (board[0][2] == board[2][2] && board[0][2] == symb2 && board[1][2] == ' ') {
        board[1][2] = symb1;
        break;
      } else if (board[1][2] == board[2][2] && board[1][2] == symb2 && board[0][2] == ' ') {
        board[0][2] = symb1;
        break;
      }

      // prevent player win diagonal
      // upwards check this
      else if (board[0][0] == board[1][1] && board[0][0] == symb2 && board[2][2] == ' ') {
        board[2][2] = symb1;
        break;
      } else if (board[0][0] == board[2][2] && board[0][0] == symb2 && board[1][1] == ' ') {
        board[1][1] = symb1;
        break;
      } else if (board[1][1] == board[2][2] && board[1][1] == symb2 && board[0][0] == ' ') {
        board[0][0] = symb1;
        break;
      }

      // downwards check this
      else if (board[0][2] == board[1][1] && board[0][2] == symb2 && board[2][0] == ' ') {
        board[2][0] = symb1;
        break;
      } else if (board[0][2] == board[2][0] && board[0][2] == symb2 && board[1][1] == ' ') {
        board[1][1] = symb1;
        break;
      } else if (board[1][1] == board[2][0] && board[1][1] == symb2 && board[0][2] == ' ') {
        board[0][2] = symb1;
        break;
      }

      //other plays check this
      else {
        for (int i = 0; i < sizeof(board) && !found; i++) {
          for (int j = 0; j < sizeof(board[i]) && !found; j++) {
            if (board[i][j] != ' ')
              continue;
            else {
              board[i][j] = symb1;
              found = true;
            }
          }
        }
      }
    }
  }
  turn++;
}
