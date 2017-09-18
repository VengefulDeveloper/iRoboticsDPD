#define weaponESC 9
unsigned long escFrameStart = micros();
unsigned long escPulseStop = micros();
unsigned long lastTime = micros();
int escPulseWidth;

int inputSpeed;

void setup() {
  pinMode(weaponESC, OUTPUT);
  Serial.begin(115200);
  Serial.println("Ready");
}

void loop() {
  if (Serial.available()) {
    inputSpeed = Serial.parseInt();
    Serial.println("ESC Speed [0,1000] Set To " + String(inputSpeed));
  }
  updateWeaponESC();

  
}

void updateWeaponESC() {
  if (micros() >= escFrameStart + 20000) {
    escPulseWidth = map(inputSpeed, 0, 1000, 1005, 1995);
    digitalWrite(weaponESC, HIGH);
    escFrameStart = micros();
    escPulseStop = escFrameStart + escPulseWidth;
  } else if (micros() >= escPulseStop) {
    digitalWrite(weaponESC, LOW);
  }
}
