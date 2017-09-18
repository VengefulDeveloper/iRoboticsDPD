//original ppm code by abhilash_patel

#define ChannelsUsed 4
#define iterations 3
unsigned long int a, b, c;
int x[15], ch1[15], ch[iterations][ChannelsUsed], i;

int avg[ChannelsUsed];

int MixedSpeeds[2] = {0, 0};
int motorValues[2] = {1, 1};

#define motorUpdateTime 20
#define weaponESC 9

#define Motor1Dir0 10
#define Motor1Dir1 11
#define Motor2Dir0 6
#define Motor2Dir1 5

#define Motor1Enable 12
#define Motor2Enable 8

#define ReceiverPin 3

#define motorDeadband 0.1 //percent error between commanded and set motor pwm value

#define redLED A0
#define blueLED A5

bool alreadyRunning = false;
unsigned long lastTime = 0;

unsigned long escFrameStart = millis();
unsigned long escPulseStop = millis();
float escPulseWidth;

void setup() {
  Serial.begin(115200);
  pinMode(Motor1Dir0, OUTPUT);
  pinMode(Motor1Dir1, OUTPUT);
  pinMode(Motor2Dir0, OUTPUT);
  pinMode(Motor2Dir1, OUTPUT);

  pinMode(ReceiverPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ReceiverPin), read_me, FALLING);

  pinMode(Motor1Enable, OUTPUT);
  pinMode(Motor2Enable, OUTPUT);

  pinMode(redLED, OUTPUT);
  pinMode(blueLED, OUTPUT);
  
  digitalWrite(blueLED, LOW);
  digitalWrite(redLED, HIGH);
}

void loop() {
  read_rc();

  updateWeaponESC();

  if (ch[iterations-1][3] > 700) {
    if (!alreadyRunning) {
      digitalWrite(blueLED, HIGH);
      digitalWrite(redLED, LOW);
      enableDrive();
      alreadyRunning = true;
    }
    
    updateWeaponESC();
    Serial.println("FWD: " + String(avg[2]) + ";\tTRN: " + String(avg[0]));

    if (motorUpdateTime < millis() - lastTime) {
      if ( (abs( ((motorValues[0] - avg[2]) / float(motorValues[0]))) > motorDeadband) || (abs(((motorValues[1] - avg[0]) / float(motorValues[1]))) > motorDeadband) ) {
        lastTime = millis();
        updateMotors(avg[2], avg[0]);
      }
    }
  } else {
    if (alreadyRunning) {
      digitalWrite(blueLED, LOW);
      digitalWrite(redLED, HIGH);
      alreadyRunning = false;
      disableDrive();
    }
  }
}

void updateWeaponESC() {
  if (millis() >= escFrameStart + 20) {
    escPulseWidth = (float(map(ch[iterations-1][1], 0,1000, 1005, 1995))/1000.0);
    digitalWrite(weaponESC, HIGH);
    escPulseStop = millis() + escPulseWidth;
  } else  if (millis() >= escPulseStop) {
    digitalWrite(weaponESC, LOW);
  }
}

void enableDrive() {
  digitalWrite(Motor1Enable, HIGH);
  digitalWrite(Motor2Enable, HIGH);
}

void disableDrive() {
  digitalWrite(Motor1Enable, LOW);
  digitalWrite(Motor2Enable, LOW);
}

void updateMotors(int Ch1, int Ch2) {
  motorValues[0] = Ch1;
  motorValues[1] = Ch2;
  int linearValue = map(Ch1, 0, 1000, -255, 255);
  int rotationalValue = map(Ch2, 0, 1000, -255, 255);

  if (abs(linearValue) < motorDeadband*255) {
    linearValue = 0;
  }
  if (abs(rotationalValue) < motorDeadband*255) {
    rotationalValue = 0;
  }

  motorMixer(linearValue, rotationalValue);
  
  if (MixedSpeeds[0] > 0) {
    digitalWrite(Motor1Dir1, LOW);
    analogWrite(Motor1Dir0, abs(MixedSpeeds[0]));

  } else {
    digitalWrite(Motor1Dir0, LOW);
    analogWrite(Motor1Dir1, abs(MixedSpeeds[0]));

  }
  if (MixedSpeeds[1] > 0) {
    digitalWrite(Motor2Dir1, LOW);
    analogWrite(Motor2Dir0, abs(MixedSpeeds[1]));

  } else {
    digitalWrite(Motor2Dir0, LOW);
    analogWrite(Motor2Dir1, abs(MixedSpeeds[1]));

  }

}

void motorMixer(int fwd, int trn) {
  int speeds[2];
  if ( ( (fwd  < (255 * motorDeadband)) && (fwd > (-255 * motorDeadband))) || ((trn  < (255 * motorDeadband)) && (trn > (-255 * motorDeadband)))) {  //if either axis is zero
    speeds[0] = constrain((fwd + trn), -255, 255);
    speeds[1] = constrain((fwd - trn), -255, 255);
  } else if (trn > 0) {
    if (fwd > 0) {

      speeds[0] = max(abs(fwd), abs(trn));
      speeds[1] = 255 - (trn / 2) - ((255 - fwd) / 2);
    } else {

      speeds[0] = -1 * max(abs(fwd), abs(trn));
      speeds[1] = -255 + (trn / 2) + ((255 + fwd) / 2);
    }
  } else {
    if (fwd > 0) {

      speeds[0] = 255 - (-trn / 2) - ((255 - fwd) / 2);
      speeds[1] = max(abs(fwd), abs(trn));
    } else {

      speeds[0] = -255 + (-trn / 2) + ((255 + fwd) / 2);
      speeds[1] = -1 * max(abs(fwd), abs(trn));
    }
  }
  if (speeds[0] == 0) {
    digitalWrite(Motor1Enable, LOW);
  } else {
    digitalWrite(Motor1Enable, HIGH);
  }

  if (speeds[1] == 0) {
    digitalWrite(Motor2Enable, LOW);
  } else {
    digitalWrite(Motor2Enable, HIGH);
  }
  MixedSpeeds[0] = constrain(speeds[0],-255,255);
  MixedSpeeds[1] = constrain(speeds[1],-255,255);
}



//BEGIN MODIFIED PPM CODE BY abhilash_patel

void read_me()  {
 //this code reads value from RC reciever from PPM pin (Pin 2 or 3)
 //this code gives channel values from 0-1000 values 
 //    -: ABHILASH :-    //
  a=micros(); //store time value a when pin value falling
  c=a-b;      //calculating time inbetween two peaks
  b=a;        // 
  x[i]=c;     //storing 15 value in array
  i=i+1;      
  if(i==15){
    for(int j=0;j<15;j++) {
      ch1[j]=x[j];
    }
    i=0;
  }
}//copy store all values from temporary array another array after 15 reading 

void computeAVG() {
  for (int i = 0; i < ChannelsUsed; i++) {
    int sum = 0;
    for (int index = 0; index < iterations; index++) {
      sum += ch[index][i];
    }
    avg[i] = (sum / iterations);
  }
}

void read_rc() {
  int i,j,k=0;
  for(k=14;k>-1;k--){
    if(ch1[k]>10000){
      j=k;
    }
  }  //detecting separation space 10000us in that another array  
  for (int x = 1; x < iterations; x++) {
    for (int channel = 0; channel < ChannelsUsed; channel++) {
      ch[x-1][channel] = ch[x][channel];                   
    }
  }
  for(i=0;i < ChannelsUsed;i++){
    ch[iterations-1][i]=(ch1[i+j+1]-1000);
  }
  computeAVG();
}     //assign ChannelsUsed channel values after separation space
