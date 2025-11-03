
#include <EEPROM.h>
#include <i2c_t3.h>

#include <SPI.h>
#include <Pixy.h>
#include <Teensy_LSM9DS1.h>

Pixy pixy;
uint16_t blocks;

LSM9DS1 imu;

#define LSM9DS1_M  0x1E // Would be 0x1C if SDO_M is LOW
#define LSM9DS1_AG  0x6B // Would be 0x6A if SDO_AG is LOW

#define PRINT_CALCULATED
//#define PRINT_RAW
#define PRINT_SPEED 250 // 250 ms between prints

// Earth's magnetic field varies by location. Add or subtract
// a declination to get a more accurate heading. Calculate
// your's here:
// http://www.ngdc.noaa.gov/geomag-web/#declination
#define DECLINATION -8.58 // Di q1eclination (degrees) in Boulder, CO.


////////////////////VARIABLEN///////////////////////
bool torwart;
bool lichtschranke;
int mL, mR, mH, mSchuss;

int mRInA = 2;          //RECHTS
int mRInB = 37;
int mRPWM = 5;

int mHInA = 1;         //HINTEN
int mHInB = 39;
int mHPWM = 29;

int mLInA = 35;         //LINKS
int mLInB = 36;
int mLPWM = 30;

//    ULTRASCHALL
int us[4];
long aUS;
long bUS;
long cUS;
long dUS;
boolean usR;
boolean usL;
//    INFRAROT
int valueIR, bestIR;
float x;
float y;

int ir[36];
int ballAngle;
int distance;
int xSmooth;
int ySmooth;
int irspeed = 100;

double ballX;
double ballY;

double driveAngle;

int BallA;

bool BallRest;
//Boden
int z;

//

//    KOMPASS
int Abweichung;
int drehung;
int oldZeit;
int oldAbweichung;
//   Magnetfeld
int winkel;
int minx;
int miny;
int maxx;
int maxy;
int xmittel;
int ymittel;
int add1;
//
int r;
int p;

//PIXY
int pixyx;
int pixywidth;
int pixyheight;
int pixyAngle;
int pixyAbweichung = 60;

int j;
int d;
int w;

bool t;


void Lichtschranke() {
  lichtschranke = false;
  if (analogRead(A0) < 700) {
    lichtschranke = true;
  }
}

void Pixyy() {

  blocks = pixy.getBlocks();
  if (blocks) {
    pixyx = pixy.blocks[0].x - 160;
    pixywidth = pixy.blocks[0].width;
    pixyheight = pixy.blocks[0].height;
  }
}

////////////////////////////////////////////////////////////////////ULTRASCHALL///////////////////////////////////////////////////////////////////////////////////////
void US() {
  int reading0 = 0;
  int reading1 = 0;
  int reading2 = 0;
  int reading3 = 0;

  if (millis() > aUS + 10) {
    Wire2.beginTransmission(113);
    Wire2.write(byte(0x02));                    //Wert in Register schreiben
    Wire2.endTransmission();
    Wire2.requestFrom(113, 2);                  //Wert aufrufen und in 2 bytes schreiben
    if (2 <= Wire2.available()) {
      reading0 = Wire2.read();                   //größerer Byte
      reading0 = reading0 << 8;                  //8 bits
      reading0 |= Wire2.read();                  //niedrigerer Byte
      us[0] = reading0;
    }
    Wire2.beginTransmission(113);
    Wire2.write(byte(0x00));
    Wire2.write(byte(0x51));
    Wire2.endTransmission(113);
    aUS = millis();
  }

  if (millis() > bUS + 10) {
    Wire2.beginTransmission(114);
    Wire2.write(byte(0x02));                    //Wert in Register schreiben
    Wire2.endTransmission();
    Wire2.requestFrom(114, 2);                  //Wert aufrufen und in 2 bytes schreiben
    if (2 <= Wire2.available()) {
      reading1 = Wire2.read();                   //größerer Byte
      reading1 = reading1 << 8;                  //8 bits
      reading1 |= Wire2.read();                  //niedrigerer Byte
      us[1] = reading1;
    }
    Wire2.beginTransmission(114);
    Wire2.write(byte(0x00));
    Wire2.write(byte(0x51));
    Wire2.endTransmission(114);
    bUS = millis();
  }

  if (millis() > cUS + 10) {
    Wire2.beginTransmission(115);
    Wire2.write(byte(0x02));                    //Wert in Register schreiben
    Wire2.endTransmission();
    Wire2.requestFrom(115, 2);                  //Wert aufrufen und in 2 bytes schreiben
    if (2 <= Wire2.available()) {
      reading2 = Wire2.read();                   //größerer Byte
      reading2 = reading2 << 8;                  //8 bits
      reading2 |= Wire2.read();                  //niedrigerer Byte
      us[2] = reading2;
    }
    Wire2.beginTransmission(115);
    Wire2.write(byte(0x00));
    Wire2.write(byte(0x51));
    Wire2.endTransmission(115);
    cUS = millis();
  }

  if (millis() > dUS + 10) {
    Wire2.beginTransmission(116);
    Wire2.write(byte(0x02));                    //Wert in Register schreiben
    Wire2.endTransmission();
    Wire2.requestFrom(116, 2);                  //Wert aufrufen und in 2 bytes schreiben
    if (2 <= Wire2.available()) {
      reading3 = Wire2.read();                   //größerer Byte
      reading3 = reading3 << 8;                  //8 bits
      reading3 |= Wire2.read();                  //niedrigerer Byte
      us[3] = reading3;
    }
    Wire2.beginTransmission(116);
    Wire2.write(byte(0x00));
    Wire2.write(byte(0x51));
    Wire2.endTransmission(116);
    dUS = millis();
  }

}

void Torwart() {
  boolean mid = false;
  int dTor = 40;
  if (abs(us[0] - us[2]) < 40 && (us[0] + us[2]) > 100) {
    mid = true;
    x = 0;
  }
  if (!mid) {
    x = -50;
    if (us[2] < us[0]) {
      x = 50;
    }
  }
  if (us[3] > dTor) {
    y = -50;
  }
  else if (us[3] < dTor) {
    y = 0;
  }
}
////////////////////////////////////////////////////////////////////KOMPASS/////////////////////////////////////////////////////////////////////////////////////////////////
void MagDeg() {
  imu.readMag();
  xmittel = (maxx + minx) / 2;
  ymittel = (maxy + miny) / 2;
  winkel = atan2((imu.my - ymittel), (imu.mx - xmittel)) * 180. / M_PI;                                                       //KOMPASS TESTEN
}

void CompassRotate() {
  winkel = winkel - add1;
  if (winkel < -180) {
    winkel = 180 - (-winkel - 180);
  }
  if (winkel > 180) {
    winkel = -180 + (winkel - 180);
  }
  Abweichung = winkel;
  double p = 2.3;                    //p = 2.0
  double d = 1.2;                    //d = 1.1
  long zeit = millis() - oldZeit;

  int drehung = p * Abweichung + (d / zeit) * (Abweichung - oldAbweichung);
  oldAbweichung = Abweichung;
  oldZeit = zeit;
  r = drehung / 3;                                                                     //DREHUNG
}

//////////////////////////////////////////////////////////////////////////////////////////////////////77
void IR () {
  ir[0] = analogRead(A6);
  ir[1] = analogRead(A7);
  ir[2] = analogRead(A8);
  ir[3] = analogRead(A9);
  ir[4] = analogRead(A11);
  ir[5] = analogRead(A10);
  digitalWrite(31, HIGH); digitalWrite(32, HIGH); digitalWrite(33, HIGH); digitalWrite(34, HIGH); ir[6] = analogRead(15);   //C15
  digitalWrite(31, LOW); digitalWrite(32, HIGH); digitalWrite(33, HIGH); digitalWrite(34, HIGH); ir[7] = analogRead(15);    //C14
  digitalWrite(31, HIGH); digitalWrite(32, LOW); digitalWrite(33, HIGH); digitalWrite(34, HIGH); ir[8] = analogRead(15);    //C13
  digitalWrite(31, LOW); digitalWrite(32, LOW); digitalWrite(33, HIGH); digitalWrite(34, HIGH); ir[9] = analogRead(15);     //C12
  digitalWrite(31, HIGH); digitalWrite(32, HIGH); digitalWrite(33, LOW); digitalWrite(34, HIGH); ir[10] = analogRead(15);   //C11

  digitalWrite(31, LOW); digitalWrite(32, HIGH); digitalWrite(33, LOW); digitalWrite(34, HIGH); ir[11] = analogRead(15);   //C10
  digitalWrite(31, HIGH); digitalWrite(32, LOW); digitalWrite(33, LOW ); digitalWrite(34, HIGH); ir[12] = analogRead(15);       //C9
  digitalWrite(31, LOW); digitalWrite(32, LOW); digitalWrite(33, LOW); digitalWrite(34, HIGH); ir[13] = analogRead(15);     //C8
  digitalWrite(31, HIGH); digitalWrite(32, HIGH); digitalWrite(33, HIGH); digitalWrite(34, LOW); ir[14] = analogRead(15);     //C7
  digitalWrite(31, LOW); digitalWrite(32, HIGH); digitalWrite(33, HIGH); digitalWrite(34, LOW); ir[15] = analogRead(15);     //C6

  digitalWrite(31, HIGH); digitalWrite(32, LOW); digitalWrite(33, HIGH); digitalWrite(34, LOW); ir[16] = analogRead(15);     //C5
  digitalWrite(31, LOW); digitalWrite(32, LOW); digitalWrite(33, HIGH); digitalWrite(34, LOW); ir[17] = analogRead(15);     //C4
  digitalWrite(31, HIGH); digitalWrite(32, HIGH); digitalWrite(33, LOW); digitalWrite(34, LOW); ir[18] = analogRead(15);     //C3
  digitalWrite(31, LOW); digitalWrite(32, HIGH); digitalWrite(33, LOW); digitalWrite(34, LOW); ir[19] = analogRead(15);     //C2
  digitalWrite(31, HIGH); digitalWrite(32, LOW); digitalWrite(33, LOW); digitalWrite(34, LOW); ir[20] = analogRead(15);     //C1

  digitalWrite(31, LOW); digitalWrite(32, LOW); digitalWrite(33, LOW); digitalWrite(34, LOW); ir[21] = analogRead(15);      //C0

  ir[22] = 10000;
  ir[23] = 10000;

  ir[24] = analogRead(A21);

  ir[25] = 10000;
  ir[26] = 10000;

  ir[27] = analogRead(A22);

  ir[28] = 10000;
  ir[29] = 10000;

  ir[30] = analogRead(A2);

  ir[31] = 10000;
  ir[32] = 10000;

  ir[33] = analogRead(A3);
  ir[34] = analogRead(A4);
  ir[35] = analogRead(A5);

}

void IRAlgorithm() {
  pixyAngle = 0;
  valueIR = 1023;
  bestIR = 36;
  BallA = 100;
  int v = 1;
  for (int i = 0; i < 36; i++) {
    if (ir[i] < valueIR) {
      bestIR = i;
      valueIR = ir[i];
    }
  }

  if (bestIR >= 33 || bestIR <= 3) {
    BallA = 0;                            //0
  }
  else if (bestIR >= 4 && bestIR <= 14) {
    BallA = 1;                            //1
  }
  else if (bestIR >= 15 && bestIR <= 21) {
    BallA = 2;                            //2
  }
  else if (bestIR >= 24 && bestIR <= 30) {
    BallA = 3;                            //3
  }

  distance = 35;                             //Random Entfernung zum Ball festgelegt !!!!!!!!!!!!!!!!!!!!!!distance > x/y-Smooth
  if (valueIR < 500) {
    distance = 25;
    if (valueIR < 400) {
      distance = 20;
    }
  }

  ballAngle = bestIR * 10;
  if (ballAngle == 0) {
    ballAngle = 1;
  }
  if (ballAngle == 180) {
    ballAngle = 179;
  }

  xSmooth = 20;
  ySmooth = 20;

  ballX = cos(ballAngle * PI / 180) * distance;
  ballY = sin(ballAngle * PI / 180) * distance;

  if (ballAngle > 0 && ballAngle < 180) {
    driveAngle = atan2(ballY - ySmooth - 3.5 , ballX);
    if (ballAngle >= 60 && ballAngle <= 120) {
      driveAngle = 0.5 * PI;
      v = 1.3;
      if (abs(pixyx) > pixyAbweichung && lichtschranke) {
        pixyAngle = 30;
        if (pixyx > 0) {
          pixyAngle = -30;
        }
      }
    }
  }
  else if (ballAngle > 180 && ballAngle <= 270) {
    if (abs(ballX) > abs(xSmooth)) {
      driveAngle = 1.5 * PI - atan2(abs(ballX) - abs(xSmooth) , abs(ballY));
    }
    else if (abs(ballX) < abs(xSmooth)) {
      driveAngle = 1.5 * PI  + atan2(abs(xSmooth) - abs(ballX) , abs(ballY)) ;
    }
  }
  else if (ballAngle > 270) {
    if (ballX < xSmooth) {
      driveAngle = 1.5 * PI - atan2(xSmooth - ballX , abs(ballY));
    }
    else if (ballX > xSmooth) {
      driveAngle = 1.5 * PI  + atan2(ballX - xSmooth, abs(ballY));
    }
  }
  driveAngle = driveAngle + pixyAngle;
  x = cos(driveAngle) * irspeed * v;
  y = sin(driveAngle) * irspeed * v;

  if (valueIR > 900) {
    valueIR = 899;
    torwart = true;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void XY() {
  mL = (x / 3) + (y / 1.7);
  mR = (x / 3) - (y / 1.7);
  mH = (2 * x) / 3;
}

////////////////////////////////////////////////////////////////////////////////

void Kalibrieren (int k = 0) {
  beep();
  while (k == 0) {
    if (digitalRead(24) == LOW) {
      minx = 10000;
      miny = 10000;
      maxx = -10000;
      maxy = -10000;
      int v = 7;
      mL = v;
      mR = v;
      mH = -v;
      motorenAnsteuern();

      for (int a = 0; a <= 300; a++) {
        imu.readMag();
        if (imu.mx > maxx) {
          maxx = imu.mx;
        }
        if (imu.mx < minx) {
          minx = imu.mx;
        }
        if (imu.my > maxy) {
          maxy = imu.my;
        }
        if (imu.my < miny) {
          miny = imu.my;
        }
        delay(10);
      }

      EEPROM.put(0, minx);
      EEPROM.put(10, miny);
      EEPROM.put(20, maxx);
      EEPROM.put(30, maxy);
      mL = 0;
      mR = 0;
      mH = 0;
      motorenAnsteuern();
    }
    if (digitalRead(25) == LOW) {
      EEPROM.get(0, minx);
      EEPROM.get(10, miny);
      EEPROM.get(20, maxx);
      EEPROM.get(30, maxy);
      imu.readMag();
      xmittel = (maxx + minx) / 2;
      ymittel = (maxy + miny) / 2;
      add1 = atan2((imu.my - ymittel), (imu.mx - xmittel)) * 180. / M_PI;
      k = 1;
    }
  }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////77
void motorenAnsteuern() {            // Die Leistungsstufe wird an die Motoren ausgegeben
  if (valueIR < 900) {
    mL = mL - r;
    mR = mR - r;
    mH = -mH - r;
  }
  else {
    mL = -r;
    mR = -r;
    mH = -r;
  }

  if (mR > 255) mR = 255;
  if (mL > 255) mL = 255;
  if (mH > 255) mH = 255;

  if (mR < -255) mR = -255;
  if (mL < -255) mL = -255;
  if (mH < -255) mH = -255;

  //////////////RECHTS///////////////////////
  if (mR >= 0) {
    digitalWrite(mRInA, HIGH);
    digitalWrite(mRInB, LOW);
    analogWrite(mRPWM, mR);
  }
  else {
    digitalWrite(mRInA, LOW);
    digitalWrite(mRInB, HIGH);
    analogWrite(mRPWM, -mR);
  }
  /////////////LINKS/////////////////////////
  if (mL >= 0) {
    digitalWrite(mLInA, HIGH);
    digitalWrite(mLInB, LOW);
    analogWrite(mLPWM, mL);
  }
  else {
    digitalWrite(mLInA, LOW);
    digitalWrite(mLInB, HIGH);
    analogWrite(mLPWM, -mL);
  }
  /////////////HINTEN////////////////////////
  if (mH >= 0) {
    digitalWrite(mHInA, HIGH);
    digitalWrite(mHInB, LOW);
    analogWrite(mHPWM, mH);
  }
  else {
    digitalWrite(mHInA, LOW);
    digitalWrite(mHInB, HIGH);
    analogWrite(mHPWM, -mH);
  }
}

void beep () {
  mL = mR = mH = 0;
  r = 2;
  motorenAnsteuern();
  delay(30);
  mL = mR = mH = 0;
  r = 0;
  motorenAnsteuern();
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial2.begin(115200);
  Serial3.begin(115200);
  delay(500);
  Serial3.print("$$$");
  delay(500);
  Serial3.println("C,000666D2CCC2");

  pixy.init();
  imu.settings.device.commInterface = IMU_MODE_I2C;
  imu.settings.device.mAddress = LSM9DS1_M;
  imu.settings.device.agAddress = LSM9DS1_AG;

  if (!imu.begin())
  {
    Serial.println("Failed to communicate with LSM9DS1.");
    Serial.println("Double-check wiring.");
    Serial.println("Default settings in this sketch will " \
                   "work for an out of the box LSM9DS1 " \
                   "Breakout, but may need to be modified " \
                   "if the board jumpers are.");
    while (1) {
    }

  }

  pinMode(2, OUTPUT);   //MOTOR R
  pinMode(37, OUTPUT);
  pinMode(38, OUTPUT);  //MOTOR Schuss
  pinMode(11, OUTPUT);
  pinMode(1, OUTPUT);  //MOTOR H
  pinMode(39, OUTPUT);
  pinMode(35, OUTPUT);  //MOTOR L
  pinMode(36, OUTPUT);
  pinMode(34, OUTPUT);  //Multiplexer S3
  pinMode(33, OUTPUT);  //Multiplexer S2
  pinMode(32, OUTPUT);  //Multiplexer S1
  pinMode(31, OUTPUT);  //Multiplexer S0

  pinMode(24, INPUT);   //Knopf 1
  pinMode(25, INPUT);   //Knopf 2
  pinMode(26, INPUT);   //Knopf 3
  pinMode(27, INPUT);   //Knopf 4
  pinMode(28, INPUT);   //Knopf 5


  pinMode(A6, INPUT);    //IR0
  pinMode(A7, INPUT);    //IR1
  pinMode(A8, INPUT);    //IR2
  pinMode(A9, INPUT);    //IR3
  pinMode(A11, INPUT);   //IR4
  pinMode(A10, INPUT);   //IR5
  pinMode(A21, INPUT);   //IR22
  pinMode(A22, INPUT);   //IR23
  pinMode(A2, INPUT);    //IR24
  pinMode(A3, INPUT);    //IR25
  pinMode(A4, INPUT);    //IR26
  pinMode(A5, INPUT);    //IR27


  pinMode(15, INPUT);   //Multiplexer Sig
  pinMode(A0, INPUT);   //Lichtschranke

  Wire2.begin();

  valueIR = 899;
  Kalibrieren();
  beep();
  j = 0;
  d = 4;
}

bool run;
void loop() {
  // put your main code here, to run repeatedl
  Lichtschranke();
  Pixyy();
  torwart = false;
  if (Serial2.available()) {
    z = Serial2.read();
  }
  if (Serial3.available()) {
    p = Serial3.read();
  }
  US();
  IR();
  IRAlgorithm();
  if (p == 5) {
    torwart = true;
  }
  if (torwart) {
    Torwart();
  }
  if (z < 12) {
    x = cos(z * PI * 6) * 100;
    y = sin(z * PI * 6) * 100;
    j = 0;
    BallRest = true;
  }
  else if (z >= 30) {
    if (j == 0 && (z - 30) == BallA) {
      d = BallA;
      j = 1;
    }
    if (BallA != d) {
      BallRest = false;
    }
    if ((z - 30) == BallA && BallRest) {
      x = 0;
      y = 0;
    }
  }
  XY();
  MagDeg();
  CompassRotate();
  motorenAnsteuern();
  while (Serial2.available()) {
    Serial2.read();
  }
}


