#include <ESP8266WiFi.h>

// Pumpengeschwindigkeit in ml/s

const char* ssid = "Cocktailautomat";
const char* password = ""; 


const String drinks[] = {
    "Pina Colada",
    "Gruene Wiese",
    "Shot"
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
    <h1 id="headline">
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
            headline.innerHTML = "Bitte Warten";
            resp = await fetch("./drink?drink="+id)
            alert(await resp.text());
            headline.innerHTML = "Getränk Auswählen";
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
  nStr.replace("NAME", drinks[id]);
  nStr.replace("ID", String(id));
  return nStr;
}





void loop() {
  while((Serial.available())) {
    Serial.read();
  }
  WiFiClient client = server.available();
  if (!client) 
  {
    return;
  }
  

  unsigned long ultimeout = millis()+250;
  while(!client.available() && (millis()<ultimeout) )
  {
    delay(1);
  }
  if(millis()>ultimeout) 
  { 
    return; 
  }
  
  // Read the first line of the request
  String sRequest = client.readStringUntil('\r');
  //Serial.println(sRequest);
  client.flush();
  
  // stop client, if request is empty
  if(sRequest=="")
  {
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
    Serial.println(sCmd);
    while(!Serial.available()) {}
    while((Serial.available())) {
      Serial.read();
    }
   
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

}
