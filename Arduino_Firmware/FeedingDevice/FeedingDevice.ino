//Loading the Libraries that are necessary to work with the Zaber shield
#include <ZaberConnection.h>
#include <ZaberShield.h>

//Defining the digital I/O pins connected to the buttons on the Zaber shield. Button 1 is connected to pin 7 and Button 2 is connected to pin 8
#define Button1Pin 7
#define Button2Pin 8

//Defining variables for state memory and debouncing
bool Button1StateOld;
bool Button1StateNew;
bool Button2StateOld;
bool Button2StateNew;

//Defining a digital I/O pin for reading a BNC input. This input is used to move the stimulus to the fly (LOW) or away from the animal (HIGH) to the RestingPosition.
#define BNCInputInOutPin 3
//Defining variables for State Memory
bool BNCInputInOutStateOld;
bool BNCInputInOutStateNew;

//Defining a digital I/O pin for reading a BNC input. This input is used to select between the do different stimuli (Low; stimulus 1, HIGH; stimulus 2)
#define BNCInputPositionSelectPin 4
//Defining variables for State Memory
bool BNCInputPositionSelectStateOld;
bool BNCInputPositionSelectStateNew;

//Defining the absolute values for relevant positions
long OutPosition = 0;
long RestingPosition = 250000;
//Positions of Stimuli relative to the Resting Position (Currently set to 8 mm distance)
long Stimulus1Position = -41995;
long Stimulus2Position = 41995;

long CalibrationMarkerDistance = 94488; //Distance of 18 mm in Steps

//Numper of Steps for a Distance = Distance in mm / Microstepsitze

int MicrostepSize = 0.1905; // in µm 

//MicrostepSize for X-LSM50B is 0.1905 µm
//Knob Distance of 1 means 0.1905 µm per klick
//Step Size is set to 0.5 mm

long KnobStepSize = 1313;

//Defining the variables for storing the Device Address for serial communication
int xAxisAddress = 1;
int zAxisAddress = 2;

String command = "";

using namespace Zaber;

/* Create a shield object to control the Zaber Arduino Shield */
Shield shield(ZABERSHIELD_ADDRESS_AA);

/* Create a connection object to control a connection using the shield */
Connection connection(shield);

void setup() {
  Serial.begin(9600);
  Serial.println("DrosoMenuPro Booting...");


  // store current position (presist after reboot and power cycle)
  //move to stored position

  pinMode(Button1Pin, INPUT);
  pinMode(Button2Pin, INPUT);

  //Using an external pulldown, better behavior of DrosoMenuPro
  pinMode(BNCInputInOutPin, INPUT);
  pinMode(BNCInputPositionSelectPin, INPUT);

  /* Initialize Zaber ASCII shield */
  shield.begin();

  Serial.println("Setting Knob Mode...");
  connection.genericCommand("set knob.enable 1", xAxisAddress);
  //Knob mode 0 is Velocity Mode 1 is Displacement Mode 2 is Force Mode
  //Set Knob mode to Displacement Mode
  connection.genericCommand("set knob.mode 1", xAxisAddress);

  connection.genericCommand("set knob.distance " + String(KnobStepSize) + "", xAxisAddress);

  /* Issue a home command to device 1 */
  Serial.println("Homing xzAxis...");
  connection.genericCommand("home", xAxisAddress);
  connection.waitUntilIdle(xAxisAddress);
  //Home z Axis


  Serial.println("Moving to Defined Resting Position...");
  connection.genericCommand("move abs " + String(RestingPosition) + "", xAxisAddress);
  connection.waitUntilIdle(xAxisAddress);

  Serial.println("Storing Current Position to Controller Menory Slot 1...");
  connection.genericCommand("tools storepos 1 current", xAxisAddress);
  connection.waitUntilIdle(xAxisAddress);



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
      if (BNCInputInOutStateNew==false) {
        Serial.println("Moving to Resting Position...");
        //connection.genericCommand("move abs " + String(RestingPosition) + "", xAxisAddress);
        connection.genericCommand("move stored 1", xAxisAddress);
        connection.waitUntilIdle(xAxisAddress);
        delay(1000);
      } else if (BNCInputInOutStateNew==true) {
        if (BNCInputPositionSelectStateNew==false) {
        Serial.println("Moving to Position 1...");
        connection.genericCommand("move rel " + String(Stimulus1Position)  +"", xAxisAddress);
        connection.waitUntilIdle(xAxisAddress);
        delay(1000);
        } else if (BNCInputPositionSelectStateNew==true) {
        Serial.println("Moving to Position 2...");
        connection.genericCommand("move rel " + String(Stimulus2Position)  +"", xAxisAddress);
        connection.waitUntilIdle(xAxisAddress);
        delay(1000);
        }
        
      }
  BNCInputInOutStateOld = BNCInputInOutStateNew;
  BNCInputPositionSelectStateOld = BNCInputPositionSelectStateNew;
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

          Serial.println("Movement to Resting Position Relative to Calibration Marker...");
          //Move Down
          //Pause
          connection.genericCommand("move rel " + String(CalibrationMarkerDistance)  +"", xAxisAddress);
          connection.waitUntilIdle(xAxisAddress);
          //Move Up
          //Pause

          Serial.println("Storing Current Position as New Resting Position to Controller Menory Slot 1...");
          connection.genericCommand("tools storepos 1 current", xAxisAddress);
          //Sore z Axis Position
          connection.waitUntilIdle(xAxisAddress);
      }
  Button1StateOld = Button1StateNew;
  }
}

void Button2Action() {
  Button2StateNew = digitalRead(Button2Pin);
  if (Button2StateNew != Button2StateOld) {
      if (Button2StateNew==true) {
        //delay(1);
      } else if (Button2StateNew==false) {
        Serial.println("Exiting DrosoMenuPro to Out Position");
        //Serial.println(String(MMinSteps(20)));
        //Serial.println(String(StepsInMM(20)));
        connection.genericCommand("move abs " + String(OutPosition)  +"", xAxisAddress);
        connection.waitUntilIdle(xAxisAddress);
      }
  Button2StateOld = Button2StateNew;
  }
}

int StepsInMM (long Steps) {
  int MM = Steps * MicrostepSize;
  return MM;
}

long MMinSteps (int MM) {
  long Steps = round(MM / MicrostepSize);
  return Steps;
}



//ToDo:
//- Add second axis movement with colision avoidance behavior
//- Add a function to calculate mm in steps and vise versa


