#include <Arduino.h>

#include "MicroGUI.h"
#include "RemoteMicroGUI.h"

#include <WiFi.h>
#include <ESPAsyncWebServer.h>


/* REMOTE MICROGUI */

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  
  if(type == WS_EVT_CONNECT){
  
    Serial.println("Websocket client connection received");
     
  } else if(type == WS_EVT_DISCONNECT){
 
    Serial.println("Client disconnected");
  
  }
}

/* Initialize remote MicroGUI */
void mgui_remote_init(const char * ssid, const char * password) {
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
  	
  server.begin();
  
  Serial.println(WiFi.localIP());
}

/* Initialize remote MicroGUI and set any textfield to display the IP address of the screen */
void mgui_remote_init(const char * ssid, const char * password, const char * textfield) {
  mgui_remote_init(ssid, password);
  mgui_set_text(textfield, WiFi.localIP().toString().c_str());
}

void mgui_send(const char * msg) {
  ws.textAll(msg);
}


// I need a function that will send the document first and then the states of all objects