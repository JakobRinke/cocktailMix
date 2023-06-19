#include <ESP8266WiFi.h>


// Pumpengeschwindigkeit in ml/s
const double pumpSpeed = 30;

const char* ssid = "ESP-Accesspoint";
const char* password = "12345678";  // set to "" for open access point w/o passwortd

const uint8_t pumpPins[] = {D0, D1, D2, D3};
const uint8_t waitPin = D8;

struct Drink
{
  String Name;
  // in ml
  double cupSize;
  double ratio[sizeof(pumpPins)];
};

const Drink drinks[] = {
  {
    "Pina Colada",
    400.0,
    {0.2, 0.5, 0.3, 0.0}
  },
  {
    "Gruene Wiese",
    400.0,
    {0.0, 0.5, 0.0, 0.5}
  },
  {
    "Shot",
    40.0,
    {1.0, 0.0, 0.0, 0.0}
  }
  
};



String WebsitePrefab = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Document</title>
    <style>
        * {
            padding: 0;
            text-align: center;
        }
        body {
            background-color: rgb(255, 249, 240);
        }
        h1 {
            font-size: 250%;
        }
        button {
            width: 90%;
            font-size: 140%;
            height: 6vh;
            margin-bottom: 5vh;
            background-color: rgb(255, 194, 150);
            color: #3d3d3d;
        }
    </style>
</head>
<body>
    <br><br>
    <h1>
        Getränk Auswählen
    </h1>
    <br><br>

    <div class="drinkWrapper">
        <ul class="drinks">
           DRINKS
        </ul>
    </div>

    <script> 
        async function getDrink(id) {
            const buttons = document.getElementsByName("button");
            buttons.forEach(b=>{
                b.style.pointerEvents = "none"
            });
            alert("Drink Wird bestellt, bitte warten");
            resp = await fetch("./drink?drink="+id)
            alert(await resp.text());
            buttons.forEach(b=>{
                b.style.pointerEvents = "auto"
            });
        }
    </script>
</body>
</html>
)=====";




const String drinkPrefab = R"=====( 
  <li class="drinkItem"> 
      <button onclick="getDrink(ID)">
          NAME
      </button>
   </li>
)=====";

const int bufferSize = 4;



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
  String drinkHTML = "";
  for (int i = 0; i < sizeof(drinks) / sizeof(drinks[0]);  i++) {
    drinkHTML += getDrinkHTML(i);
  }
  WebsitePrefab.replace("DRINKS", drinkHTML);
}

String getDrinkHTML(int id) {
  String nStr = drinkPrefab.substring(0, drinkPrefab.length());
  nStr.replace("NAME", drinks[id].Name);
  nStr.replace("ID", String(id));
  return nStr;
}



int calcFillTime(double t, double cupSize) {
  return (int) (1000 * t * cupSize / pumpSpeed);
}

void putInRow(double recipe[], double cupSize) {
  for(int i = 0; i < sizeof(recipe); i++) {
    if (recipe[i] > 0) {
         digitalWrite(pumpPins[i], HIGH);
         delay(calcFillTime(recipe[i], cupSize));
         digitalWrite(pumpPins[i], LOW);
    }  
  }
}

void putIn(Drink drink) {
  digitalWrite(waitPin, HIGH);
  Serial.println(drink.Name);
  putInRow(drink.ratio , drink.cupSize);
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
  if(sParam.length()>0)
  {
    int iEqu=sParam.indexOf("=");
    if(iEqu>=0)
    {
      sCmd = sParam.substring(iEqu+1,sParam.length());
      Serial.println(sCmd);
      Serial.println(sParam.substring(1, iEqu));
    }
  }




  ///////////////////////////
  // format the html response
  ///////////////////////////
  String sResponse,sHeader;
  if (sPath=="/") {  
    sResponse=WebsitePrefab;
    sHeader  = "HTTP/1.1 200 OK\r\n";
    sHeader += "Content-Length: ";
    sHeader += sResponse.length();
    sHeader += "\r\n";
    sHeader += "Content-Type: text/html\r\n";
    sHeader += "Connection: close\r\n";
    sHeader += "\r\n";
  }
  else {
    putIn(drinks[sCmd.toInt()]);
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
