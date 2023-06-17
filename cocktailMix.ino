
#include <ESP8266WiFi.h>


// Pumpengeschwindigkeit in ml/s
const double pumpSpeed = 30;

const char* ssid = "ESP-Accesspoint";
const char* password = "12345678";  // set to "" for open access point w/o passwortd

const int bufferSize = 4;


const uint8_t pumpPins[] = {D0, D1, D2, D3};

const uint8_t waitPin = D8;


unsigned long ulReqcount;
WiFiServer server(80);

void setup() {
   ulReqcount=0; 
  Serial.begin(9600);
  for(int i = 0; i < sizeof(pumpPins); i++) {
    pinMode(pumpPins[i], OUTPUT);
  }
  pinMode(waitPin, OUTPUT);

  delay(1);
  
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  server.begin();
}



int calcFIllTime(double t, double cupSize) {
  return (int) (1000 * t * cupSize / pumpSpeed);
}

void putInRow(double recipe[], double cupSize) {
  for(int i = 0; i < sizeof(recipe); i++) {
    if (recipe[i] > 0) {
         digitalWrite(pumpPins[i], HIGH);
         delay(calcFIllTime(recipe[i], cupSize));
         digitalWrite(pumpPins[i], LOW);
    }  
  }
}

void putIn(String recipe, double cupSize) {
  double rec[sizeof(pumpPins)];
  for(int i = 0; i < sizeof(pumpPins); i++) { 
    rec[i] = recipe.substring(i*bufferSize, (i+1)*bufferSize).toDouble();
    Serial.println(rec[i]);
  }
  digitalWrite(waitPin, HIGH);
  putInRow(rec, cupSize);
  delay(3000);
  digitalWrite(waitPin, LOW);
  
}


void loop() {
   // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) 
  {
    return;
  }
  
  // Wait until the client sends some data
  Serial.println("new client");
  unsigned long ultimeout = millis()+250;
  while(!client.available() && (millis()<ultimeout) )
  {
    delay(1);
  }
  if(millis()>ultimeout) 
  { 
    Serial.println("client connection time-out!");
    return; 
  }
  
  // Read the first line of the request
  String sRequest = client.readStringUntil('\r');
  //Serial.println(sRequest);
  client.flush();
  
  // stop client, if request is empty
  if(sRequest=="")
  {
    Serial.println("empty request! - stopping client");
    client.stop();
    return;
  }
  
  // get path; end of path is either space or ?
  // Syntax is e.g. GET /?pin=MOTOR1STOP HTTP/1.1
  String sPath="",sParam="", sCmd="";
  String sGetstart="GET ";
  int iStart,iEndSpace,iEndQuest;
  iStart = sRequest.indexOf(sGetstart);
  if (iStart>=0)
  {
    iStart+=+sGetstart.length();
    iEndSpace = sRequest.indexOf(" ",iStart);
    iEndQuest = sRequest.indexOf("?",iStart);
    
    // are there parameters?
    if(iEndSpace>0)
    {
      if(iEndQuest>0)
      {
        // there are parameters
        sPath  = sRequest.substring(iStart,iEndQuest);
        sParam = sRequest.substring(iEndQuest,iEndSpace);
      }
      else
      {
        // NO parameters
        sPath  = sRequest.substring(iStart,iEndSpace);
      }
    }
  }
  
  ///////////////////////////////////////////////////////////////////////////////
  // output parameters to serial, you may connect e.g. an Arduino and react on it
  ///////////////////////////////////////////////////////////////////////////////
  double sL = 0;
  if(sParam.length()>0)
  {
    int iEqu=sParam.indexOf("=");
    if(iEqu>=0)
    {
      sCmd = sParam.substring(iEqu+1,sParam.length());
      Serial.println(sCmd);
      Serial.println(sParam.substring(1, iEqu));
      sL = sParam.substring(1, iEqu).toDouble();
    }
  }




  ///////////////////////////
  // format the html response
  ///////////////////////////
  String sResponse,sHeader;
  if (sCmd.length() != sizeof(pumpPins)*bufferSize) {  
    sResponse="Invalid Arg";
    sHeader  = "HTTP/1.1 200 OK\r\n";
    sHeader += "Content-Length: ";
    sHeader += sResponse.length();
    sHeader += "\r\n";
    sHeader += "Content-Type: text/html\r\n";
    sHeader += "Connection: close\r\n";
    sHeader += "\r\n";
  }
  else {
    Serial.println(sL);
    putIn(sCmd, sL);
    sResponse="Done";
    sHeader  = "HTTP/1.1 200 OK\r\n";
    sHeader += "Content-Length: ";
    sHeader += sResponse.length();
    sHeader += "\r\n";
    sHeader += "Content-Type: text/html\r\n";
    sHeader += "Connection: close\r\n";
    sHeader += "\r\n";
  }




  
  // Send the response to the client
  client.print(sHeader);
  client.print(sResponse);
  
  // and stop the client
  client.stop();

  Serial.println("Client disonnected");
}
