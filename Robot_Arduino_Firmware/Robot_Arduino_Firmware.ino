#include <Servo.h>

//pwm code by abhilash_patel

unsigned long int a,b,c;
int x[15],ch1[15],ch[7],i;
int motorValues[2] = {0,0};

#define weaponESC 3
#define Motor1 6
#define Motor2 9
#define motorDeadband 0.05
#define Motor1Multiplier 1  //change to -1 to reverse direction
#define Motor2Multiplier 1

Servo ServoWeaponESC;

void setup() {
  pinMode(2, INPUT_PULLUP);
  pinMode(Motor1, OUTPUT);
  pinMode(Motor2, OUTPUT);

  ServoWeaponESC.attach(weaponESC);
  attachInterrupt(digitalPinToInterrupt(2), read_me, FALLING);
}

void loop() {
  read_rc();//read values of channel n are stored in ch[n]
  ServoWeaponESC.write(map(ch[3],1000,2000,0,180));
  delay(100);
  if ((motorDeadband < (abs((ch[1] - motorValues[0]))/ motorValues[0])) || (motorDeadband < (abs((ch[2] - motorValues[1]))/ motorValues[1]))) {
    updateMotors();
  }
}

void updateMotors() {
  int linearValue = map(ch[1], 1000, 2000, 0, 1023);
  int rotationalValue = map(ch[2], 1000, 2000, 0, 1023);

  
}



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
  int i = 1, j, k=0;
  for(k=14;k>-1;k--){
    if(ch1[k]>10000){
      j=k;
    }
  }  //detecting separation space 10000us in that another array                     
  for(i=1;i<=6;i++){
    ch[i]=(ch1[i+j]-1000);
  }
}     //assign 6 channel values after separation space
