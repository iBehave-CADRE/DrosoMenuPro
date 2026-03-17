#include <ZaberConnection.h>
#include <ZaberShield.h>

const byte interruptPin = 3;
int InputState = 0;
volatile bool InputChange = false;

using namespace Zaber;

/* Create a shield object to control the Zaber Arduino Shield */
Shield shield(ZABERSHIELD_ADDRESS_AA);

/* Create a connection object to control a connection using the shield */
Connection connection(shield);

void setup() {
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), DetectInputChange, CHANGE);
  /* Initialize Zaber ASCII shield */
  shield.begin();
  /* Issue a home command to device 1 */
  connection.genericCommand("home", 1);
  connection.waitUntilIdle(1);
}

void loop() {
  //noInterrupts();
  MoveStage();
  delay(1);
  //interrupts();
}

void DetectInputChange() {
    InputChange = true;
    //Serial.println("Change Detected");
}

void MoveStage() {
    if (InputChange==true) { // If there was a change of state on the input pin
    InputState = digitalRead(3); // Detect to which state it hast changed...high or low...high means in low means out
    if (InputState == HIGH) {
        connection.genericCommand("move abs 500000", 1);
        connection.waitUntilIdle(1);
        delay(1000);
    } else {
        connection.genericCommand("move abs 100000", 1);
        connection.waitUntilIdle(1);
        delay(1000);
    }
    InputChange == false; //Reset Change Detection to no change detected
  }
}