const String WiFiname = "CAYABS";
const String Pass = "titi1234";
const String Channel = "SWXFN4R4FM0ON266";
String response, GET;
boolean errorRX;

void setup(){
  Serial.begin(9600);
  Serial1.begin(9600);
  sendData("AT+CWQAP\r\n", 100);
  sendData("AT+CWJAP=\"" + WiFiname + "\",\"" + Pass + "\"\r\n", 100);
  Update("100");
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

void Update(String Data){
  GET = "GET /update?api_key=" + Channel + "&field1=" + Data + "\r\nHost:api.thingspeak.com\r\n";
  sendData("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",80\r\n", 100);
  sendData("AT+CIPSEND=" + String(GET.length()) + "\r\n", 100);
  sendData(GET, 100);
}
