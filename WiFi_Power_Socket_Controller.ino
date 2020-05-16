#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>

#ifndef ACCESSPOINTSSID
#define ACCESSPOINTSSID "MyIOT"
#define ACCESSPOINTPASSWORD  "12345678"
#endif

const char *accessPointSSID = ACCESSPOINTSSID;
const char *accessPointPasword = ACCESSPOINTPASSWORD;
ESP8266WebServer webServer(80);
bool accessPointStarted = false;
//bool accessPointSetting = true;
String wiFiSSID = "";
String wiFiPassword = "";
IPAddress wiFiLocalIP(0,0,0,0);
bool wiFiConnected = false;
bool prevSwitchState = false;
int socketState = HIGH;
int switchState = 0;

void setup() {
  Serial.begin(9600);
  pinMode(1, OUTPUT);
  digitalWrite(1, LOW);
  pinMode(0, OUTPUT);
  setSocketState(HIGH);
  //WiFi.softAPdisconnect(true);
  startAccessPoint();

  
  for (int i=0;i<5;i++) {
    Serial.println(i);
    delay(1000);     
  }
  webServer.on("/", accessPointHandler);
  webServer.begin();
  
  //switchState = digitalRead(2);
  //if (switchState == 0)
  //  Serial.println("0 IS LOW");
  //else
  //  Serial.println("0 IS HIGH");

  startAccessPoint();
  connectToWiFi();
  if (WiFi.status() != WL_CONNECTED)
    startAccessPoint();
  
  //int switchState = 1;
  //if (switchState == 1) {
  //  if (!accessPointStarted)
  //    startAccessPoint();  
  //} else {
  //  if (accessPointStarted)
  //    stopAccessPoint();  
  //}
}

void loop() {
  //if (!wiFiConnected)
  //  connectToWiFi();
  webServer.handleClient();

  //if (switchState != prevSwitchState) {
  //  if (switchState == 0) 
  //    Serial.println("deger : 0");
  //  else
  //    Serial.println("deger : 1");
  //  prevSwitchState = switchState;    
  //}
}

void startAccessPoint() {
  Serial.println("Starting Access Point...");
  if (WiFi.softAP(accessPointSSID, accessPointPasword))
    Serial.println("Access Point Started");
  else
    Serial.println("Error - Access Point Server Could Not Started");
  accessPointStarted = true;
  Serial.println("Access Point Server Started");
}

void stopAccessPoint() {
  Serial.println("Stopping Access Point");
  WiFi.softAPdisconnect(false);
  accessPointStarted = false;
}

void setSocketState(int state){
  digitalWrite(0,state);
  socketState = state;
}

void connectToWiFi(){
  wiFiConnected = false;
  Serial.print("Connecting to WiFi...");
  //WiFi.mode(WIFI_STA);

  WiFi.begin(wiFiSSID, wiFiPassword);

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    i++;
    if (i>100)
      break;
  }

  if (WiFi.status() == WL_CONNECTED) {
    wiFiConnected = true;
    wiFiLocalIP = WiFi.localIP();
    Serial.println("");
    Serial.println("Connected to " + wiFiSSID + " IP : " + ip2Str(wiFiLocalIP));
  } else {
    Serial.println("Could not connect to : " + wiFiSSID);
  }
}
String ip2Str(IPAddress ip){
  String s="";
  for (int i=0; i<4; i++) {
    s += i  ? "." + String(ip[i]) : String(ip[i]);
  }
  return s;
}

void accessPointHandler() {

  if (!webServer.authenticate("mysocket", "candan")) {
    return webServer.requestAuthentication();
  }
  
  String requestURI = webServer.uri();
  String argOp = "";
  String argNetworkName = "";
  String argNetworkPassword = "";
  String argPin2Value = "0";

  
  for (int i = 0; i < webServer.args(); i++) {    
    if (webServer.argName(i) == "networkName")
      argNetworkName = webServer.arg(i);
    if (webServer.argName(i) == "networkPassword")
      argNetworkPassword = webServer.arg(i);
    if (webServer.argName(i) == "pin2Value")
      argPin2Value = webServer.arg(i);
    if (webServer.argName(i) == "op")
      argOp = webServer.arg(i);
  }

  for (int i=0; i<webServer.headers();i++){
    Serial.println(webServer.headerName(i) + ":" + webServer.header(i));
  }
  if (argOp == "HIGH") { 
      setSocketState(HIGH);  
  } else {
    if (argOp == "LOW") {
      setSocketState(LOW);
    } else {
      if (argOp == "WiFi") {
        wiFiSSID = argNetworkName;
        wiFiPassword = argNetworkPassword;
        connectToWiFi();      
      } else {
        if (argOp == "StartServer") {
          startAccessPoint();        
        } else {
          if (argOp == "StopServer") {
            stopAccessPoint();
          } else {
            if (argOp == "SetPin2") {
              if (argPin2Value == "1")
                digitalWrite(1,HIGH);
              else
                digitalWrite(1,LOW);
            }
          }
        }
      }
    }    
  }

  String stateText = "<a href=\"./?op=HIGH\"><img style=\"width:64px\" src=\"data:image/png;base64,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\"/></a>";
  if (socketState == HIGH)
    stateText = "<a href=\"./?op=LOW\"><img style=\"width:64p\" src=\"data:image/png;base64,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\"/></a> ";
  String wiFiStatusText = "<p>Status : DISCONNECTED</p>";
  if (wiFiConnected)
    wiFiStatusText = "<p>Status : CONNECTED</p><p>IP : <a href=\"http://" + ip2Str(WiFi.localIP()) + "\" target=\"_self\" style=\"text-decoration:none;color:#007bff\">" + ip2Str(WiFi.localIP()) + "</a></p>";
  String accessPointStatusText = "<p>Status : STOPPED</p>"
    "<button style=\"background-color:#007bff;color:white;padding:5px 20px;font-size:14pt;margin-top:10px\" onclick=\"startServerClick()\">Start</button>";
  if (accessPointStarted) {
    accessPointStatusText = "<p>Status : STARTED</p>"
      "<p>IP : <a href=\"http://192.168.4.1\" target=\"_self\" style=\"text-decoration:none;color:#007bff\">192.168.4.1</a></p>"
      "<button style=\"background-color:#007bff;color:white;padding:5px 20px;font-size:14pt;margin-top:10px\" onclick=\"stopServerClick()\">Stop</button>";  
  }
  
  String htmlText = "<!DOCTYPE html>"
      "<html>"
      "<head>"
      "<meta charset=\"UTF-8\">"
      "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
      "<style>"
      "label {font-size:10pt;}"
      "input {display:block;width:100%;background-color:#cccccc;padding:5px;margin-top:3px;border:1px solid #555555}"
      "</style>"
      "</head>"
      "<body style=\"background-color:#08162b;color:white;font-family:Verdana\">"
      "<div style=\"width:300px;margin:auto;margin-top:50px;margin-bottom:30px;text-align:center\">"
      "<p style=\"margin-bottom:50px;font-size:16pt;font-weight:bold\">My Smart Socket</p>" + stateText + 
      "</div>"
      "<hr/>"
      "<div style=\"width:300px;margin:auto;margin-top:50px\">"
      "<p style=\"font-weight:bold\">WiFi Info</p>"
      "<div style=\"margin-left:20px;margin-bottom:40px\">" + wiFiStatusText +       
      "</div> "
      "<p style=\"font-weight:bold\">WiFi Settings</p>"
      "<div style=\"margin-left:20px;margin-bottom:40px\">"
      "<label for=\"networkName\">Network Name</label>"
      "<input type=\"text\" id=\"networkName\" value=\"" + wiFiSSID + "\"/>"
      "<label for=\"networkPassword\">Password</label>"
      "<input type=\"password\" id=\"networkPassword\" value=\"" + wiFiPassword + "\"/>"
      "<button style=\"background-color:#007bff;color:white;padding:5px 20px;font-size:14pt;margin-top:10px\" onclick=\"wiFiSaveClick()\">Save And Connect</button>"
      "</div>"
      "<p style=\"font-weight:bold\">Access Point Info</p>"
      "<div style=\"margin-left:20px;margin-bottom:40px\">" + accessPointStatusText +
      "</div>"
      "</div>"
      "<script type=\"text/javascript\">"
      "function wiFiSaveClick() {"
      "window.location = './?op=WiFi&networkName=' + document.getElementById('networkName').value + '&networkPassword=' + document.getElementById('networkPassword').value;"
      "}"
      "function stopServerClick() {"
      "window.location = './?op=StopServer';"
      "}"
      "function startServerClick() {"
      "window.location = './?op=StartServer';"
      "}"
      "</script>"
      "</body>"
      "</html>"
      ;
      
  webServer.send(200, "text/html", htmlText);
}
