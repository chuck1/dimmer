#include <Wire.h>

void hndReceive(int c)
{
  Serial.println("hndReceive");
  Serial.println(c);

  
  int i = 0;
  unsigned char buf[16];
  while(Wire.available()) {
    buf[i] = Wire.read();
    Serial.print(buf[i]);
    Serial.print(" ");
    ++i;
  }
  Serial.println();
}
void hndRequest()
{
  Serial.println("hndRequest");

  //Wire.write(2);
  short h = 257;
  Wire.write((char *)&h, 2);
}


void setup()
{
  Wire.begin(8);
  Wire.onReceive(hndReceive);
  Wire.onRequest(hndRequest);

  Serial.begin(9600);
}

void loop()
{
  delay(15);
}
