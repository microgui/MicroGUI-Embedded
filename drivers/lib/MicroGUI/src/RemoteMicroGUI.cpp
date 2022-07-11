#include <Arduino.h>

#include "MicroGUI.h"
#include "RemoteMicroGUI.h"

#include <WiFi.h>
#include <ESPAsyncWebServer.h>

bool remoteInit = false;

bool getRemoteInit() {
    return remoteInit;
}


/* REMOTE MICROGUI */

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;

    String message = (char*)data;
    
    DynamicJsonDocument doc(100);    // Length of JSON plus some slack

    DeserializationError error = deserializeJson(doc, message);
    if (error) {
      // TODO: Render a text telling the user that the deserialization failed
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }

    JsonObject root = doc.as<JsonObject>();

    /*
    if (getRemoteInit()) {
      char buf[32];
      sprintf(buf, "{\"%s\": %i}", (const char*)root["Parent"], (int)root["Value"]);
      Serial.println(buf);
    }
    */

    mgui_set_value((const char*)root["Parent"], (int)root["Value"]);
    latest = MGUI_event((const char*)root["Event"], (const char*)root["Parent"], (int)root["Value"]);

    newEvent = true;

    
    /*
    if (strcmp((char*)data, "getValues") == 0) {
      notifyClients(getSliderValues());
    }*/
  }
}

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len)
{  
  if(type == WS_EVT_CONNECT)
  {  
    Serial.println("WebSocket client connection received");
  } 
  else if (type == WS_EVT_DATA) 
  {
    handleWebSocketMessage(arg, data, len);
  } 
  else if(type == WS_EVT_DISCONNECT)
  {
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
  remoteInit = true;
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