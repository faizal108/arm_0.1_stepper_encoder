#include <AccelStepper.h>

#define dirPin 6
#define stepPin 7

#define CLK 2
#define DT 3
#define BTN 4

// this lib just used for setup motor
AccelStepper stepper = AccelStepper(1, stepPin, dirPin);

int BTNState = 0;
int increment = 100;
int index = 0;
int moveDist = 0;
int prevDist = 0;
int counter = 0;
int currentStateCLK;
int lastStateCLK;
String currentDir = "";

//Recording button
const int recordBtnPin = 10;
const int recordLedPin =  12;
int recordBtnState = 0;

//Play button
const int playBtnPin = 11;
const int playLedPin =  13;
int playBtnState = 0;

//memory
int memory[200];

void setup() {
  stepper.setMaxSpeed(4000);
  stepper.setAcceleration(2500);
  stepper.setSpeed(3500);

  // Set encoder pins as inputs
  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);
  pinMode(BTN, INPUT);

  // Setup Serial Monitor
  Serial.begin(115200);

  // Read the initial state of CLK
  lastStateCLK = digitalRead(CLK);

  // Call updateEncoder() when any high/low changed seen
  // on interrupt 0 (pin 2), or interrupt 1 (pin 3)
  attachInterrupt(0, freeMove, CHANGE);
  attachInterrupt(1, freeMove, CHANGE);

  //  Record button & Led configuration
  pinMode(recordBtnPin, INPUT);
  digitalWrite(recordBtnPin, LOW);
  pinMode(recordLedPin, OUTPUT);
  digitalWrite(recordLedPin, LOW);

  //  Play button & Led configuration
  pinMode(playBtnPin, INPUT);
  digitalWrite(playBtnPin, LOW);
  pinMode(playLedPin, OUTPUT);
  digitalWrite(playLedPin, LOW);
}

void startPlaying() {
  Serial.println("Playing your saved recording.......");
  for (int i = 0 ; i < index; i++) {
    Serial.print("\t");
    Serial.println(memory[i]);
    stepper.moveTo(memory[i]);
    while (stepper.distanceToGo() > 0 || stepper.distanceToGo() < 0) {
      stepper.runToPosition();
    }
  }
}

void startRecording() {
  Serial.println("Recording your moves.......");
  moveDist = counter;
  if (prevDist != moveDist) {
    stepper.moveTo(moveDist);
    while (stepper.distanceToGo() > 0 || stepper.distanceToGo() < 0) {
      stepper.runToPosition();
    }
    memory[index] = moveDist;
    index++;
  }
  prevDist = moveDist;
}

void freeMove() {
  if (playBtnState != HIGH) {
    // Read the current state of CLK
    currentStateCLK = digitalRead(CLK);
    // If last and current state of CLK are different, then pulse occurred
    // React to only 1 state change to avoid double count
    if (currentStateCLK != lastStateCLK  && currentStateCLK == 1) {

      // If the DT state is different than the CLK state then
      // the encoder is rotating CCW so decrement
      if (digitalRead(DT) != currentStateCLK) {
        counter = counter - increment;
        currentDir = "CCW";
      } else {
        // Encoder is rotating CW so increment
        counter = counter + increment;
        currentDir = "CW";
      }

      Serial.print("Direction: ");
      Serial.print(currentDir);
      Serial.print(" | Counter: ");
      Serial.println(counter);
    }

    // Remember last CLK state
    lastStateCLK = currentStateCLK;
  }
}


void loop() {
  stepper.runSpeed();

  recordBtnState = digitalRead(recordBtnPin);
  playBtnState = digitalRead(playBtnPin);
  BTNState = digitalRead(BTN);

  moveDist = counter;
  stepper.moveTo(moveDist);
  while (stepper.distanceToGo() > 0 || stepper.distanceToGo() < 0) {
    stepper.runToPosition();
  }

  //for recording
  if (recordBtnState == HIGH) {
    digitalWrite(recordLedPin, HIGH);
    startRecording();
  } else {
    digitalWrite(recordLedPin, LOW);
  }

  //for playing a recording
  if (playBtnState == HIGH) {
    digitalWrite(playLedPin, HIGH);
    startPlaying();
  } else {
    digitalWrite(playLedPin, LOW);
  }

  //  encoder button
  if (BTNState == LOW) {
    stepper.moveTo(0);
    while (stepper.distanceToGo() > 0 || stepper.distanceToGo() < 0) {
      stepper.runToPosition();
    }
    counter = 0;
  }

  delay(100);
}
