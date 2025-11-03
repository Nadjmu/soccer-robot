
//for (int i = 0; i < 4; i++) {
//    if (q[i] < schwell) {
//      qCounter++;
//      boden = true;
//      qSum = qSum + i * 90 + 45;
//    }
//  }

int schwell = 3500;
int schwellka = 3300;
int ka[6];
int ki[18];
int q[4];

int kaCounter;
int qCounter;
int kaSum;
int qSum;

int driveDirec;

int z;
int dt = 8500;
int b; 

unsigned long topR, bottomR, right, left, topL, bottomL;

unsigned long FloorTimer;
int FloorCounter;
int k;
////////////

bool boden;

void BodenRead() {
  boden = false;

  ki[0] = analogRead(A10);
  ki[1] = analogRead(A5);
  ki[2] = 10000;
  ki[3] = 10000;
  ki[4] = 10000;
  ki[5] = 10000;
  ki[6] = 10000;
  ki[7] = 10000;
  ki[8] = analogRead(A20);
  ki[9] = analogRead(A15);
  ki[10] = analogRead(A2);
  ki[11] = analogRead(A13);
  ki[12] = analogRead(A21);
  ki[13] = analogRead(A14);
  ki[14] = analogRead(A7);
  ki[15] = analogRead(A18);
  ki[16] = analogRead(A17);
  ki[17] = analogRead(A6);

  ka[0] = analogRead(A22);
  ka[1] = analogRead(A11);
  ka[2] = analogRead(A1);
  ka[3] = analogRead(A3);
  ka[4] = analogRead(A16);
  ka[5] = analogRead(A19);

  q[0] = analogRead(A12);
  q[1] = analogRead(A4);
  q[2] = analogRead(A8);
  q[3] = analogRead(A9);

  analogReadResolution(12);
  
}

void BodenProcess () {
  bool c = false; 
  kaCounter = 0;
  kaSum = 0;
  
  if (millis() > FloorTimer + dt) {
    FloorCounter = 0;
    }
  ////////////////////////////////  
  if (millis() > topR + 1000) {
    topR = 0;
  }
  if (millis() > topL + 1000) {
    topL = 0;
  }
  if (millis() > bottomL + 1000) {
    bottomL = 0;
  }
  if (millis() > bottomR + 1000) {
    bottomR = 0;
  }
  //Äußere Bodensensoren -
  if (ka[0] < schwellka) {
    right = millis();
    c = true;
    b = 0; 
  }
  if (ka[1] < schwellka) {
    topR = millis();
    if (ka[2] < schwellka) {
      c = true;
      b = 1;
      }
  }
  if (ka[2] < schwellka) {
    topL = millis();
  }
  if (ka[3] < schwellka) {
    left = millis();
    c = true;
    b = 2; 
  }
  if (ka[4] < schwellka) {
    bottomL = millis();
    if (ka[5] < schwellka) {
      c = true;
      b = 3;
      }
  }
  if (ka[5] < schwellka) {
    bottomR = millis();
  }
  //--
  //Äußerer Kreis
  for (int i = 0; i < 6; i++) {
    if (ka[i] < schwellka) {
      int a = 0;
      if (right > left && (bottomR || bottomL) != 0 && i < 3) {
        a = 6;
      }
      kaCounter++;
      boden = true;
      kaSum = kaSum + (i + a) * 60;
    }
  }

  //Innerer Kreis
  if (!boden) {
    for (int i = 0; i < 18; i++) {
      if (ki[i] < schwell) {
        boden = true;
      }
    }
  }
  //QUADRAT
  for (int i = 0; i < 4; i++) {
    if (q[i] < schwell) {
      //int a = 0;
      //if (bottomR != 0 && i == 1) {
        //a = 6;
      //}
      //qCounter++;
      boden = true;
      //qSum = qSum + (i + a) * 90 + 45;
    }
  }
/////////////////////////////////////////////////////////////////
  if (!c && k == 1) {
    k = 0;
    FloorCounter++;
  }
  else if (c) {
    k = 1;
    FloorTimer = millis();
  }
////////////////////////////////////////////////////////////////
}
  
//
void DireC() {
  if (kaCounter >= 2) {
    driveDirec = kaSum / kaCounter;
  }
  else if (kaCounter == 1) {
    if (ka[0] < schwellka && bottomR == 0 && topR == 0) {
      driveDirec = 0;
    }
    else if (ka[3] < schwellka && bottomR == 0 && topR == 0) {
      driveDirec = 180;
    }
    else {
      driveDirec = driveDirec;
    }
  }
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial1.begin(115200);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  pinMode(A4, INPUT);
  pinMode(A5, INPUT);
  pinMode(A6, INPUT);
  pinMode(A7, INPUT);
  pinMode(A8, INPUT);
  pinMode(A9, INPUT);
  pinMode(A10, INPUT);
  pinMode(A11, INPUT);
  pinMode(A12, INPUT);
  pinMode(A13, INPUT);
  pinMode(A14, INPUT);
  pinMode(A15, INPUT);
  pinMode(A16, INPUT);
  pinMode(A17, INPUT);
  pinMode(A18, INPUT);
  pinMode(A19, INPUT);
  pinMode(A20, INPUT);
  pinMode(A21, INPUT);
  pinMode(A22, INPUT);

  left = right = bottomR = topR = topL = bottomL = 0;
  FloorTimer = 0;
  FloorCounter = 0;
  k = 0;
}

void loop() {
  // put your main code here, to run repeatedly:
  if (FloorCounter >= 8) {
    FloorCounter = 0;
    }
  BodenRead();
  BodenProcess();
  //Serial.print("BODEN:"); Serial.print(boden); Serial.print("    k:"); Serial.print(k); Serial.print("   FLOORCOUNTER:"); Serial.print(FloorCounter); Serial.print("  b:"); Serial.println(b); 
  DireC();
  z = driveDirec - 180;
  if (z < 0) {
    z = z + 360;
  }
  z = z / 30;
  if (!boden) {                                                    //Jessi ist nice ... mega nice 
    if (FloorCounter >= 5 && FloorCounter <= 7) {
      z = 30 + b;
      } 
    else {
      z = 20;
      }
  }
  Serial1.write(z);
  //Serial.print("BODEN:"); Serial.print(boden); Serial.print("           kaCounter:"); Serial.print(kaCounter); Serial.print("               z:"); Serial.println(z*30);
}


