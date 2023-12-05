//missing features
//turn win conditions into method
//for ai to play OTHER plays
//victory on last turn counts as draw
//maybe have the player pick a game for 1 or 2 players and also have messages for when p1 wins or p2 wins

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

#include <LiquidCrystal.h>     //16 x 2  LCD
#include <IRremote.hpp>        //remote control
#include <Adafruit_GFX.h>      // Core graphics library
#include <XTronical_ST7735.h>  // Hardware-specific library
#include <SPI.h>

//set pins
#define receiver 2  // Signal Pin of IR receiver

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(3, 4, 5, 6, 7, 8);

// set up pins we are going to use to talk to the screen
#define TFT_RST -1  // Display reset pin (-1 if connected to reset)
#define TFT_DC 10   // register select (stands for Data Control perhaps!)
#define TFT_CS 9    // Display enable

//Class below presumes we are using hardware SPI and internally uses:
// SPI clock to pin 13
// SPI Data to pin 11
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

//variables
bool assigned = false;
int turn = 1;
long playerNum;
char symb1 = '$', symb2 = 'O';
static int posx = 0, posy = 0;

//Declare objects
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
    case 0xB946FF00: posy -= 1; break;        // vol+                 moves cursor up
    case 0xBB44FF00: posx -= 1; break;        //fast back             moves cursor left
    case 0xBF40FF00: assigned = true; break;  // play/pause button    confirms play
    case 0xBC43FF00: posx += 1; break;        //fast forward          moves cursor right
    case 0xF807FF00: symb1 = 'O'; break;      //donw arrow            picks symbol
    case 0xEA15FF00: posy += 1; break;        // vol-                 moves cursor down
    case 0xF609FF00: symb1 = 'X'; break;      //up arrow              picks symbol
    default:
      lcd.print("Invalid button");
  }  // End Case
  //store the last decodedRawData
  last_decodedRawData = irrecv.decodedIRData.decodedRawData;
  delay(500);  // Do not get immediate repeat
}  //END translateIR

////////////////////////////////////
void setup() {
  Serial.begin(9600);

  // Start
  irrecv.enableIRIn();  //the receiver
  tft.init();           //the ST7735S chip (128x128 lcd)
  lcd.begin(16, 2);     //the 16x2 lcd


  //set configurations for lcd screen
  tft.fillScreen(ST7735_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(ST7735_WHITE);
  tft.setRotation(2);  //the way I assembled it, the lcd is upside down

  // //draw board
  tft.setCursor(12, 12);
  tft.print("  |   |  ");

  tft.setCursor(12, 24);
  tft.print("--|---|--");

  tft.setCursor(12, 36);
  tft.print("  |   |  ");

  tft.setCursor(12, 48);
  tft.print("--|---|--");

  tft.setCursor(12, 60);
  tft.print("  |   |  ");

  //welcome message
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
      if (symb1 != '$')
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
  Serial.print("turn ");
  Serial.println(turn);

  //variables
  bool playerTurn = (playerNum == 1 && turn % 2 != 0) || (playerNum == 2 && turn % 2 == 0);
  static int playerScore = 0, aiScore = 0;
  int count = 0;
  bool isDraw = false;

  //board array
  static char board[3][3] = { { ' ', ' ', ' ' }, { ' ', ' ', ' ' }, { ' ', ' ', ' ' } };

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

  static int turnDiff = 0;
  bool boardFull = (turn - turnDiff) == 10;

  if (boardFull && !isVictory(board)) isDraw = true;

  Serial.println(turnDiff);
  Serial.println(boardFull);
  Serial.println(isVictory(board));
  Serial.println(isDraw);

  if (isVictory(board) || isDraw) {

    turnDiff = turn - 1;
    if (isDraw) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Ops, it's a draw");

    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      if (!playerTurn) {  //i.e. if last turn was player's
        playerScore++;
        lcd.print("VICTORY IS YOURS");
      } else {
        aiScore++;
        lcd.print("Oh no! You lost");
      }
    }

    //print scores
    lcd.setCursor(0, 1);
    lcd.print(" You:");
    lcd.print(playerScore);
    lcd.print("  -  AI:");
    lcd.print(aiScore);
    delay(5000);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Starting another");
    lcd.setCursor(0, 1);
    lcd.print("game in: ");

    lcd.print(3);
    delay(1000);

    lcd.setCursor(9, 1);
    lcd.print(2);
    delay(1000);

    lcd.setCursor(9, 1);
    lcd.print(1);
    delay(1000);

    //reset turn counter
    Serial.print("turn ");
    Serial.println(turn);

    lcd.clear();
    if (!playerTurn) {
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

    //reset board
    for (int i = 0; i < sizeof(board); i++) {
      for (int j = 0; j < sizeof(board); j++) {
        board[i][j] = ' ';
      }
    }

    //print new empty board
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

  //check if player turn
  if (playerTurn) {
    //variables
    assigned = false;

    //msg to player 16x2 lcd
    lcd.clear();
    lcd.print(" Make your play");
    lcd.setCursor(0, 1);
    lcd.print(" (RED = cursor)");

    //print cursor 128x128 lcd
    tft.setTextColor(ST7735_RED);
    tft.setCursor(24 + (posx * 42), 12 + (posy * 24));
    tft.print(symb1);

    // check for valid play
    while (!assigned) {

      static bool isRed = false;
      if (!isRed) {
        tft.setTextColor(ST7735_RED);
        isRed = true;
      } else {
        tft.setTextColor(ST7735_BLACK);
        isRed = false;
      }
      delay(250);

      tft.setCursor(24 + (posx * 42), 12 + (posy * 24));
      tft.print(symb1);

      if (irrecv.decode())  // have we received an IR signal?
      {
        translateIR();
        irrecv.resume();         // receive the next value
        if (posx > 2) posx = 0;  //if player goes out of
        if (posx < 0) posx = 2;  //bounds, the cursor loops
        if (posy > 2) posy = 0;  //back to the beginning of
        if (posy < 0) posy = 2;  //line or column

        //print board on 128x128 lcd so that when player moves cursor,
        //the "background" stays the same, but the previous cursor is deleted
        //and a new one in a new position is printed
        tft.fillScreen(ST7735_BLACK);
        tft.setTextColor(ST7735_WHITE);

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

        //print cursor over previous board
        tft.setTextColor(ST7735_RED);
        tft.setCursor(24 + (posx * 42), 12 + (posy * 24));
        tft.print(symb1);
        delay(500);
      }

      if (assigned == true && board[posy][posx] != ' ') {  //when player presses play/pause button, assigned = true
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

  // if AI turn
  else {
    //variables
    bool found = false;

    //message to player (ai personality)
    if (turn % 2 == 0) {
      lcd.clear();
      lcd.print("Thinking...");
      delay(1500 / turn);
    } else if (turn % 3 == 0) {
      lcd.clear();
      lcd.print("Not bad...");
      delay(1500);
    } else if (turn % 5 == 0) {
      lcd.clear();
      lcd.print("I won't lose");
      delay(2000);
    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Humm...");
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

      // prevent player win horizontal
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
      } else if (board[2][1] == board[2][2] && board[2][1] == symb2 && board[2][0] == ' ') {
        board[2][0] = symb1;
        break;
      }

      // prevent player win vertical
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
      // downward
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

      // upward
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
      //check corners to win next turn
      else if (board[0][1] == symb1) {
        if (board[1][0] == symb1 && board[0][0] == ' ' && board[0][2] == ' ' && board[2][0] == ' ') {
          board[0][0] = symb1;
          break;
        }

        if (board[1][2] == symb1 && board[0][2] == ' ' && board[0][0] == ' ' && board[2][2] == ' ') {
          board[0][2] = symb1;
          break;
        }
      }

      else if (board[2][1] == symb1) {
        if (board[1][0] == symb1 && board[2][0] == ' ' && board[0][0] == ' ' && board[2][2] == ' ') {
          board[2][0] = symb1;
          break;
        }

        if (board[1][2] == symb1 && board[2][2] == ' ' && board[0][2] == ' ' && board[2][0] == ' ') {
          board[2][2] = symb1;
          break;
        }
      }

      // if middle empty, check for wins on next turn
      else if (board[1][1] == ' ') {
        if ((board[0][0] == symb1 && board[0][1] == symb1 && board[2][2] == ' ' && board[2][1] == ' ')
            || (board[0][1] == symb1 && board[0][2] == symb1 && board[2][0] == ' ' && board[2][1] == ' ')
            || (board[2][0] == symb1 && board[2][1] == symb1 && board[0][2] == ' ' && board[0][1] == ' ')
            || (board[2][1] == symb1 && board[2][2] == symb1 && board[0][0] == ' ' && board[0][1] == ' ')
            || (board[0][0] == symb1 && board[1][0] == symb1 && board[2][2] == ' ' && board[1][2] == ' ')
            || (board[1][0] == symb1 && board[2][0] == symb1 && board[0][2] == ' ' && board[1][2] == ' ')
            || (board[0][2] == symb1 && board[1][2] == symb1 && board[2][0] == ' ' && board[1][2] == ' ')
            || (board[1][2] == symb1 && board[2][2] == symb1 && board[0][0] == ' ' && board[1][2] == ' ')) {
          board[1][1] = symb1;
          break;
        }
      }

      // if symb1 is in the middle, check for wins on next turn
      else if (board[1][1] == symb1) {
        // check corners first
        // top left
        if (board[0][0] == symb1 && board[0][1] == ' ' && board[0][2] == ' ' && board[2][1] == ' ') {
          board[0][1] = symb1;
          break;
        }
        if (board[0][0] == symb1 && board[1][0] == ' ' && board[2][0] == ' ' && board[1][2] == ' ') {
          board[1][0] = symb1;
          break;
        }
        // top right
        if (board[0][2] == symb1 && board[0][1] == ' ' && board[0][0] == ' ' && board[2][1] == ' ') {
          board[0][1] = symb1;
          break;
        }
        if (board[0][2] == symb1 && board[1][2] == ' ' && board[2][2] == ' ' && board[1][0] == ' ') {
          board[1][2] = symb1;
          break;
        }
        // bottom left
        if (board[2][0] == symb1 && board[1][0] == ' ' && board[0][0] == ' ' && board[1][2] == ' ') {
          board[1][0] = symb1;
          break;
        }
        if (board[2][0] == symb1 && board[2][1] == ' ' && board[0][1] == ' ' && board[2][2] == ' ') {
          board[2][1] = symb1;
          break;
        }
        // bottom right
        if (board[2][2] == symb1 && board[1][2] == ' ' && board[0][2] == ' ' && board[1][0] == ' ') {
          board[1][2] = symb1;
          break;
        }
        if (board[2][2] == symb1 && board[2][1] == ' ' && board[2][0] == ' ' && board[0][1] == ' ') {
          board[2][1] = symb1;
          break;
        }
      }

      //select random positions
      randomSeed(analogRead(A0));
      int px = random(0, 3);
      randomSeed(analogRead(A1));
      int py = random(0, 3);
      int attempt = 0;


      // // rounds 2 and 3 try to line up two symbols
      for (int i = 0; i < 3 && !found; i++) {
        attempt = 0;
        for (int j = 0; j < 3 && !found; j++) {
          if (board[i][j] != symb1) {
            continue;
          }
          Serial.print("i");
          Serial.println(i);
          Serial.print("j");
          Serial.println(j);
          Serial.print("px");
          Serial.println(px);
          Serial.print("py");
          Serial.println(py);
          while (!found) {
            if (board[px][py] == ' ' &&
                // same line play adjacent
                ((abs(i - px) == 0 && abs(j - py) == 1 && ((board[i][0] == ' ' && board[i][1] == ' ') || (board[i][0] == ' ' && board[i][2] == ' ') || (board[i][1] == ' ' && board[i][2] == ' ')))
                 // same row play adjacent
                 || (abs(i - px) == 1 && abs(j - py) == 0 && ((board[0][j] == ' ' && board[1][j] == ' ') || (board[0][j] == ' ' && board[2][j] == ' ') || (board[1][j] == ' ' && board[2][j] == ' ')))
                 //form diagonal adjacent
                 || (abs(i - px) == 1 && abs(j - py) == 1 && px == 1 && py == 1)
                 //play across board same line
                 || (abs(i - px) == 0 && abs(j - py) == 2 && board[i][1] == ' ')
                 //play across board same row
                 || (abs(i - px) == 2 && abs(j - py) == 0 && board[1][j] == ' ')
                 //play across board diagonal
                 || (abs(i - px) == 2 && abs(j - py) == 2 && board[1][1] == ' '))) {
              board[px][py] = symb1;
              found = true;

            } else {
              randomSeed(analogRead(A0));
              px = random(0, 3);
              randomSeed(analogRead(A1));
              py = random(0, 3);
              attempt++;
              if (attempt == 10) {
                break;
              }
            }
          }
        }
      }
      Serial.print("attempt ");

      Serial.println(attempt);

      // for round 1 and other random plays, like round 2 if the AI plays second
      while (!found) {
        if (board[px][py] == ' ') {
          board[px][py] = symb1;
          found = true;
        } else {
          randomSeed(analogRead(A0));
          px = random(0, 3);
          randomSeed(analogRead(A1));
          py = random(0, 3);
        }
      }
    }
  }
  turn++;
}

//Functions
//////////////////

bool isVictory(char board[3][3]) {
  if (
    // horizontal
    (board[0][0] != ' ' && board[0][0] == board[0][1] && board[0][1] == board[0][2]) ||  //
    (board[1][0] != ' ' && board[1][0] == board[1][1] && board[1][1] == board[1][2]) ||  //
    (board[2][1] != ' ' && board[2][0] == board[2][1] && board[2][1] == board[2][2]) ||  //
    //vertical
    (board[0][0] != ' ' && board[0][0] == board[1][0] && board[1][0] == board[2][0]) ||  //
    (board[0][1] != ' ' && board[0][1] == board[1][1] && board[1][1] == board[2][1]) ||  //
    (board[0][2] != ' ' && board[0][2] == board[1][2] && board[1][2] == board[2][2]) ||  //
    //diagonal
    (board[0][0] != ' ' && board[0][0] == board[1][1] && board[1][1] == board[2][2]) ||  //downward
    (board[2][0] != ' ' && board[2][0] == board[1][1] && board[1][1] == board[0][2])) {  //upwards
    return true;
  }

  return false;
}

// bool boardFull(char board[3][3]) {
//   int count = 0;
//   for (int i = 0; i < sizeof(board); i++) {
//     for (int j = 0; j < sizeof(board); j++) {
//       if (board[i][j] != ' ') count++;
//     }
//   }
//   if (count == 9) return true;

//   return false;
// }
