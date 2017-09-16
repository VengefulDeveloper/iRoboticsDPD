#include <Servo.h>

//original ppm code by abhilash_patel

#define ChannelsUsed 4
#define iterations 3
unsigned long int a,b,c;
int x[15],ch1[15],ch[iterations][6],i;

int motorValues[2] = {0,0};

#define weaponESC 9

#define Motor1Dir0 10
#define Motor1Dir1 11
#define Motor2Dir0 6
#define Motor2Dir1 5

#define ReceiverPin 3

#define motorDeadband 0.05 //percent error between commanded and set motor pwm value
#define Motor1Multiplier 1  //change to -1 to reverse direction
#define Motor2Multiplier 1

Servo ServoWeaponESC;

void setup() {
  pinMode(Motor1Dir0, OUTPUT);
  pinMode(Motor1Dir1, OUTPUT);
  pinMode(Motor2Dir0, OUTPUT);
  pinMode(Motor2Dir1, OUTPUT);
  
  pinMode(ReceiverPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ReceiverPin), read_me, FALLING);
  
  ServoWeaponESC.attach(weaponESC);
}

void loop() {
  read_rc();
  ServoWeaponESC.write(map(ch[1],0,1000,0,180));
  
  if ((motorDeadband < (abs((ch[0] - motorValues[0]))/ motorValues[0])) || (motorDeadband < (abs((ch[3] - motorValues[1]))/ motorValues[1]))) {
    updateMotors();
  }

  //needs to be tuned
  delay(100);
  //end needs to be tuned
}

void updateMotors() {
  motorValues[0] = ch[0];
  motorValues[1] = ch[3];
  int linearValue = map(ch[1], 0, 1000, 0, 1023);
  int rotationalValue = map(ch[2], 0, 1000, 0, 1023);

  //need to finish mixing code
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

void read_rc(){
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
}     //assign ChannelsUsed channel values after separation space
