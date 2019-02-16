const String WiFiname = "CAYABS";
const String Pass = "titi1234";
const String PM25API = "XW8KYL7WZBVPF9PS";
const String PM10API = "FU1OULGH590QLL88";
const String DateAPI = "QN1K6NCJWTUZ99OB";
const String TimeAPI = "71IRWXS81X4GMJ0O";
String response, TimeS, DateS, PM25S, PM10S, GET;
boolean errorRX;

#include <SD.h>
#include <SPI.h>
int pinCS = 53;
File DataLog;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial1.begin(9600);
  pinMode(pinCS, OUTPUT);
  while (SD.begin()!=true)
  {
    Serial.println("SD Card: ERROR");
    delay(1000);
  } 
  Serial.println("SD Card: READY");
  sendData("AT+CWQAP\r\n", 100);
  sendData("AT+CWJAP=\"" + WiFiname + "\",\"" + Pass + "\"\r\n", 100);
  TimeS = Time();
  DateS = Date();
  PM10S = PM(PM10API);
  PM25S = PM(PM25API);
  delay(3000);
  DataLog = SD.open("LogTest1.txt", FILE_WRITE);
  if (DataLog) {
    DataLog.println(TimeS + ";" + DateS + ";" + PM10S + ";" + PM25S);
    DataLog.close();
    Serial.println("done.");
  } else {
    Serial.println("error"); 
  }
  delay(3000);
  DataLog = SD.open("LogTest1.txt");
  if (DataLog) {
    Serial.println("LogTest1.txt:");
    while (DataLog.available()) {
      Serial.write(DataLog.read());
    }
    DataLog.close();
  } else {
    Serial.println("error");
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}
void sendData(String command, const int TimeDelay){
  char c;
  String ln;
  response = "";
  boolean endRX = false;
  Serial1.print(command);
  while (!endRX){
    while ( Serial1.available()){
      c = Serial1.read();
      Serial.write(c);
      if (c == '\n'){
        if (ln.startsWith("OK") || ln.startsWith("+IPD")){
          endRX = true;
        }
        else if (ln.startsWith("ERROR") || ln.startsWith("FAIL") || ln.startsWith("SEND FAIL")){
          endRX = true;
          errorRX = true;
        }
        response += ln + "\n";
        ln = "";
      }
      else ln += c;
    }
  }
  delay(TimeDelay);
}

String Time()
{
  GET = "GET /apps/thinghttp/send_request?api_key=" + TimeAPI + "\r\nHost:api.thingspeak.com\r\n";
  sendData("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",80\r\n", 100);
  sendData("AT+CIPSEND=" + String(GET.length()) + "\r\n", 100);
  sendData(GET, 100); 
  if (errorRX){
    errorRX = false;
    return ("ERR");
  }
  int i = 0; String filtered;
  while (response.charAt(i) != ':') i++;
  i++;
  int j = i;
  while (response.charAt(j) != ':') j++;
  j++;
  while (response.charAt(j) != ':') j++;
  filtered = response.substring(i, j);
  return (filtered);
}

String Date()
{
  GET = "GET /apps/thinghttp/send_request?api_key=" + DateAPI + "\r\nHost:api.thingspeak.com\r\n";
  sendData("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",80\r\n", 100);
  sendData("AT+CIPSEND=" + String(GET.length()) + "\r\n", 100);
  sendData(GET, 100); 
  if (errorRX){
    errorRX = false;
    return ("ERR");
  }
  int i = 0;
  while (response.charAt(i) != ',') i++;
  while (response.charAt(i) != ' ') i++;
  i++;
  response = response.substring(i);
  int j = 0;
  while (response.charAt(j) != 'C') j++;
  response = response.substring(0, j);
  return (response);
}

String PM(String type)
{
  GET = "GET /apps/thinghttp/send_request?api_key=" + type + "\r\nHost:api.thingspeak.com\r\n";
  sendData("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",80\r\n", 100);
  sendData("AT+CIPSEND=" + String(GET.length()) + "\r\n", 100);
  sendData(GET, 100);  
  if (errorRX){
    errorRX = false;
    return ("ERR");
  }
  int i = 0;
  while (response.charAt(i) != ':') i++;
  i++;
  response = response.substring(i);
  int j = 0;
  while (response.charAt(j) != 'C') j++;
  response = response.substring(0, j);
  return (response);
}
