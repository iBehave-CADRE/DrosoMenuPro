//Loading the Libraries that are necessary to work with the Zaber shield
#include <ZaberConnection.h>
#include <ZaberShield.h>

//Defining the digital I/O pins connected to the buttons on the Zaber shield. Button 1 is connected to pin 7 and Button 2 is connected to pin 8
#define Button1Pin 7
#define Button2Pin 8

//Depending on if there is still an analog port free we can also just use an analog output
#define BNCSynchOutPosition1Pin 9
#define BNCSynchOutPosition2Pin 10

//Defining variables for state memory and debouncing
bool Button1StateOld;
bool Button1StateNew;
bool Button2StateOld;
bool Button2StateNew;

//Defining a digital I/O pin for reading a BNC input. This input is used to move the stimulus to the fly (LOW) or away from the animal (HIGH) to the RestingPosition.
#define BNCInputInOutPin 5
//Defining variables for State Memory
bool BNCInputInOutStateOld;
bool BNCInputInOutStateNew;

//Defining a digital I/O pin for reading a BNC input. This input is used to select between the do different stimuli (Low; stimulus 1, HIGH; stimulus 2)
#define BNCInputPositionSelectPin 6
//Defining variables for State Memory
bool BNCInputPositionSelectStateOld;
bool BNCInputPositionSelectStateNew;

//MicrostepSize for X-LSM50B is 0.1905 µm
const float MicrostepSize = 0.047625; // in µm/step

//Defining the absolute values for resting and out positions
//those positions are setup specific and need to be adapted on a case to case basis
//long xOutPosition = 0;
//long zOutPosition = 5000;
//long xRestingPosition = 250000;
//long zStimulusHeightPosition = 50000;


//Positions of Stimuli relative to the Resting Position (Currently set to 8 mm distance)
//Those values are calibrated to the stimulus holder design dimenstions and ball radius
const float xStimulus1Position = 5.0; //in mm
const float xStimulus2Position = -5.0; //in mm
const float xCalibrationMarkerDistance = 15.0; //Distance of 15 mm
//Clearance distance is also setup specific
const float zClearanceDistance = 3.0;

//Knob Distance of 1 means 0.1905 µm per klick
const float KnobStepSize = 0.25; // in mm

//Defining the variables for storing the Device Address for serial communication
int xAxisAddress = 1;
int zAxisAddress = 2;

//String command = "";

using namespace Zaber;

/* Create a shield object to control the Zaber Arduino Shield */
Shield shield(ZABERSHIELD_ADDRESS_AA);

/* Create a connection object to control a connection using the shield */
Connection connection(shield);

int StepsInMM(long Steps, float uStepSize) {
  int MM = Steps * (uStepSize / 1000.0);
  Serial.println(String(MM));
  return MM;
}

long MMinSteps(float MM, float uStepSize) {
  long Steps = round(MM / (uStepSize / 1000));
  //Serial.println(String(Steps));
  return Steps;
}

void setup() {
  Serial.begin(9600);
  Serial.println("DrosoMenuPro Booting...");

  pinMode(Button1Pin, INPUT);
  pinMode(Button2Pin, INPUT);
  pinMode(BNCSynchOutPosition1Pin, OUTPUT);
  digitalWrite(BNCSynchOutPosition1Pin, LOW);
  pinMode(BNCSynchOutPosition2Pin, OUTPUT);
  digitalWrite(BNCSynchOutPosition1Pin, LOW);

  //Using an external pulldown, better behavior of DrosoMenuPro
  pinMode(BNCInputInOutPin, INPUT);
  pinMode(BNCInputPositionSelectPin, INPUT);

  /* Initialize Zaber ASCII shield */
  shield.begin();

  connection.genericCommand("renumber", xAxisAddress);

  //Turning off LEDs to avoid PMT damage
  Serial.println("Turning off the lamps for 2P imaging...");
  connection.genericCommand("set system.led.enable 0", xAxisAddress);
  connection.genericCommand("set system.led.enable 0", zAxisAddress);

  Serial.println("Setting Knob Mode...");
  connection.genericCommand("set knob.enable 1", xAxisAddress);
  connection.genericCommand("set knob.enable 1", zAxisAddress);
  //Knob mode 0 is Velocity Mode 1 is Displacement Mode 2 is Force Mode
  //Set Knob mode to Displacement Mode
  connection.genericCommand("set knob.mode 1", xAxisAddress);
  connection.genericCommand("set knob.mode 1", zAxisAddress);

  connection.genericCommand("set knob.distance " + String(MMinSteps(KnobStepSize, MicrostepSize)) + "", xAxisAddress);
  connection.genericCommand("set knob.distance " + String(MMinSteps(KnobStepSize, MicrostepSize)) + "", zAxisAddress);

  /* Issue a home command to device 1 and 2 */
  //Serial.println("Homing xzAxis...");
  //connection.genericCommand("home", xAxisAddress);
  //connection.waitUntilIdle(xAxisAddress);
  //connection.genericCommand("home", zAxisAddress);
  //connection.waitUntilIdle(zAxisAddress);

  //Serial.println("Moving to Defined Resting Position...");
  //connection.genericCommand("move abs " + String(xRestingPosition) + "", xAxisAddress);
  //connection.waitUntilIdle(xAxisAddress);
  //connection.genericCommand("move abs " + String(zStimulusHeightPosition) + "", zAxisAddress);
  //connection.waitUntilIdle(zAxisAddress);

  Serial.println("Ready for Experiments...");
}

void loop() {
  MoveStage();
  delay(1);
  Button1Action();
  delay(1);
  Button2Action();
  delay(1);
}

void MoveStage() {
  //Reading the status of the two digital input pins to later decide where to move to
  BNCInputInOutStateNew = digitalRead(BNCInputInOutPin);
  BNCInputPositionSelectStateNew = digitalRead(BNCInputPositionSelectPin);

  if (BNCInputInOutStateNew != BNCInputInOutStateOld) {
    connection.genericCommand("tools parking unpark", zAxisAddress);
    connection.genericCommand("tools parking unpark", xAxisAddress);
      if (BNCInputInOutStateNew==false) {
        Serial.println("Moving to Resting Position...");

        //Setting BNC Synch Signal to LOW for both positions
        digitalWrite(BNCSynchOutPosition1Pin, LOW);
        digitalWrite(BNCSynchOutPosition2Pin, LOW);

        connection.genericCommand("move rel -" + String(MMinSteps(zClearanceDistance, MicrostepSize))  +"", zAxisAddress);
        connection.waitUntilIdle(zAxisAddress);

        connection.genericCommand("move stored 1", xAxisAddress);
        connection.waitUntilIdle(xAxisAddress);

        connection.genericCommand("move rel " + String(MMinSteps(zClearanceDistance, MicrostepSize))  +"", zAxisAddress);
        connection.waitUntilIdle(zAxisAddress);

      } else if (BNCInputInOutStateNew==true) {
        if (BNCInputPositionSelectStateNew==false) {
        Serial.println("Moving to Position 1...");

        connection.genericCommand("move rel -" + String(MMinSteps(zClearanceDistance, MicrostepSize))  +"", zAxisAddress);
        connection.waitUntilIdle(zAxisAddress);

        connection.genericCommand("move rel " + String(MMinSteps(xStimulus1Position, MicrostepSize))  +"", xAxisAddress);
        connection.waitUntilIdle(xAxisAddress);

        connection.genericCommand("move rel " + String(MMinSteps(zClearanceDistance, MicrostepSize))  +"", zAxisAddress);
        connection.waitUntilIdle(zAxisAddress);

        //Setting BNC Synch Signal to HIGH for position 1
        digitalWrite(BNCSynchOutPosition1Pin, HIGH);
        digitalWrite(BNCSynchOutPosition2Pin, LOW);

        } else if (BNCInputPositionSelectStateNew==true) {
        Serial.println("Moving to Position 2...");

        connection.genericCommand("move rel -" + String(MMinSteps(zClearanceDistance, MicrostepSize))  +"", zAxisAddress);
        connection.waitUntilIdle(zAxisAddress);

        connection.genericCommand("move rel " + String(MMinSteps(xStimulus2Position, MicrostepSize))  +"", xAxisAddress);
        connection.waitUntilIdle(xAxisAddress);

        connection.genericCommand("move rel " + String(MMinSteps(zClearanceDistance, MicrostepSize))  +"", zAxisAddress);
        connection.waitUntilIdle(zAxisAddress);

        //Setting BNC Synch Signal to HIGH for position 2
        digitalWrite(BNCSynchOutPosition1Pin, LOW);
        digitalWrite(BNCSynchOutPosition2Pin, HIGH);
        }
      }
  BNCInputInOutStateOld = BNCInputInOutStateNew;
  BNCInputPositionSelectStateOld = BNCInputPositionSelectStateNew;
  connection.genericCommand("tools parking park", zAxisAddress);
  connection.genericCommand("tools parking park", xAxisAddress);
  }
}

//Function for Creating a Calibration Button
//Calibration Workflow: Move to Calibration marker.
//Push Button
//Then the stage moves to resting position relative to the calibration marker.
//DrosoMenuPro then stores the current position as new Resting Position
//This way, each fly can have their own resting position and simulus position
//Calibration Marker is 18 mm away from Stimulus Position 2
void Button1Action() {
  Button1StateNew = digitalRead(Button1Pin);
  if (Button1StateNew != Button1StateOld) {
      if (Button1StateNew==true) {
        //delay(1);
      } else if (Button1StateNew==false) {
          //Execution if Button is pushed
          connection.genericCommand("tools parking unpark", zAxisAddress);
          connection.genericCommand("tools parking unpark", xAxisAddress);


          Serial.println("Movement to Resting Position Relative to Calibration Marker...");

          connection.genericCommand("move rel -" + String(MMinSteps(zClearanceDistance, MicrostepSize))  +"", zAxisAddress);
          connection.waitUntilIdle(zAxisAddress);

          connection.genericCommand("move rel " + String(MMinSteps(xCalibrationMarkerDistance, MicrostepSize))  +"", xAxisAddress);
          connection.waitUntilIdle(xAxisAddress);

          connection.genericCommand("move rel " + String(MMinSteps(zClearanceDistance, MicrostepSize))  +"", zAxisAddress);
          connection.waitUntilIdle(zAxisAddress);
          //Sore z Axis Position
          Serial.println("Storing Current Position as New Resting Position to Controller Menory Slot 1...");
          connection.genericCommand("tools storepos 1 current", xAxisAddress);

          connection.waitUntilIdle(xAxisAddress);

          connection.genericCommand("tools parking park", zAxisAddress);
          connection.genericCommand("tools parking park", xAxisAddress);
      }
  Button1StateOld = Button1StateNew;
  }
}


////We use park and unpark instead of homing procedure to avoid crashing into other devices under the microscope
void Button2Action() {
  Button2StateNew = digitalRead(Button2Pin);
  if (Button2StateNew != Button2StateOld) {
      if (Button2StateNew==true) {
        //delay(1);
      } else if (Button2StateNew==false) {
        Serial.println("Unparking for calibration...");
        connection.genericCommand("tools parking unpark", zAxisAddress);
        connection.genericCommand("tools parking unpark", xAxisAddress);
      }
  Button2StateOld = Button2StateNew;
  }
}