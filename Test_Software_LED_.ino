boolean SwitchOn;
int OutputPin = 2;
String PM25S, PM10S, ln;
char c;

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C  lcd(0x3F,2,1,0,4,5,6,7);

void setup() {
  Serial.begin(9600);
  pinMode(OutputPin, OUTPUT);
  lcd.begin (20,4);
  lcd.setBacklightPin(3,POSITIVE);
  lcd.setBacklight(HIGH);
}

void loop() {
  while (Serial.available()){
    c = Serial.read();
    if (c == ';'){
      PM10S = ln;
      ln = "";
    }
    else if (c == ','){
      PM25S = ln;
      ln = "";
    }
    else ln += c;
  }
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("PM10:" + PM10S + "   " + "PM2.5:" + PM25S);
  lcd.setCursor(0,2);
  if ((PM10S.toInt()<101)&&(PM25S.toInt()<101)){ 
    lcd.print("Low Risk            ");
    SwitchOn = false;
  }
  else if ((PM10S.toInt()<201)&&(PM25S.toInt()<201)){ 
    lcd.print("Moderate Risk       ");
    SwitchOn = true;
  }
  else if ((PM10S.toInt()>200)||2(PM25S.toInt()>200)){ 
    lcd.print("High Risk           ");
    SwitchOn = true;
  }
  else lcd.print("Error               ");

  if (SwitchOn) digitalWrite(OutputPin, HIGH);
  else digitalWrite(OutputPin, LOW);

  delay(5000);
}
