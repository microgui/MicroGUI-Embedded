//
//   Extension of MicroGUI Embedded library to enable remote monitoring and
//   control of an embedded display running MicroGUI as well as
//   remote GUI upload via the editor
//
//   Utilized libraries: AsyncTCP, ESPAsyncWebServer
// 
//   written by Linus Johansson 2022 for MicroGUI
//

#include <Arduino.h>

#include "MicroGUI.h"
#include "RemoteMicroGUI.h"

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <Preferences.h>
#include <DNSServer.h>

DNSServer dnsServer;
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

char AP_SSID[20];

Preferences preferences;
static volatile bool wifi_connected = false;    // Flag for indicating whether WiFi is connected or not
static volatile bool ap_on = false;   // Flag for whether WiFi is in access point mode or not, to avoid setting radio mode twice
String wifiSSID, wifiPassword;

bool remoteInit = false;    // Flag to let non-remote MicroGUI know whether remote is initialized or not

char event[100];
char parent[100];

char IPTextField[100] = "default_IP";    // Name of textfield to display IP on when connected

/* Class for handling requests to captive portal */
class CaptiveRequestHandler : public AsyncWebHandler {
public:
  CaptiveRequestHandler() {
    /* THIS IS WHERE YOU CAN PLACE HTTP CALLS, if needed in the future 
    server.on("/example", HTTP_GET, [](AsyncWebServerRequest *request) {
      AsyncResponseStream *response = request->beginResponseStream("text/html");
      response->print("<!DOCTYPE html><html><head><title>MicroGUI WiFi portal</title></head><body>");
      response->print("<h1>Hello</h1>");
      response->print("</body></html>");
      request->send(response);
    });*/
  }
  virtual ~CaptiveRequestHandler() {}

  bool canHandle(AsyncWebServerRequest *request) {
    return true;
  }

  void handleRequest(AsyncWebServerRequest *request) {
    switch(request->method()) {
      case HTTP_GET:
        if(request->hasParam("ssid") && request->hasParam("pass")) {
          AsyncWebParameter * ssid = request->getParam("ssid");
          wifiSSID = ssid->value();
          Serial.printf("[MicroGUI Remote]: ssid: %s\n", ssid->value().c_str());

          AsyncWebParameter * pass = request->getParam("pass");
          wifiPassword = pass->value();
          Serial.printf("[MicroGUI Remote]: pass: %s\n", pass->value().c_str());
        
          preferences.begin("wifi", false);
          Serial.println("[MicroGUI Remote]: Writing new ssid");
          preferences.putString("ssid", ssid->value().c_str());

          Serial.println("[MicroGUI Remote]: Writing new pass");
          preferences.putString("password", pass->value().c_str());
          preferences.end();

          AsyncResponseStream *response = request->beginResponseStream("text/html");
          response->print("<h1>OK! Trying to connect to new WiFi...</h1>");
          response->print("<h2>This portal will be closed shortly if connection is successful</h2>");
          request->send(response);
          
          Serial.println("[MicroGUI Remote]: Trying to connect to new WiFi...");
          WiFi.begin(ssid->value().c_str(), pass->value().c_str());
        }

        else {
          Serial.println("[MicroGUI Remote]: Captive portal requested!");
          AsyncResponseStream *response = request->beginResponseStream("text/html");
          response->print("<!DOCTYPE html><html><head><title>MicroGUI WiFi portal</title>");
          response->print("<meta name='viewport' content='width=device-width, initial-scale=1'>");
          response->print("<style> body { display: flex; flex-direction: column; align-items: center; text-align: center; } form { display: flex; flex-direction: column; align-items: center; font-size: 1.5rem; } </style>");
          response->print("</head><body>");
          response->print("<h1>MicroGUI WiFi portal</h1><h2>Enter your WiFi network's credentials below</h2>");
          response->print("<form action='/' method='GET'><label>SSID: </label><input id='ssid' name='ssid' length=32><label>Password: </label><input id='pass' name='pass' length=64><input type='submit' value='submit'></form>");
          response->print("</body></html>");
          request->send(response);
        }
        
      break;
    }
  }
};

/* When WiFi connects */
void wifiOnConnect() {
  Serial.println("[MicroGUI Remote]: STA Connected");
  Serial.print("[MicroGUI Remote]: STA SSID: ");
  Serial.println(WiFi.SSID());
  Serial.print("[MicroGUI Remote]: STA IPv4: ");
  Serial.println(WiFi.localIP());
  WiFi.mode(WIFI_MODE_STA);     // Close AP mode
  ap_on = false;
  mgui_hide_border();
  mgui_set_text(IPTextField, WiFi.localIP().toString().c_str());
}

/* When WiFi disconnects */
void wifiOnDisconnect() {
  Serial.println("[MicroGUI Remote]: STA Disconnected");
  mgui_show_border();
  mgui_set_text(IPTextField, "IP: N/A");
  
  if(!ap_on) {
    WiFi.mode(WIFI_MODE_APSTA);   // Start AP mode
    WiFi.softAP(AP_SSID);
    Serial.println("[MicroGUI Remote]: AP Started");
    Serial.print("[MicroGUI Remote]: AP SSID: ");
    Serial.println(AP_SSID);
    Serial.print("[MicroGUI Remote]: AP IPv4: ");
    Serial.println(WiFi.softAPIP());
    ap_on = true;
  }
  
  WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());   // Try to connect with stored credentials
}

/* WiFi events */
void WiFiEvent(WiFiEvent_t event) {
  switch(event) {
    case SYSTEM_EVENT_AP_START:
      WiFi.softAPsetHostname(AP_SSID);
      break;
    case SYSTEM_EVENT_STA_START:
      WiFi.setHostname(AP_SSID);
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
      wifiOnConnect();
      wifi_connected = true;
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      wifi_connected = false;
      wifiOnDisconnect();
      break;
    default:
      break;
  }
}

/* WebSocket message handler */
void handleWebSocketMessage(AsyncWebSocketClient * client, void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;

  static bool new_doc = false;
  // char *new_document;
  // int new_doc_index = 0;
  static String new_document;

  if(info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;

    String message = (char*)data;

    /* If document is requested, update the document and send it in chunks */
    if(strcmp((char*)data, "documentRequest") == 0) {
      Serial.print(F("[MicroGUI Remote]: Document requested by WebSocket client "));
      Serial.println(client->id());
      // Fetch all the latest values/states
      mgui_update_doc();

      // Send the document to the client that requested it
      uint16_t chunk_size = 2000;
      char temp[chunk_size + 1];
      for(int i = 0; i < strlen(document); i += chunk_size) {
        strncpy(temp, document + i, chunk_size);
        temp[chunk_size] = '\0';
        ws.text(client->id(), temp);
        delay(50);
      }
      ws.text(client->id(), "DOCUMENT SENT");

      Serial.println("[MicroGUI Remote]: Document sent!");
    }

    /* If the display is prompted to receive a new GUI document */
    else if(strcmp((char*)data, "newDocument") == 0) {
      Serial.println("[MicroGUI Remote]: Incoming new document!");
      new_doc = true;
      ws.text(client->id(), "OK");
    }

    else if(strcmp((char*)data, "NEW DOCUMENT SENT") == 0) {
      Serial.println("[MicroGUI Remote]: Entire new document sent, time to render!");
      from_persistant = false;
      mgui_render((char*)new_document.c_str());
      new_document = "";
      new_doc = false;
      ws.text(client->id(), "NEW DOCUMENT RECEIVED");
    }

    else {
      if(new_doc) {
        new_document += (char*)data;
        ws.text(client->id(), "OK");
      }
      else {
        DynamicJsonDocument doc(200);    // Length of JSON plus some slack

        DeserializationError error = deserializeJson(doc, message);
        if (error) {
          Serial.print(F("deserializeJson() failed: "));
          Serial.println(error.f_str());
          return;
        }

        JsonObject root = doc.as<JsonObject>();

        mgui_set_value((const char*)root["Parent"], (int)root["Value"], true);

        memcpy(event, (const char*)root["Event"], strlen((const char*)root["Event"]) + 1);
        memcpy(parent, (const char*)root["Parent"], strlen((const char*)root["Parent"]) + 1);

        delete latest;
        latest = new MGUI_event(event, parent, (int)root["Value"]);

        new_event = true;

        doc.clear();
      }
    }
  }
}

/* WebSocket event handler */
void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {  
  if(type == WS_EVT_CONNECT) {  
    Serial.println("[MicroGUI Remote]: WebSocket client connection received");
  } 
  else if(type == WS_EVT_DATA) {
    handleWebSocketMessage(client, arg, data, len);
  } 
  else if(type == WS_EVT_DISCONNECT) {
    Serial.println("[MicroGUI Remote]: WebSocket client disconnected");
  }
}

/* Initialize remote MicroGUI */
void mgui_remote_init() {
  snprintf(AP_SSID, 23, "MicroGUI-%04X", (uint16_t)ESP.getEfuseMac());    // Create an AP SSID from MAC address

  WiFi.onEvent(WiFiEvent);
  WiFi.mode(WIFI_MODE_STA);

  // Captive portal
  dnsServer.start(53, "*", WiFi.softAPIP());
  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);

  // WebSocket
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  server.begin();

  Serial.println("[MicroGUI Remote]: Web server initialized!");
  remoteInit = true;

  // WiFi connection
  preferences.begin("wifi", false);
  wifiSSID = preferences.getString("ssid", "none");           // SSID and Password stored in non-volatile storage
  wifiPassword = preferences.getString("password", "none");
  preferences.end();
  Serial.print("[MicroGUI Remote]: Stored SSID: ");
  Serial.println(wifiSSID);

  WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
}

/* Initialize remote MicroGUI and set any textfield to display the IP address of the screen */
void mgui_remote_init(const char * textfield) {
  memcpy(IPTextField, textfield, strlen(textfield) + 1);

  mgui_remote_init();
}

void mgui_remote_set_wifi(const char * ssid, const char * password) {
  preferences.begin("wifi", false);
  if(!preferences.getString("ssid").equals(ssid)) {
    wifiSSID = preferences.putString("ssid", ssid);
  }
  if(!preferences.getString("password").equals(password)) {
    wifiPassword = preferences.putString("password", password);
  }
  preferences.end();
}

void mgui_remote_init(const char * ssid, const char * password) {
  mgui_remote_set_wifi(ssid, password);

  mgui_remote_init();
}

void mgui_remote_init(const char * ssid, const char * password, const char * textfield) {
  mgui_remote_set_wifi(ssid, password);
  memcpy(IPTextField, textfield, strlen(textfield) + 1);

  mgui_remote_init();
}

/* Returns true if remote MicroGUI has been initialized */
bool getRemoteInit() {
  return remoteInit;
}

/* Broadcast a WebSocket message */
void mgui_send(const char * msg) {
  ws.textAll(msg);
}

/* This needs to be in the loop for captive portal to work */
void mgui_run_captive() {
  dnsServer.processNextRequest();
}

/* Returns true if MicroGUI is connected to WiFi */
bool mgui_remote_connected() {
  return wifi_connected;
}