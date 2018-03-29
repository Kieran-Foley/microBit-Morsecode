/*********************************************************************************************************************************************
*
* micro.cpp:
*
* First written on 25/02/2018 by Kieran Foley
* Last modified on 09/03/2018 by Kieran Foley
*
* Author: Kieran Foley
*
* Program Name: micro.cpp
*
* Program Description:
* This program creates a protocol between two Microbit's connected by a wire on pin 1 to send voltages to each other. The voltages represent
* Morse Code, it does this by using timed voltages (> 300ms for a dot, >300 and < 900 for a dash). The combinations of dots and dashes then get          * converted into characters represented in the morseValues map. The microbits are divided into 'sender' and 'reciever' to depict which one will
* send the code and which one will show the message created.
*
*********************************************************************************************************************************************/

#include "MicroBit.h"
#include <string>
#include <vector>
#include <map>

using namespace std;

// Sender microBit
bool sender = false;
// Reciever microBit
bool receiver = false;
// Bool to show if the program is already running or not.
bool running = false;
// If the button has been pressed or not (state).
bool pressed = false;
// overall program elapsed time.
uint64_t currentTime = system_timer_current_time();
// How long button A has been pressed for
uint64_t difference = 0;
// Holds the message created by the user
vector<char> message;

// Initiate the microBit
MicroBit uBit;
MicroBitPin P1(MICROBIT_ID_IO_P1, MICROBIT_PIN_P1, PIN_CAPABILITY_ALL);

// Morse code values and their corresponding characters.
map<string, char> morseValues {
  {".-", 'A'},
  {"-...", 'B'},
  {"-.-.", 'C'},
  {"-..", 'D'},
  {".", 'E'},
  {"..-.", 'F'},
  {"--..", 'G'},
  {"....", 'H'},
  {"..", 'I'},
  {".---", 'J'},
  {"-.-", 'K'},
  {".-..", 'L'},
  {"--", 'M'},
  {"-.", 'N'},
  {"---", 'O'},
  {".--.", 'P'},
  {"--.-", 'Q'},
  {".-.", 'R'},
  {"...", 'S'},
  {"-", 'T'},
  {"..-", 'U'},
  {"...-", 'V'},
  {".--", 'W'},
  {"-..-", 'X'},
  {"-.--", 'Y'},
  {"--..", 'Z'},

  {".----", '1'},
  {"..---", '2'},
  {"...--", '3'},
  {"....-", '4'},
  {".....", '5'},
  {"-....", '6'},
  {"--...", '7'},
  {"---..", '8'},
  {"----.", '9'},
  {"-----", '0'},
};

// Evaluate how long button A has been pressed for and sends voltages to the reciever.
void processSender() {
  uBit.io.P1.setDigitalValue(0);
      while (1){

          currentTime = system_timer_current_time();

          // Send voltage whilst the button is being pressed
          while (uBit.buttonA.isPressed()){
            uBit.io.P1.setDigitalValue(1);
            pressed = true;
          }

          uBit.io.P1.setDigitalValue(0);
          // How the the button has been pressed for
          difference = system_timer_current_time() - currentTime;

          // If button has been pressed, display dot or dash depending on timing
          if (pressed){
            if (difference > 300 && difference < 900){
              uBit.display.print('-');
              uBit.sleep(300);
            } else if (difference < 300){
              uBit.display.print('.');
              uBit.sleep(300);
            }
          }
        // Revert state
        pressed = false;
        // Clears the display
        uBit.display.clear();
        // Reset the time the button has been pressed for
        difference = 0;
      }

}

// Prints out the message to the reciever microBit.
void printMessage(vector<char> message) {
  string buffer;
  vector<char> finalMessage;

  for (int i = 0; i < message.size(); ++i){
    if (message[i] == '_'){
      finalMessage.push_back(morseValues[buffer]);
      buffer = "";
    } else if (message[i] == ' '){
      finalMessage.push_back(' ');
      buffer = "";
    } else {
      buffer += message[i];
    }
  }

  for (int i = 0; i < finalMessage.size(); ++i){
    uBit.display.scroll(finalMessage[i]);
  }
  message.clear();
  finalMessage.clear();
}

// Evaluates if their is a voltage being sent or not, if yes, add a space/dot/dash depending on timing.
void processReceiver() {
  uBit.io.P1.setDigitalValue(0);
  while (1){
          currentTime = system_timer_current_time();

          //When no pin values are set
          if (!pressed){
            while (P1.getDigitalValue(PullUp) == 0){
              pressed = false;
            }

            difference = system_timer_current_time() - currentTime;
            // Print out the message.
            if (difference > 7000) {
              message.push_back('_');
              printMessage(message);
            // Add's a space to the message.
            } else if (difference > 3000){
              message.push_back('_');
              message.push_back(' ');
            // New Letter
            } else if (difference > 900 && difference < 3000) {
              message.push_back('_');
            }
          }

          //When pin values are being sent.
          currentTime = system_timer_current_time();

          while (P1.getDigitalValue(PullUp) == 1){
              pressed = true;
          }

          difference = system_timer_current_time() - currentTime;

          if (pressed){
            // Adds a dash to the message.
            if (difference > 300 && difference < 900){
              uBit.display.print('-');
              message.push_back('-');
              uBit.sleep(300);
            } else if (difference < 300){
              // Adds a dot to the message.
              uBit.display.print('.');
              message.push_back('.');
              uBit.sleep(300);
            }
            pressed = false;
            uBit.display.clear();
            difference = 0;
          }
        }

}

// Assigns each microBit to be either a sender or reciever.
int main() {
  uBit.init();

  // If the program is being started up depict microBits to sender / reciever
  while (!running) {
    if (uBit.buttonA.isPressed() || uBit.buttonB.isPressed()) {
      // If button A has been pressed, assign it to be sender.
      sender = uBit.buttonA.isPressed() ? true : false;
      // If button B has been pressed, assign it to be Reciever.
      receiver = uBit.buttonB.isPressed() ? true : false;
      // Prints out S or R on the sender / reciever microBit.
      uBit.display.print(sender ? "S" : "R");
      uBit.sleep(1500);
      running = true;
    }
  }
    
  while (running) {
    if (receiver)
      processReceiver();
    if (sender)
      processSender();
  }
  // End program
  release_fiber();
}