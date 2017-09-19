//original ppm code by abhilash_patel

/* values for ppm code */
#define ChannelsUsed 4      //define the number of radio channels needed
#define ReceiverPin 3
unsigned long int a, b, c;    
int x[15], ch1[15], i; 
int avg[ChannelsUsed];

/* values for updating the motor speeds */
#define motorUpdateTime 20
unsigned long lastTime = 0;
int MixedSpeeds[2] = {0, 0};
int motorValues[2] = {1, 1};
int speeds[2];

/* values for the weapon */
#define weaponESC 9
bool notWaiting = true;
unsigned long escFrameStart = micros();
unsigned long escPulseStop = micros();
int escPulseWidth;

/* defines pins for the motor outputs */
#define Motor1Dir0 10
#define Motor1Dir1 11
#define Motor2Dir0 6
#define Motor2Dir1 5

#define Motor1Enable 12
#define Motor2Enable 8

/* defines the size of the deadband to reduce jitter and the effects of noise */
#define motorDeadband 0.1 //percent error between commanded and set motor pwm value
const int range255Deadband = motorDeadband * 255;

/* Status LED pins */
#define redLED A0
#define blueLED A5
bool alreadyRunning = false;

/* code to setup the pins and initial states */
void setup() {
  pinMode(Motor1Dir0, OUTPUT);    //set all the motor PWM output pins as outputs
  pinMode(Motor1Dir1, OUTPUT);
  pinMode(Motor2Dir0, OUTPUT);
  pinMode(Motor2Dir1, OUTPUT);

  pinMode(Motor1Enable, OUTPUT);  //sets the motor enable pins as outputs to allow full on/off control of the motors
  pinMode(Motor2Enable, OUTPUT);
  
  
  pinMode(ReceiverPin, INPUT_PULLUP);   //set the input pin for the radio receiver to use the input pullup resistor
  attachInterrupt(digitalPinToInterrupt(ReceiverPin), read_me, FALLING);    //set an interrupt for the ppm receiver to avoid timing issues


  pinMode(redLED, OUTPUT);    //set the status LED pins as outputs
  pinMode(blueLED, OUTPUT);
  
  digitalWrite(blueLED, LOW);   //initially turn the blue LED off and the red LED on to indicate no reciever connected/ no data being received
  digitalWrite(redLED, HIGH);
}


/* loop to run continuously to keep the robot running and responding to inputs, as well as show status */
void loop() {
  read_rc();  //read the constantly updated ppm values out of ch1[] and shift them into avg[]
  
  if (avg[3] > 850) {   //if an alive signal is seen coming from the transmitter, run the 'armed' code
    if (!alreadyRunning) {    //if this alive signal was not found last time, run this code
      digitalWrite(blueLED, HIGH);  //turn the blue light on to indicate a Tx was found
      digitalWrite(redLED, LOW);    // turn off the red status light for the same reason
      enableDrive();  //pull the enable pins of the L293D hbridge high to enable the motor outputs
      alreadyRunning = true;  //set the condition to true so that this loop does not execute every time through loop()
    }
    
    updateWeaponESC();    //call this function each time through the loop to monitor and create the correct square waveform to send the correct throttle value to the weapon's ESC to control the weapon's speed

    if (motorUpdateTime < millis() - lastTime) {    //if the motors are due to be updated (rather than every time through the loop to save time and reduce jitter)
      if ( (abs( ((motorValues[0] - avg[2]) / float(motorValues[0]))) > motorDeadband) || (abs(((motorValues[1] - avg[0]) / float(motorValues[1]))) > motorDeadband) ) {    // check if the difference between the recieved positions are more than motorDeadband as a percent away from where the motors are currently set to (for each channel)
        updateMotors(avg[2], avg[0]);   //send the recieved control values for fwd/bkw and lft/rht to the function to set the correct output speed/direction for each
      }
      lastTime = millis();    //if the motors have been updated, set the new lasttime to now
    }
  } else {  //if the alive signal is not found (ie a Tx is not connected)
    if (alreadyRunning) { //check to see if the alive signal was previously found, and if so run this loop otherwise skip the code (to avoid running code that changes nothing
      digitalWrite(blueLED, LOW);   //set the blue LED to off
      digitalWrite(redLED, HIGH); // set the red LED to on to represent a Tx not being found
      alreadyRunning = false; //set the condition to false so this loop does not run again until a tx was found, and so when it is found it runs the loop to enable drive and turn the blue LED on
      disableDrive();     //set the enable pins of the hbridge to low so the motors do not turn due to noise or other issues
    }
  }
}

/* monitor and update the square waveform signal used to control the weapon's ESC to set the motor speed */
void updateWeaponESC() {    
  if (micros() >= escFrameStart + 20000) {    //check to see if the 20ms framelength is met and if so, calculate a new waveform
    escPulseWidth = constrain(map(avg[1], 0, 1000, 1000, 2000), 1000, 2000);    //take the input throttle value and map it to the 1ms (1000microseconds) to 2ms corresponding to the value, and then ensure it is within the 1000 to 2000 range to avoid singal issues
    digitalWrite(weaponESC, HIGH);    //write the signal pin high to begin the waveform
    escFrameStart = micros();     //save the start of the waveform as the time in microseconds since the start of the program
    escPulseStop = escFrameStart + escPulseWidth;   //set the time to turn off the signal pin to the start of the frame plus however long the high pulse should be for the given throttle value
    notWaiting = true;      //set the condition to true so that the second else if statment will run once it is time to turn off the signal pin
  } else if ((micros() >= escPulseStop) && notWaiting) {  //otherwise, if the output is still within the 20ms frame, but has passed the calculated time to pull the signal pin low (and the last condition to be met was the start of a new frame)
    digitalWrite(weaponESC, LOW);   //write the signal pin low
    notWaiting = false;   //set the condition to false so that this loop does not run and overwrite the pin low every time through the loop for no reason
  }
}

/* enable both drive motors */
void enableDrive() {
  digitalWrite(Motor1Enable, HIGH);  // write the enable pin for the first motor high to enable the output
  digitalWrite(Motor2Enable, HIGH); //write the enable pin for the second motor high
}

/* disable both drive motors */
void disableDrive() {
  digitalWrite(Motor1Enable, LOW);    //pull the enable pin for motor 1 low to disable the output
  digitalWrite(Motor2Enable, LOW);    //write the motor 2 enable pin low 
}

/* main function used to map inputs and set motor speeds using PWM output, as well as enable/disable motors if a value is within the motor deadband or not */
void updateMotors(int Ch1, int Ch2) {
  motorValues[0] = Ch1;       //store the fwd/bkw values used to update the motors to compare against in the percent error if statement in loop()
  motorValues[1] = Ch2;       //stores the lft/rht values
  int linearValue = map(Ch1, 0, 1000, -255, 255);   //maps the input 0-1000 value accross -255 and 255 for fwd/bkw
  int rotationalValue = map(Ch2, 0, 1000, -255, 255);   //maps the input 0 to 1000 to -255 and 255 for lft/rht

  linearValue = (abs(linearValue) < range255Deadband) ? (0) : (linearValue);  //set the linear value to 0 if it is within the motordeadband when applied to a 255 range
  rotationalValue = (abs(rotationalValue) < range255Deadband) ? (0) : (rotationalValue);  //do the same with the rotational value to avoid noise issuse and jittering on the output

  
  motorMixer(linearValue, rotationalValue);   //call the main function used to mix the linear and rotational values across two seperate motors


  if (abs(speeds[0]) < range255Deadband) {  // if the calculated motor speed is within the deadband
    digitalWrite(Motor1Enable, LOW);    //write the motor 1 enable pin low to disable the motor
  } else {    //otherwise if it is outside the deadband
    digitalWrite(Motor1Enable, HIGH);   //write the motor 1 enable pin high to enable the output
  }

  /* The same as the above code to enable/disable the output if the calculated speed is outside/inside the deadband */
  
  if (abs(speeds[1]) < range255Deadband) {
    digitalWrite(Motor2Enable, LOW);
  } else {
    digitalWrite(Motor2Enable, HIGH);
  }
  

  if (MixedSpeeds[0] > 0) {       //if the speed is positive, use the positive direction 
    digitalWrite(Motor1Dir1, LOW);    //pull the negative direction input pin to ground
    analogWrite(Motor1Dir0, MixedSpeeds[0]);   //write the motor 1 speed to the positive direction input pin
  } else {    //otherwise if the speed is negative
    digitalWrite(Motor1Dir0, LOW);    //pull the positive direction pin to ground
    analogWrite(Motor1Dir1, -MixedSpeeds[0]);   //write the positive value of the speed to the negative direction input pin (cannot use a negative value in analogWrite, so because MixedSpeeds[0] must be negative, simply multiply by -1 to get the positive value)
  }

  /* Doing the exact same thing as above, but applying the values to the second motor */

  if (MixedSpeeds[1] > 0) {
    digitalWrite(Motor2Dir1, LOW);
    analogWrite(Motor2Dir0, abs(MixedSpeeds[1]));
  } else {
    digitalWrite(Motor2Dir0, LOW);
    analogWrite(Motor2Dir1, abs(MixedSpeeds[1]));
  }
}

/* determine the correct functions to use for motor mixing depending on the inputs given and return the constrained motor speeds so they can be written */
void motorMixer(int fwd, int trn) {
  if ( ( (fwd  < (255 * motorDeadband)) && (fwd > (-255 * motorDeadband))) || ((trn  < (255 * motorDeadband)) && (trn > (-255 * motorDeadband)))) {  //if either axis is within the deadband of zero
    speeds[0] = constrain((fwd + trn), -255, 255);    //use this simple function to determine output speed of motor 1
    speeds[1] = constrain((fwd - trn), -255, 255);    //motor 2's equivalent
  } else if (trn > 0) {   //otherwise if the lft/rht value is positive (right)
    if (fwd > 0) {  //and if the fwd/bkw is positive (fwd, right)
      speeds[0] = ((fwd > trn) ? (fwd) : (trn)); //max(abs(fwd), abs(trn));   //function for motor 1's speed in this case
      speeds[1] = 255 - (trn / 2) - ((255 - fwd) / 2);    //function for motor 2's speed in this quadrant
    } else {    //otherwise, if fwd/bkw is negative (backward, right)
      speeds[0] = -1 * ((-fwd > trn) ? (-fwd) : (trn));
      speeds[1] = -255 + (trn / 2) + ((255 + fwd) / 2);
    }
  } else {  //otherwise the trn value is negative (lft)
    if (fwd > 0) {  //if the fwd/bkw is positive (fwd, left)
      speeds[0] = 255 - (-trn / 2) - ((255 - fwd) / 2);
      speeds[1] = ((fwd > -trn) ? (fwd) : (-trn));
    } else {    //if the fwd/bkw is negative (backward, left)
      speeds[0] = -255 + (-trn / 2) + ((255 + fwd) / 2);
      speeds[1] = -1 * ((-fwd > -trn) ? (-fwd) : (-trn));
    }
  }
  
  MixedSpeeds[0] = constrain(speeds[0],-255,255);   //constrains the first motor's output speed of the function to -255 to 255 to avoid issues with values larger than expected
  MixedSpeeds[1] = constrain(speeds[1],-255,255);   //same for the second motor's speed
}


//BEGIN MODIFIED PPM CODE BY abhilash_patel

void read_me()  {
  a = micros();
  c = a - b;
  b = a; 
  x[i] = c;
  i = i + 1;
  if (i == 15) {
    for (int j = 0; j < 15; j++) {
      ch1[j] = x[j];
    }
    i = 0;
  }
}


void read_rc() {
  int i, j, k = 0;
  for (k = 14; k > -1; k--) {
    if (ch1[k] > 10000) {
      j = k;
    }
  }
  for (i = 0; i < ChannelsUsed; i++) {
    avg[i] = (ch1[i + j+1] - 1000);
  }
}
