const String WiFiname = "CAYABS";
const String Pass = "titi1234";
const String PM25API = "XW8KYL7WZBVPF9PS";
const String PM10API = "FU1OULGH590QLL88";
const String DateAPI = "QN1K6NCJWTUZ99OB";
String response, GET;
boolean errorRX;

void setup(){
  Serial.begin(9600);
  Serial1.begin(9600);
  sendData("AT+CWQAP\r\n", 100);
  sendData("AT+CWJAP=\"" + WiFiname + "\",\"" + Pass + "\"\r\n", 100);
  Serial.println(Date());
}

void loop(){
  
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
