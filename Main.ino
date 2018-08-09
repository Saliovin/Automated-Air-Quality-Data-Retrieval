//--------------------------------------------------------------------------------//
//Changeable Values---------------------------------------------------------------//
//--------------------------------------------------------------------------------//
const String WiFiname = "XXXXXX";  //WiFi SSID
const String Pass = "XXXXXX";  //WiFi password
const String PM25API = "XXXXXXXXXXXXXXXX";  //API key for the PM25 data
const String PM10API = "XXXXXXXXXXXXXXXX";  //API key for the PM10 data
const String DateAPI = "XXXXXXXXXXXXXXXX";  //API key for the Date data
const String TimeAPI = "XXXXXXXXXXXXXXXX";  //API key for the Time data
const String ChannelAPI = "XXXXXXXXXXXXXXXX";  //API key for the Thingspeak channel
const String FileName = "Log.txt";  //Text file name where the data is saved
//--------------------------------------------------------------------------------//
//End of Changeable Values--------------------------------------------------------//
//--------------------------------------------------------------------------------//

//--------------------------------------------------------------------------------//
//Do Not Change the Program Below-------------------------------------------------//
//Unless You Know What You're Doing-----------------------------------------------//
//--------------------------------------------------------------------------------//
String response, TimeS, DateS, PM25S, PM10S, PM10Prev, PM25Prev, GET;  //Variables to store the different data needed by the module
boolean errorRX, AllSame;  //errorRX:Variable to indicate if there is an error in the transmission or the receiving of data; AllSame:Variable to indicate if all 3 retreivals are equal;

//LCD Display Setup----------------------------------//
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C  lcd(0x3F,2,1,0,4,5,6,7);
//---------------------------------------------------//

//SD Card Module Setup-------------------------------//
#include <SD.h>
#include <SPI.h>
File DataLog;
const int pinCS = 53;
//---------------------------------------------------//

void setup()
{
  //Bitrate Setup--------------------------------------//
  Serial.begin(9600);
  Serial1.begin(9600);
  //---------------------------------------------------//
  //LCD Display Initialization-------------------------//
  lcd.begin (20,4);
  lcd.setBacklightPin(3,POSITIVE);
  lcd.setBacklight(HIGH);
  //---------------------------------------------------//
  //SD Card Module Setup-------------------------------//
  lcd.print("Initializing SD card");
  while (!SD.begin(53)) {
    lcd.clear();
    lcd.print("Failed Initialize");
  }
  lcd.clear();
  lcd.print("SD Card Ready");
  //---------------------------------------------------//
  //ESP 8266 Setup-------------------------------------//
  lcd.clear();  //Clear the LCD screen 
  lcd.print("Disconnecting WiFi");
  sendData("AT+CWQAP\r\n", 100);  //AT command to diconnect WiFi connection
  lcd.clear();
  lcd.println("Joining WiFi:");
  lcd.print(WiFiname);
  sendData("AT+CWJAP=\"" + WiFiname + "\",\"" + Pass + "\"\r\n", 100);  //AT command to connect to set WiFi connection
  //---------------------------------------------------//
}

void loop()
{
  //--------------------------------------------------------------------------------//
  //This is the main program loop. It will first retreive and sort the different----//
  //data types then store them in variables. The contents of these variables are----//
  //displayed on the LCD and then used to update the Thingspeak channel. Lastly, the//
  //data is stored in a text file on the sd card inserted in the sd car module.-----//
  //--------------------------------------------------------------------------------//
  //Retreival of data----------------------------------//
  while (!AllSame){  //Do not continue when the 3 samples are not the same
    for (int i = 0; i < 3; i++){  //Retreive data 3 times and check is they are the same
      PM25Prev = PM25S;
      PM10Prev = PM10S;
      lcd.setCursor(0,3);
      lcd.print("Retreiving PM25     ");
      PM25S = PM(PM25API);
      lcd.setCursor(0,3);
      lcd.print("Retreiving PM10     ");
      PM10S = PM(PM10API);
      if ((PM25S == PM25Prev)&&(PM10S == PM25Prev)) AllSame = true;
      else AllSame = false; 
    }
  }
  AllSame = false;  //Reset boolean else the program above will be skipped
  lcd.setCursor(0,3);
  lcd.print("Retreiving Time     ");
  TimeS = Time();
  lcd.setCursor(0,3);
  lcd.print("Retreiving Date     ");
  DateS = Date();
  //---------------------------------------------------//
  //Displaying of data---------------------------------//
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(TimeS + " " + DateS);
  lcd.setCursor(0,1);
  lcd.print("PM10:" + PM10S + "   " + "PM2.5:" + PM25S);
  lcd.setCursor(0,2);
  //Check what level of risk does the data fall into
  if ((PM10S.toInt()&&PM25S.toInt())<101) lcd.print("Low Risk            ");
  else if ((PM10S.toInt()&&PM25S.toInt())<201) lcd.print("Moderate Risk       ");
  else if ((PM10S.toInt()&&PM25S.toInt())>200) lcd.print("High Risk           ");
  else lcd.print("Error               ");
  //---------------------------------------------------//
  //Updating of Thingspeak channel---------------------//
  Update(PM10S, PM25S);
  //---------------------------------------------------//
  //Log to SD Card-------------------------------------//
  lcd.setCursor(0,3);
  lcd.print("Logging to SD Card  ");
  DataLog = SD.open(FileName, FILE_WRITE);
  //If the file is successfully opened, write the data
  if (DataLog) {
    DataLog.println(TimeS + ";" + DateS + ";" + PM10S + ";" + PM25S);
    DataLog.close();
    lcd.setCursor(0,3);
    lcd.print("Done Writing        ");
  } else {
    lcd.setCursor(0,3);
    lcd.print("Unable to Open File ");
  }
  //---------------------------------------------------//
  delay(3600000); //Delay the program for 1 hour till the next update
}

//--------------------------------------------------------------------------------//
//This function accepts 2 values, a string for the AT command and an integer for--//
//the waiting time after the function finishes.-----------------------------------//
//When called, the function commands the arduino to send the accepted string to---//
//the ESP 8266 and then waits for a response and saves it to a variable-----------//
//--------------------------------------------------------------------------------//
void sendData(String command, const int TimeDelay) {
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

//--------------------------------------------------------------------------------//
//This function, when called, will establish a connection with the Thingspeak API-//
//and then send a GET reqeuest to retreive the data(Time). Once the request is----//
//acknowledged and the module receives the data, the arduino will then proceed to-//
//filter out the unecessary data and only leave the actual needed value, in this--//
//case, the time. this filtered data will then be returned to the calling line.---//
//--------------------------------------------------------------------------------//
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

//--------------------------------------------------------------------------------//
//This function, when called, will establish a connection with the Thingspeak API-//
//and then send a GET reqeuest to retreive the data(Date). Once the request is----//
//acknowledged and the module receives the data, the arduino will then proceed to-//
//filter out the unecessary data and only leave the actual needed value, in this--//
//case, the date. this filtered data will then be returned to the calling line.---//
//--------------------------------------------------------------------------------//
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

//--------------------------------------------------------------------------------//
//The function accepts a string. This string is used to indicate what type of PM--//
//is requested, either PM10 or PM2.5.---------------------------------------------//
//This function, when called, will establish a connection with the Thingspeak API-//
//and then send a GET reqeuest to retreive the data(PM). Once the request is------//
//acknowledged and the module receives the data, the arduino will then proceed to-//
//filter out the unecessary data and only leave the actual needed value, in this--//
//case, the PM. this filtered data will then be returned to the calling line.-----//
//--------------------------------------------------------------------------------//
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

//--------------------------------------------------------------------------------//
//This function accepts 2 strings, the retreived value of PM10 and PM2.5.---------//
//When called, this function will establish a connection with the Thingspeak API--//
//and send a GET request to update the Thingspeak channel with the accepted-------//
//strings.------------------------------------------------------------------------//
//--------------------------------------------------------------------------------//
void Update(String PM10, String PM25){
  while (PM10.length() < 3) PM10 = "0" + PM10;
  while (PM25.length() < 3) PM25 = "0" + PM25;
  if (PM10 != "ERR"){
    GET = "GET /update?api_key=" + ChannelAPI + "&field1=" + PM10 + "&field2=" + PM25 + "\r\nHost:api.thingspeak.com\r\n";
    sendData("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",80\r\n", 100);
    sendData("AT+CIPSEND=" + String(GET.length()) + "\r\n", 100);
    sendData(GET, 100);
  }
}
