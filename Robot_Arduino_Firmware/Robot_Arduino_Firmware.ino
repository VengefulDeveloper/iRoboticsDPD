#include <Servo.h>

//original ppm code by abhilash_patel

#define ChannelsUsed 4
#define iterations 3
unsigned long int a, b, c;
int x[15], ch1[15], ch[iterations][ChannelsUsed], i;

int avg[ChannelsUsed];

int MixedSpeeds[2] = {0, 0};
int motorValues[2] = {1, 1};

#define weaponESC 9

#define Motor1Dir0 10
#define Motor1Dir1 11
#define Motor2Dir0 6
#define Motor2Dir1 5

#define ReceiverPin 3

#define motorDeadband 0.15 //percent error between commanded and set motor pwm value
#define Motor1Multiplier 1  //change to -1 to reverse direction
#define Motor2Multiplier 1

#define redLED A0
#define blueLED A5

Servo ServoWeaponESC;


void setup() {
  Serial.begin(115200);
  pinMode(Motor1Dir0, OUTPUT);
  pinMode(Motor1Dir1, OUTPUT);
  pinMode(Motor2Dir0, OUTPUT);
  pinMode(Motor2Dir1, OUTPUT);

  pinMode(ReceiverPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ReceiverPin), read_me, FALLING);

  pinMode(12, OUTPUT);
  pinMode(8, OUTPUT);

  pinMode(A0, OUTPUT);
  pinMode(A5, OUTPUT);

  ServoWeaponESC.attach(weaponESC);
}

void loop() {
  read_rc();
  if (ch[iterations-1][3] > 700) {
    digitalWrite(blueLED, HIGH);
    digitalWrite(redLED, LOW);
    ServoWeaponESC.write(map(ch[iterations][1], 0, 1000, 0, 180));

    //ARMED
  } else {
    //NO RADIO CONNECTED
    digitalWrite(blueLED, LOW);
    digitalWrite(redLED, HIGH);
  }
}

void updateMotors(int M1, int M2) {
  motorValues[0] = M1;
  motorValues[1] = M2;
  int linearValue = map(M1, 0, 1000, -255, 255);
  int rotationalValue = map(M2, 0, 1000, -255, 255);

  motorMixer(linearValue, rotationalValue);

  if (MixedSpeeds[0] > 0) {
    digitalWrite(Motor1Dir1, LOW);
    delay(1);
    analogWrite(Motor1Dir0, MixedSpeeds[0]);
  } else {
    digitalWrite(Motor1Dir0, LOW);
    delay(1);
    analogWrite(Motor1Dir1, MixedSpeeds[0]);
  }
  if (MixedSpeeds[1] > 0) {
    digitalWrite(Motor2Dir1, LOW);
    delay(1);
    analogWrite(Motor2Dir0, MixedSpeeds[0]);
  } else {
    digitalWrite(Motor2Dir0, LOW);
    delay(1);
    analogWrite(Motor2Dir1, MixedSpeeds[0]);
  }
}

void motorMixer(int fwd, int trn) {
  int speeds[2];

  if ((fwd == 0) || (trn == 0)) {
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
      speeds[0] = 255 - (trn / 2) - ((255 - fwd) / 2);
      speeds[1] = max(abs(fwd), abs(trn));
    } else {
      speeds[0] = -255 + (trn / 2) + ((255 + fwd) / 2);
      speeds[1] = -1 * max(abs(fwd), abs(trn));
    }
  }

  MixedSpeeds[0] = speeds[0];
  MixedSpeeds[1] = speeds[1];
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
