//
//   Library for making MicroGUI functional on embedded displays
//
//   Utilized libraries: LovyanGFX, LVGL, ArduinoJSON, LinkedList
// 
//   written by Linus Johansson 2022 for MicroGUI
//

#include <Arduino.h>

#include "MicroGUI.h"
#include "RemoteMicroGUI.h"

#include <LinkedList.h>

#define LGFX_AUTODETECT // Autodetect board
#define LGFX_USE_V1

#include <LovyanGFX.hpp>

#include <lvgl.h>
#include "lv_conf.h"

#include <ArduinoJson.h>
#include <ArduinoJson.hpp>

#include <string.h>


static LGFX lcd; // declare display variable

/* Setup screen resolution for LVGL */
static uint16_t screenWidth;
static uint16_t screenHeight;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[4800];

/* Variables/objects for MicroGUI events */
static MGUI_event * default_event = new MGUI_event("Default", "None", 0);
static MGUI_object * none_object = new MGUI_object(new lv_obj_t, "None", "None", "None");
MGUI_event * latest;
bool new_event = false;

/* Linked lists for storing object pointers, for later access */
LinkedList<MGUI_object*> buttons;
LinkedList<MGUI_object*> switches;
LinkedList<MGUI_object*> sliders;
LinkedList<MGUI_object*> checkboxes;
LinkedList<MGUI_object*> textfields;
LinkedList<MGUI_object*> dividers;
LinkedList<MGUI_object*> radiobuttons;
LinkedList<MGUI_object*> progressbars;

/* For storing the initial json document internally */
char document[20000];

bool from_persistant = false;

/* Border for indicating disconnected WiFI, may be used for other applications as well */
lv_obj_t * border;
bool border_vis = false;

/* LVGL styling */
uint8_t font_sizes[] = {14, 18, 24, 32, 40};
const lv_font_t *font_list[] = {&lv_font_montserrat_14, &lv_font_montserrat_18, &lv_font_montserrat_24, &lv_font_montserrat_32, &lv_font_montserrat_40};

/* Radiobuttons styling */
static lv_style_t style_radio;
static lv_style_t style_radio_chk;

/* MicroGUI object class functions */

MGUI_object::MGUI_object(lv_obj_t * obj, const char * obj_type, const char * obj_name, const char * obj_event) {
  this->object = obj;
  memcpy(this->type, obj_type, strlen(obj_type) + 1);
  memcpy(this->parent_id, obj_name, strlen(obj_name) + 1);
  memcpy(this->event_id, obj_event, strlen(obj_name) + 1);
}

MGUI_object::MGUI_object() {
}

// Links an LVGL object with this MicroGUI object
void MGUI_object::setObject(lv_obj_t * obj) {
  this->object = obj;
}

// Returns linked LVGL object
lv_obj_t * MGUI_object::getObject() {
  return this->object;
}

// Returns object type, e.g. Button or Textfield
char* MGUI_object::getType() {
  return this->type;
}

// Returns object's event id
char* MGUI_object::getEvent() {
  return this->event_id;
}

// Returns object's parent id, i.e. the object's name
char* MGUI_object::getParent() {
  return this->parent_id;
}

/* MicroGUI event class functions */

MGUI_event::MGUI_event(const char * event, const char * parent, int val) {
  memcpy(this->event_id, event, strnlen(event, 100) + 1);
  memcpy(this->parent_id, parent, strnlen(parent, 100) + 1);
  this->value = val;
}

MGUI_event::MGUI_event() {
  memcpy(this->event_id, "None", strlen("None") + 1);
  memcpy(this->parent_id, "None", strlen("None") + 1);
  this->value = 0;
}

// Returns the id of the triggered event
char* MGUI_event::getEvent() {
  return this->event_id;
}

// Returns the parent id of the object which triggered the event
char* MGUI_event::getParent() {
  return this->parent_id;
}

// Returns a value associated with the triggered event
int MGUI_event::getValue() {
  return this->value;
}


/* MicroGUI function prototypes */
void mgui_parse(char json[]);

void mgui_render_canvas(JsonPair kv, JsonObject root);
void mgui_render_button(JsonPair kv, JsonObject root);
void mgui_render_switch(JsonPair kv, JsonObject root);
void mgui_render_slider(JsonPair kv, JsonObject root);
void mgui_render_checkbox(JsonPair kv, JsonObject root);
void mgui_render_textfield(JsonPair kv, JsonObject root);
void mgui_render_divider(JsonPair kv, JsonObject root);
void mgui_render_radiobuttons(JsonPair kv, JsonObject root);
void mgui_render_progressbar(JsonPair kv, JsonObject root);


/* Display function prototypes */
void display_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
void touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data);


/* Parse json for important data in the beginning */
void mgui_parse(char json[]) {
  memcpy(document, json, strlen(json));   // Store a copy of the GUI document

  DynamicJsonDocument doc(sizeof(document));

  DeserializationError error = deserializeJson(doc, (const char*)document);
  if(error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  JsonObject root = doc.as<JsonObject>();

  // Important data
  screenWidth = root["ROOT"]["props"]["width"];
  screenHeight = root["ROOT"]["props"]["height"];

  doc.clear();
}

/* Initialize display for use with MicroGUI and render either a stored or the default GUI */ 
void mgui_init() {
  preferences.begin("gui", false);
  String temp_doc = preferences.getString("main", "none");    // Get GUI stored in flash
  preferences.end();
  // Set that as the main GUI document if it exists, otherwise set the default GUI as main document
  if(!temp_doc.equals("none")) {
    from_persistant = true;
    mgui_init((char*)(temp_doc.c_str()));
    return;
  }

  // This is the default GUI
  // Before replacing this, it is important that you change the name of the IP textfield to default_IP and that you change that corresponding textfield 
  // in ROOT["nodes"] to default_IP as well. If you do not do this, the remote monitoring will break.
  char json[] = "{\"ROOT\":{\"type\":{\"resolvedName\":\"CanvasArea\"},\"isCanvas\":true,\"props\":{\"id\":\"canvasElement\",\"width\":480,\"height\":320,\"persistant\":false,\"background\":{\"r\":0,\"g\":0,\"b\":0,\"a\":1}},\"displayName\":\"Canvas\",\"custom\":{},\"hidden\":false,\"nodes\":[\"Textfield_1\",\"Textfield_2\",\"Textfield_3\",\"Switch_1\",\"Checkbox_1\",\"Button_1\",\"Slider_1\",\"default_IP\"],\"linkedNodes\":{}},\"Textfield_1\":{\"type\":{\"resolvedName\":\"Textfield\"},\"isCanvas\":false,\"props\":{\"text\":\"This default screen is shown because no other GUI was found\",\"fontSize\":14,\"textAlign\":\"left\",\"fontWeight\":500,\"width\":40,\"height\":30,\"color\":{\"r\":255,\"g\":255,\"b\":255,\"a\":1},\"pageX\":21,\"pageY\":125},\"displayName\":\"Textfield\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}},\"Textfield_2\":{\"type\":{\"resolvedName\":\"Textfield\"},\"isCanvas\":false,\"props\":{\"text\":\"Have a look at the tutorials to get started with your own GUI\",\"fontSize\":14,\"textAlign\":\"left\",\"fontWeight\":500,\"width\":40,\"height\":30,\"color\":{\"r\":255,\"g\":255,\"b\":255,\"a\":1},\"pageX\":22,\"pageY\":153},\"displayName\":\"Textfield\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}},\"Textfield_3\":{\"type\":{\"resolvedName\":\"Textfield\"},\"isCanvas\":false,\"props\":{\"text\":\"Welcome to MicroGUI v1.1\",\"fontSize\":32,\"textAlign\":\"left\",\"fontWeight\":500,\"width\":40,\"height\":30,\"color\":{\"r\":248,\"g\":231,\"b\":28,\"a\":1},\"pageX\":31,\"pageY\":56},\"displayName\":\"Textfield\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}},\"Switch_1\":{\"type\":{\"resolvedName\":\"Switch\"},\"isCanvas\":false,\"props\":{\"state\":true,\"size\":\"small\",\"color\":{\"r\":126,\"g\":211,\"b\":33,\"a\":1},\"event\":\"\",\"pageX\":112,\"pageY\":209},\"displayName\":\"Switch\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}},\"Checkbox_1\":{\"type\":{\"resolvedName\":\"Checkbox\"},\"isCanvas\":false,\"props\":{\"state\":true,\"size\":\"medium\",\"color\":{\"r\":208,\"g\":2,\"b\":27,\"a\":1},\"event\":\"\",\"pageX\":334,\"pageY\":208},\"displayName\":\"Checkbox\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}},\"Button_1\":{\"type\":{\"resolvedName\":\"Button\"},\"isCanvas\":false,\"props\":{\"text\":\"Hello\",\"size\":\"small\",\"variant\":\"contained\",\"background\":{\"r\":63,\"g\":81,\"b\":181,\"a\":1},\"color\":{\"r\":255,\"g\":255,\"b\":255,\"a\":1},\"event\":\"\",\"pageX\":206,\"pageY\":206},\"displayName\":\"Button\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}},\"Slider_1\":{\"type\":{\"resolvedName\":\"Slider\"},\"isCanvas\":false,\"props\":{\"size\":\"small\",\"width\":300,\"value\":50,\"min\":0,\"max\":100,\"color\":{\"r\":80,\"g\":227,\"b\":194,\"a\":1},\"valueLabelDisplay\":\"auto\",\"event\":\"\",\"pageX\":88,\"pageY\":270},\"displayName\":\"Slider\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}},\"default_IP\":{\"type\":{\"resolvedName\":\"Textfield\"},\"isCanvas\":false,\"props\":{\"text\":\"\",\"fontSize\":14,\"textAlign\":\"left\",\"fontWeight\":500,\"width\":40,\"height\":30,\"color\":{\"r\":255,\"g\":255,\"b\":255,\"a\":1},\"pageX\":14,\"pageY\":12},\"displayName\":\"Textfield\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}}}";
  mgui_init(json);
}

/* Initialize display for use with MicroGUI and render GUI, without specifying display orientation */
void mgui_init(char json[]) {
  if(!screenWidth) mgui_parse(json);

  if(screenWidth < screenHeight) {
    mgui_init(json, MGUI_PORTRAIT);
  } else {
    mgui_init(json, MGUI_LANDSCAPE);
  }
}

/* Initialize display for use with MicroGUI and render GUI */
void mgui_init(char json[], int rotation) {
  if(!screenWidth) mgui_parse(json);

  lcd.init();   // Initialize LovyanGFX
  lv_init();    // Initialize lvgl

  // Setting display rotation
  if(rotation % 2 == 1 && screenWidth < screenHeight) {
    lcd.setRotation(MGUI_PORTRAIT);
    Serial.println("[MicroGUI]: Aspect ratio of GUI suggests that it was created for portrait mode, landscape mode not possible");
  } else if(rotation % 2 == 0 && screenWidth > screenHeight) {
    lcd.setRotation(MGUI_LANDSCAPE);
    Serial.println("[MicroGUI]: Aspect ratio of GUI suggests that it was created for landscape mode, portrait mode not possible");
  } else {
    lcd.setRotation(rotation % 4);
  }

  /* LVGL : Setting up buffer to use for display */
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * 10);

  /* LVGL : Setup & Initialize the display device driver */
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = display_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  /* LVGL : Setup & Initialize the input device driver */
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = touchpad_read;
  lv_indev_drv_register(&indev_drv);

  // Render main document
  mgui_render(document);
}

/* Let the display do its' work, returns a MicroGUI event */
MGUI_event * mgui_run() {
  // LVGL tick function
  lv_timer_handler();

  // Run DNS for captive portal if remote initialized
  if(getRemoteInit()) {
    mgui_run_captive();
  }

  if(new_event) {      // Only return new events
    new_event = false;

    #if 1
    Serial.println("-----------------------------------------");
    Serial.print(F("[MicroGUI Event] Parent: "));
    Serial.println(latest->getParent());
    Serial.print(F("[MicroGUI Event] Type: "));
    Serial.println(latest->getEvent());
    Serial.print(F("[MicroGUI Event] Value: "));
    Serial.println(latest->getValue());
    Serial.println("-----------------------------------------");
    #endif

    return latest;
  }
  return default_event;
}

/* Callback function for all widget actions on the display, turn them into MGUI events */
static void widget_cb(lv_event_t * e) {
  uint32_t * active_id = (uint32_t*)lv_event_get_user_data(e);
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t * object = lv_event_get_target(e);
  lv_obj_t * parent = lv_event_get_current_target(e);
  lv_obj_t * child = lv_obj_get_child(parent, *active_id);

  if(object == parent) return;  // If container is clicked, do nothing
  
  delete latest;

  int value;
  bool broadcast_event = true;        // Flag for choosing what events to broadcast, default is broadcast all

  // "Convert" LVGL event to MicroGUI event
  if(code == LV_EVENT_CLICKED) {     // If button short-click, more events available from LVGL, implement later?
    if(lv_obj_check_type(object, &lv_btn_class)) {
      value = 1;
      latest = new MGUI_event(((MGUI_object*)lv_obj_get_user_data(object))->getEvent(), 
                            ((MGUI_object*)lv_obj_get_user_data(object))->getParent(), 
                            value);
      broadcast_event = false;              // Sending button events is useless since there is no visual change on this event, however may be useful for something else??? Disabled for now..
    }
    else if(lv_obj_check_type(object, &lv_checkbox_class)) { // If radiobutton
      lv_obj_clear_state(child, LV_STATE_CHECKED);   // Uncheck the previous radio button
      lv_obj_add_state(object, LV_STATE_CHECKED);    // Check the current radio button
      
      value = (int)lv_obj_get_state(object);
      
      latest = new MGUI_event(((MGUI_object*)lv_obj_get_user_data(object))->getEvent(), 
                              ((MGUI_object*)lv_obj_get_user_data(object))->getParent(), 
                              value);
      
      *active_id = lv_obj_get_index(object);
    }
  }
  else if(code == LV_EVENT_VALUE_CHANGED) {
    if(lv_obj_check_type(object, &lv_slider_class)) {     // If slider
      value = lv_slider_get_value(object);
      latest = new MGUI_event(((MGUI_object*)lv_obj_get_user_data(object))->getEvent(), 
                              ((MGUI_object*)lv_obj_get_user_data(object))->getParent(), 
                              value);
      broadcast_event = false;            // Do not try to broadcast all slider value changes to avoid overloading the websocket connection. Only send final slider values, i.e. on slider release
    } 
    else if(lv_obj_check_type(object, &lv_switch_class) || lv_obj_check_type(object, &lv_checkbox_class)) {    // If switch or checkbox
      value = (int)lv_obj_get_state(object) & LV_STATE_CHECKED ? 1 : 0;
      latest = new MGUI_event(((MGUI_object*)lv_obj_get_user_data(object))->getEvent(), 
                              ((MGUI_object*)lv_obj_get_user_data(object))->getParent(), 
                              value);
    }
  }
  else if(code == LV_EVENT_RELEASED) {
    if(lv_obj_check_type(object, &lv_slider_class)) {     // If slider
      value = lv_slider_get_value(object);
      latest = new MGUI_event(((MGUI_object*)lv_obj_get_user_data(object))->getEvent(), 
                              ((MGUI_object*)lv_obj_get_user_data(object))->getParent(), 
                              value);
    } 
  }
  new_event = true;

  // Broadcast change if remote is initialized
  if(getRemoteInit()) {
    if(broadcast_event) {
      char buf[100];
      sprintf(buf, "{\"%s\": %i}", ((MGUI_object*)lv_obj_get_user_data(object))->getParent(), value);
      mgui_send(buf);
    }
  }
}

/* Clear MicroGUI objects lists */
void mgui_clear_lists() {
  // Deletes objects from memory
  for(int i = 0; i < textfields.size(); i++) {
    delete textfields.get(i);
  }
  for(int i = 0; i < buttons.size(); i++) {
    delete buttons.get(i);
  }
  for(int i = 0; i < switches.size(); i++) {
    delete switches.get(i);
  }
  for(int i = 0; i < checkboxes.size(); i++) {
    delete checkboxes.get(i);
  }
  for(int i = 0; i < sliders.size(); i++) {
    delete sliders.get(i);
  }
  for(int i = 0; i < dividers.size(); i++) {
    delete dividers.get(i);
  }
  for(int i = 0; i < radiobuttons.size(); i++) {
    delete radiobuttons.get(i);
  }
  for(int i = 0; i < progressbars.size(); i++) {
    delete progressbars.get(i);
  }

  // Clears object references from lists
  buttons.clear();
  switches.clear();
  sliders.clear();
  checkboxes.clear();
  textfields.clear();
  dividers.clear();
  radiobuttons.clear();
  progressbars.clear();
}

/* Render MicroGUI from json */
void mgui_render(char json[]) {
  DynamicJsonDocument doc(sizeof(document));

  DeserializationError error = deserializeJson(doc, (const char*)json);
  if(error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  mgui_clear_lists();

  JsonObject root = doc.as<JsonObject>();

  // List all keys from the document
  for(JsonPair kv : root) {
    const char * type = (const char*)root[kv.key()]["type"]["resolvedName"];    // Find object type

    // If object is the Canvas, i.e background
    if(mgui_compare(type, "CanvasArea")) {
      mgui_render_canvas(kv, root);
    }
    // If object is a button
    else if(mgui_compare(type, "Button")) {
      mgui_render_button(kv, root);
    }
    // If object is a switch
    else if(mgui_compare(type, "Switch")) {
      mgui_render_switch(kv, root);
    }
    // If object is a slider
    else if(mgui_compare(type, "Slider")) {
      mgui_render_slider(kv, root);
    }
    // If object is a checkbox
    else if(mgui_compare(type, "Checkbox")) {
      mgui_render_checkbox(kv, root);
    }
    // If object is a textfield
    else if(mgui_compare(type, "Textfield")) {
      mgui_render_textfield(kv, root);
    }
    // If object is a divider
    else if(mgui_compare(type, "Divider")) {
      mgui_render_divider(kv, root);
    }
    // If object is a Radiobutton
    else if(mgui_compare(type, "Radiobutton")) {
      mgui_render_radiobuttons(kv, root);
    }
    // If object is a Progressbar
    else if(mgui_compare(type, "Progressbar")) {
      mgui_render_progressbar(kv, root);
    }
  }

  Serial.println("[MicroGUI]: GUI successfully rendered!");
  
  // Try to store GUI in flash if canvas (ROOT) prop "persistant" is true
  if(root["ROOT"]["props"]["persistant"] && !from_persistant) {
    preferences.begin("gui", false);
    preferences.clear();
    
    uint8_t status = preferences.putString("main", json);
    if(!status) {
      // If unsuccessful
      Serial.println("[MicroGUI]: GUI was too large to be stored in persistant memory.");
    } else {
      // If successful
      Serial.println("[MicroGUI]: Stored new GUI in persistant memory!");
      memcpy(document, json, strlen(json));
    }

    delay(50);
    preferences.end();
  }
}

/* Search for an object in a list with corresponding name, linear search */
MGUI_object * mgui_find_object(const char * obj_name, LinkedList<MGUI_object*> *list) {
  for(int i = 0; i < list->size(); i++) {
    if(strcmp(obj_name, list->get(i)->getParent()) == 0) {
      return list->get(i);
    }
  }
  return none_object;
}

/* Returns true if strings are equal and false if not, for strings less than 100 characters */
bool mgui_compare(const char * string1, const char * string2) {
  return !(strncmp(string1, string2, 100));
}

/* Set the value of a MicroGUI object */
void mgui_set_value(const char * obj_name, int value) {
  mgui_set_value(obj_name, value, true);
}

void mgui_set_value(const char * obj_name, int value, bool send) {
  // Find MicroGUI object
  MGUI_object * object = mgui_find_object(obj_name, &textfields);
  if(strcmp(object->getType(), "None") == 0) {
    object = mgui_find_object(obj_name, &sliders);
  }
  if(strcmp(object->getType(), "None") == 0) {
    object = mgui_find_object(obj_name, &switches);
  }
  if(strcmp(object->getType(), "None") == 0) {
    object = mgui_find_object(obj_name, &checkboxes);
  }

  // Change its' value according to type
  if(strcmp(object->getType(), "Textfield") == 0) {
    String val = String(value);
    char buf[16];
    val.toCharArray(buf, 16);
    mgui_set_text(obj_name, buf);
    return;
  } 
  else if(strcmp(object->getType(), "Slider") == 0) {
    lv_slider_set_value(object->getObject(), value, LV_ANIM_OFF);
  } 
  else if(strcmp(object->getType(), "Switch") == 0) {
    if(value) lv_obj_add_state(object->getObject(), LV_STATE_CHECKED);
    else lv_obj_clear_state(object->getObject(), LV_STATE_CHECKED);
  } 
  else if(strcmp(object->getType(), "Checkbox") == 0) {
    if(value) lv_obj_add_state(object->getObject(), LV_STATE_CHECKED);
    else lv_obj_clear_state(object->getObject(), LV_STATE_CHECKED);
  } 
  else {
    Serial.print(F("[MicroGUI]: Could not change the value of "));
    Serial.println(obj_name);
    return;
  }

  // Broadcast value change to connected WebSocket clients
  if(getRemoteInit() && send) {
    char buf[100];
    sprintf(buf, "{\"%s\": %i}", obj_name, value);
    //Serial.println(buf);
    mgui_send(buf);
  }
}

/* Set the text of a MicroGUI object */
void mgui_set_text(const char * obj_name, const char * text) {
  mgui_set_text(obj_name, text, true);
}

void mgui_set_text(const char * obj_name, const char * text, bool send) {
  // Find MicroGUI object, which has text
  MGUI_object * object = mgui_find_object(obj_name, &textfields);
  if(strcmp(object->getType(), "None") == 0) {
    object = mgui_find_object(obj_name, &buttons);
  }

  // Change it's text according to type
  if(strcmp(object->getType(), "Textfield") == 0) {
    lv_label_set_text(object->getObject(), text);
    
    // Broadcast value change to connected WebSocket clients
    if(getRemoteInit() && send) {
      char buf[100];
      sprintf(buf, "{\"%s\": \"%s\", \"type\": \"Textfield\"}", obj_name, text);
      //Serial.println(buf);
      mgui_send(buf);
    }
  }
  else if(strcmp(object->getType(), "Button") == 0) {
    Serial.println("[MicroGUI]: Updating text of buttons is not yet supported");
    return;
  } 
  else {
    Serial.print(F("[MicroGUI]: Could not change the text of "));
    Serial.println(obj_name);
    return;
  }
}

/* Get integer value of object */
int mgui_get_value(const char * obj_name) {
  // Search for object in relevant lists
  MGUI_object * object = mgui_find_object(obj_name, &sliders);
  // If not found ( object->getType() == "None" ) , try other list
  if(strcmp(object->getType(), "None") == 0) {
    object = mgui_find_object(obj_name, &switches);
  }
  if(strcmp(object->getType(), "None") == 0) {
    object = mgui_find_object(obj_name, &checkboxes);
  }

  // Getting value from LVGL object types
  if(strcmp(object->getType(), "Slider") == 0) {
    return lv_slider_get_value(object->getObject());
  }
  else if(strcmp(object->getType(), "Switch") == 0) {
    return (int)lv_obj_get_state(object->getObject()) & LV_STATE_CHECKED ? 1 : 0;
  }
  else if(strcmp(object->getType(), "Checkbox") == 0) {
    return (int)lv_obj_get_state(object->getObject()) & LV_STATE_CHECKED ? 1 : 0;
  }
  else {
    Serial.print(F("[MicroGUI]: Could not get the value of "));
    Serial.print(F(obj_name));
    return -1;
  }
}

/* Update GUI document with latest values/states */
void mgui_update_doc() {
  DynamicJsonDocument doc(sizeof(document));

  DeserializationError error = deserializeJson(doc, (const char*)document);
  if(error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  JsonObject root = doc.as<JsonObject>();
  
  // Loop through all objects in all lists and update each object in the json document
  for(int i = 0; i < textfields.size(); i++) {
    root[textfields.get(i)->getParent()]["props"]["text"] = lv_label_get_text(textfields.get(i)->getObject());
  }
  for(int i = 0; i < switches.size(); i++) {
    root[switches.get(i)->getParent()]["props"]["state"] = (int)lv_obj_get_state(switches.get(i)->getObject()) & LV_STATE_CHECKED ? 1 : 0;
  }
  for(int i = 0; i < checkboxes.size(); i++) {
    root[checkboxes.get(i)->getParent()]["props"]["state"] = (int)lv_obj_get_state(checkboxes.get(i)->getObject()) & LV_STATE_CHECKED ? 1 : 0;
  }
  for(int i = 0; i < sliders.size(); i++) {
    root[sliders.get(i)->getParent()]["props"]["value"] = lv_slider_get_value(sliders.get(i)->getObject());
  }
  //Unsure if this is how to do it
  for(int i = 0; i < radiobuttons.size(); i++) {
    root[radiobuttons.get(i)->getParent()]["props"]["state"] = (int)lv_obj_get_state(radiobuttons.get(i)->getObject()) & LV_STATE_CHECKED ? 1 : 0;
  }
  // TODO: add for progressbars

  serializeJson(root, document);
  doc.clear();
}

/* Function for rendering a canvas */
void mgui_render_canvas(JsonPair kv, JsonObject root) {
  lv_obj_t * canvas = lv_obj_create(lv_scr_act());
  lv_obj_set_size(canvas, screenWidth, screenHeight);
  lv_obj_align(canvas, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_bg_color(canvas, lv_color_make(root[kv.key()]["props"]["background"]["r"], root[kv.key()]["props"]["background"]["g"], root[kv.key()]["props"]["background"]["b"]), 0);
  lv_obj_set_style_border_width(canvas, 0, 0);
  lv_obj_set_style_border_width(canvas, 0, 0);
  lv_obj_set_style_radius(canvas, 0, 0);
}

/* Function for rendering a button */
void mgui_render_button(JsonPair kv, JsonObject root) {
  // Create LVGL object
  lv_obj_t * button = lv_btn_create(lv_scr_act());

  // Create MGUI_object for newly created button
  MGUI_object * m_button = new MGUI_object;
  m_button->setObject(button);
  memcpy(m_button->getType(), (const char*)root[kv.key()]["type"]["resolvedName"], strlen((const char*)root[kv.key()]["type"]["resolvedName"]));
  memcpy(m_button->getParent(), kv.key().c_str(), strlen(kv.key().c_str()));
  memcpy(m_button->getEvent(), (const char*)root[kv.key()]["props"]["event"], strlen((const char*)root[kv.key()]["props"]["event"]));

  // Store MGUI_object pointer in linked list
  buttons.add(m_button);

  // Store the MGUI object as user data
  lv_obj_set_user_data(button, m_button);

  // Add event callback
  lv_obj_add_event_cb(button, widget_cb, LV_EVENT_CLICKED, NULL);   

  // Styling
  lv_obj_set_pos(button, root[kv.key()]["props"]["pageX"], root[kv.key()]["props"]["pageY"]);
  lv_obj_set_height(button, LV_SIZE_CONTENT);
  lv_obj_set_style_bg_color(button, lv_color_make(root[kv.key()]["props"]["background"]["r"], root[kv.key()]["props"]["background"]["g"], root[kv.key()]["props"]["background"]["b"]), 0);
  
  // Add label to button
  lv_obj_t * label = lv_label_create(button);
  const char* text = root[kv.key()]["props"]["text"];
  lv_label_set_text(label, text);
  lv_obj_center(label);
  lv_obj_set_style_text_color(label, lv_color_make(root[kv.key()]["props"]["color"]["r"], root[kv.key()]["props"]["color"]["g"], root[kv.key()]["props"]["color"]["b"]), 0);   
}

/* Function for rendering a switch */
void mgui_render_switch(JsonPair kv, JsonObject root) {
  lv_obj_t * sw = lv_switch_create(lv_scr_act());
  
  MGUI_object * m_switch = new MGUI_object;
  m_switch->setObject(sw);
  memcpy(m_switch->getType(), (const char*)root[kv.key()]["type"]["resolvedName"], strlen((const char*)root[kv.key()]["type"]["resolvedName"]));
  memcpy(m_switch->getParent(), kv.key().c_str(), strlen(kv.key().c_str()));
  memcpy(m_switch->getEvent(), (const char*)root[kv.key()]["props"]["event"], strlen((const char*)root[kv.key()]["props"]["event"]));

  switches.add(m_switch);
  lv_obj_set_user_data(sw, m_switch);
  lv_obj_add_event_cb(sw, widget_cb, LV_EVENT_VALUE_CHANGED, NULL);
  
  if(root[kv.key()]["props"]["state"]) {
    lv_obj_add_state(sw, LV_STATE_CHECKED);
  }

  // Styling
  lv_obj_set_pos(sw, root[kv.key()]["props"]["pageX"], root[kv.key()]["props"]["pageY"]);
  lv_obj_set_style_bg_color(sw, lv_color_make(188, 188, 188), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(sw, lv_color_make(root[kv.key()]["props"]["color"]["r"], root[kv.key()]["props"]["color"]["g"], root[kv.key()]["props"]["color"]["b"]), LV_PART_INDICATOR | LV_STATE_CHECKED);

  if(mgui_compare((const char*)root[kv.key()]["props"]["size"], "medium")) {
    lv_obj_set_height(sw, (lv_coord_t)25);
    lv_obj_set_width(sw, (lv_coord_t)56);
    lv_obj_set_style_pad_all(sw, (lv_coord_t)4, LV_PART_KNOB);
  } else {
    lv_obj_set_height(sw, (lv_coord_t)20);
    lv_obj_set_width(sw, (lv_coord_t)44);
    lv_obj_set_style_pad_all(sw, (lv_coord_t)3, LV_PART_KNOB);
  }
}

/* Function for rendering a slider */
void mgui_render_slider(JsonPair kv, JsonObject root) {
  lv_obj_t * slider = lv_slider_create(lv_scr_act());

  MGUI_object * m_slider = new MGUI_object;
  m_slider->setObject(slider);
  memcpy(m_slider->getType(), (const char*)root[kv.key()]["type"]["resolvedName"], strlen((const char*)root[kv.key()]["type"]["resolvedName"]));
  memcpy(m_slider->getParent(), kv.key().c_str(), strlen(kv.key().c_str()));
  memcpy(m_slider->getEvent(), (const char*)root[kv.key()]["props"]["event"], strlen((const char*)root[kv.key()]["props"]["event"]));

  sliders.add(m_slider);
  lv_obj_set_user_data(slider, m_slider);
  lv_obj_add_event_cb(slider, widget_cb, LV_EVENT_VALUE_CHANGED, NULL);   // LV_EVENT_ALL did not work, hence the two callback setups
  lv_obj_add_event_cb(slider, widget_cb, LV_EVENT_RELEASED, NULL);

  // Styling
  lv_obj_set_width(slider, root[kv.key()]["props"]["width"]);
  lv_obj_set_pos(slider, root[kv.key()]["props"]["pageX"], root[kv.key()]["props"]["pageY"]);
  lv_slider_set_range(slider, root[kv.key()]["props"]["min"], root[kv.key()]["props"]["max"]);
  lv_slider_set_value(slider, root[kv.key()]["props"]["value"], LV_ANIM_OFF);
  lv_obj_set_style_bg_color(slider, lv_color_make(root[kv.key()]["props"]["color"]["r"], root[kv.key()]["props"]["color"]["g"], root[kv.key()]["props"]["color"]["b"]), LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(slider, lv_color_make(root[kv.key()]["props"]["color"]["r"], root[kv.key()]["props"]["color"]["g"], root[kv.key()]["props"]["color"]["b"]), LV_PART_KNOB);
}

/* Function for rendering a checkbox */
void mgui_render_checkbox(JsonPair kv, JsonObject root) {
  lv_obj_t * checkbox = lv_checkbox_create(lv_scr_act());

  MGUI_object * m_checkbox = new MGUI_object;
  m_checkbox->setObject(checkbox);
  memcpy(m_checkbox->getType(), (const char*)root[kv.key()]["type"]["resolvedName"], strlen((const char*)root[kv.key()]["type"]["resolvedName"]));
  memcpy(m_checkbox->getParent(), kv.key().c_str(), strlen(kv.key().c_str()));
  memcpy(m_checkbox->getEvent(), (const char*)root[kv.key()]["props"]["event"], strlen((const char*)root[kv.key()]["props"]["event"]));

  checkboxes.add(m_checkbox);
  lv_obj_set_user_data(checkbox, m_checkbox);
  lv_obj_add_event_cb(checkbox, widget_cb, LV_EVENT_VALUE_CHANGED, NULL);

  if(root[kv.key()]["props"]["state"]) {
    lv_obj_add_state(checkbox, LV_STATE_CHECKED);
  }

  // Styling
  lv_obj_set_pos(checkbox, root[kv.key()]["props"]["pageX"], root[kv.key()]["props"]["pageY"]);
  lv_checkbox_set_text(checkbox, "");
  lv_obj_set_style_border_color(checkbox, lv_color_make(root[kv.key()]["props"]["color"]["r"], root[kv.key()]["props"]["color"]["g"], root[kv.key()]["props"]["color"]["b"]), LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(checkbox, lv_color_make(255, 255, 255), LV_PART_INDICATOR | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(checkbox, lv_color_make(root[kv.key()]["props"]["color"]["r"], root[kv.key()]["props"]["color"]["g"], root[kv.key()]["props"]["color"]["b"]), LV_PART_INDICATOR | LV_STATE_CHECKED);
  
  if(mgui_compare((const char*)root[kv.key()]["props"]["size"], "medium")) {
    lv_obj_set_style_text_font(checkbox, font_list[2], 0);   // Sets checkbox size
  }
}

void radiobutton_create(lv_obj_t * parent, const char * txt, JsonObject root, JsonPair kv){
  lv_obj_t * radiobutton = lv_checkbox_create(parent);    
  lv_checkbox_set_text(radiobutton, txt);
  lv_obj_add_flag(radiobutton, LV_OBJ_FLAG_EVENT_BUBBLE);
  lv_obj_add_style(radiobutton, &style_radio, LV_PART_INDICATOR);
  lv_obj_add_style(radiobutton, &style_radio_chk, LV_PART_INDICATOR | LV_STATE_CHECKED);
  
  // Create MGUI object, TODO: Update this to be correct. Type? Parent?
  MGUI_object * m_radiobuttons = new MGUI_object;
  m_radiobuttons->setObject(radiobutton);
  memcpy(m_radiobuttons->getType(), (const char*)root[kv.key()]["type"]["resolvedName"], strlen((const char*)root[kv.key()]["type"]["resolvedName"]));
  memcpy(m_radiobuttons->getParent(), txt, strlen(txt));
  memcpy(m_radiobuttons->getEvent(), (const char*)root[kv.key()]["props"]["event"], strlen((const char*)root[kv.key()]["props"]["event"]));

  // Store MGUI_object pointer in linked list
  radiobuttons.add(m_radiobuttons);
  lv_obj_set_user_data(radiobutton, m_radiobuttons);
}

/*Function for rendering radiobuttons*/
void mgui_render_radiobuttons(JsonPair kv, JsonObject root){
  /* Create LVGL object */
  lv_obj_t * radiobutton = lv_obj_create(lv_scr_act()); // Rename??


  // Check how many radiobuttons and names
  uint32_t amount = root[kv.key()]["props"]["amount"];

  // Styling
  lv_style_init(&style_radio);
  lv_style_set_radius(&style_radio, LV_RADIUS_CIRCLE);
  lv_style_init(&style_radio_chk);
  lv_style_set_bg_img_src(&style_radio_chk, NULL);

  // Create container for the radiobuttons
  lv_obj_set_flex_flow(radiobutton, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_size(radiobutton, lv_pct(40), lv_pct(10 * amount + 18));

  // Event handling
  static uint32_t active_index_1 = 1; // 1 is the first radibutton because we have the label above. 
  lv_obj_add_event_cb(radiobutton, widget_cb, LV_EVENT_CLICKED, &active_index_1);

  // Correct colors
  lv_obj_set_style_bg_color(radiobutton, lv_color_make(root["ROOT"]["props"]["background"]["r"], root["ROOT"]["props"]["background"]["g"], root["ROOT"]["props"]["background"]["b"]), 0);
  lv_obj_set_style_border_color(radiobutton, lv_color_make(root["ROOT"]["props"]["background"]["r"], root["ROOT"]["props"]["background"]["g"], root["ROOT"]["props"]["background"]["b"]), 0);
  lv_obj_set_style_text_color(radiobutton, lv_color_make(root[kv.key()]["props"]["color"]["r"], root[kv.key()]["props"]["color"]["g"], root[kv.key()]["props"]["color"]["b"]), 0);

  lv_obj_set_pos(radiobutton, root[kv.key()]["props"]["pageX"], root[kv.key()]["props"]["pageY"]);

  // Label
  lv_obj_t * label = lv_label_create(radiobutton);
  const char* text = root[kv.key()]["props"]["text"];
  lv_label_set_text(label, text);

  uint32_t i;
  char buf[32];

  for(i = 0; i < amount; i++) {
      lv_snprintf(buf, sizeof(buf), root[kv.key()]["props"]["labelTexts"][i], (int)i + 1);
      radiobutton_create(radiobutton, buf, root, kv);
  }
  
  // Make the first checkbox checked
  lv_obj_add_state(lv_obj_get_child(radiobutton, 1), LV_STATE_CHECKED);
}

/* Function for rendering a progressbar */
void mgui_render_progressbar(JsonPair kv, JsonObject root) {
  static lv_style_t style_bg;
  static lv_style_t style_indic;

  lv_obj_t * progressbar = lv_bar_create(lv_scr_act());
  // Create MGUI_object for newly created component
  MGUI_object * m_progressbar = new MGUI_object;
  m_progressbar->setObject(progressbar);
  memcpy(m_progressbar->getType(), (const char*)root[kv.key()]["type"]["resolvedName"], strlen((const char*)root[kv.key()]["type"]["resolvedName"]));
  memcpy(m_progressbar->getParent(), kv.key().c_str(), strlen(kv.key().c_str()));
  memcpy(m_progressbar->getEvent(), (const char*)root[kv.key()]["props"]["event"], strlen((const char*)root[kv.key()]["props"]["event"]));

  // Store MGUI_object pointer in linked list
  progressbars.add(m_progressbar);

  // Store the MGUI_object as user data
  lv_obj_set_user_data(progressbar, m_progressbar);

  // Styling and placement 
  lv_obj_set_size(progressbar, root[kv.key()]["props"]["size"], 20);
  lv_obj_set_pos(progressbar, root[kv.key()]["props"]["pageX"], root[kv.key()]["props"]["pageY"]);

  // Value, temporary
  lv_bar_set_value(progressbar, 70, LV_ANIM_ON);
}

/* Function for rendering a textfield */
void mgui_render_textfield(JsonPair kv, JsonObject root) {
  lv_obj_t * textfield = lv_label_create(lv_scr_act());

  MGUI_object * m_textfield = new MGUI_object;
  m_textfield->setObject(textfield);
  memcpy(m_textfield->getType(), (const char*)root[kv.key()]["type"]["resolvedName"], strlen((const char*)root[kv.key()]["type"]["resolvedName"]));
  memcpy(m_textfield->getParent(), kv.key().c_str(), strlen(kv.key().c_str()));
  memcpy(m_textfield->getEvent(), "NoInput", strlen("NoInput"));

  textfields.add(m_textfield);
  lv_obj_set_user_data(textfield, m_textfield);

  // Styling
  lv_obj_set_pos(textfield, root[kv.key()]["props"]["pageX"], root[kv.key()]["props"]["pageY"]);
  const char* text = root[kv.key()]["props"]["text"];
  lv_label_set_text(textfield, text);
  lv_obj_set_style_text_color(textfield, lv_color_make(root[kv.key()]["props"]["color"]["r"], root[kv.key()]["props"]["color"]["g"], root[kv.key()]["props"]["color"]["b"]), 0);

  uint8_t fontSize = (uint8_t)root[kv.key()]["props"]["fontSize"];
  uint8_t i = 0;
 
  while(i < sizeof font_sizes && fontSize > font_sizes[i]) {
    i++;
  }
  lv_obj_set_style_text_font(textfield, font_list[i], 0);   // Sets font size

}

/* Function for rendering a divider */
void mgui_render_divider(JsonPair kv, JsonObject root) {
  lv_obj_t * divider = lv_obj_create(lv_scr_act());

  MGUI_object * m_divider = new MGUI_object;
  m_divider->setObject(divider);
  memcpy(m_divider->getType(), (const char*)root[kv.key()]["type"]["resolvedName"], strlen((const char*)root[kv.key()]["type"]["resolvedName"]));
  memcpy(m_divider->getParent(), kv.key().c_str(), strlen(kv.key().c_str()));
  memcpy(m_divider->getEvent(), "NoInput", strlen("NoInput"));

  dividers.add(m_divider);
  lv_obj_set_user_data(divider, m_divider);

  int height = (int)root[kv.key()]["props"]["thickness"];
  int width = (int)root[kv.key()]["props"]["length"];

  if(mgui_compare(root[kv.key()]["props"]["orientation"], "vertical")) {
    int temp = height;
    height = width;
    width = temp;
  }

  lv_obj_set_size(divider, width, height);
  lv_obj_align(divider, LV_ALIGN_TOP_LEFT, (lv_coord_t)root[kv.key()]["props"]["pageX"], (lv_coord_t)root[kv.key()]["props"]["pageY"]);
  lv_obj_set_style_bg_color(divider, lv_color_make(root[kv.key()]["props"]["color"]["r"], root[kv.key()]["props"]["color"]["g"], root[kv.key()]["props"]["color"]["b"]), 0);
  lv_obj_set_style_border_width(divider, 0, 0);
  lv_obj_set_style_border_width(divider, 0, 0);
  lv_obj_set_style_radius(divider, 0, 0);
  lv_obj_set_scrollbar_mode(divider, LV_SCROLLBAR_MODE_OFF);
}

// Initial attempt for rendering the divider element. This is not a suitable solution at the moment because of a 
// difference in line origins in MicroGUI Web app and LVGL. In web app origin is top left of object, in LVGL it is middle left.
// Saved for potential future uses...

// static lv_point_t divider_points[100][2];
// int divIndex = 0;

// void mgui_render_divider(JsonPair kv, JsonObject root) {
//   divider_points[divIndex][0] = {(lv_coord_t)root[kv.key()]["props"]["pageX"], (lv_coord_t)((int)root[kv.key()]["props"]["pageY"] + ((int)root[kv.key()]["props"]["thickness"] / 2))};
//   divider_points[divIndex][1] = {(lv_coord_t)((int)root[kv.key()]["props"]["pageX"] + (int)root[kv.key()]["props"]["length"]), (lv_coord_t)((int)root[kv.key()]["props"]["pageY"] + ((int)root[kv.key()]["props"]["thickness"] / 2))};

//   lv_obj_t * divider = lv_line_create(lv_scr_act());

//   MGUI_object * m_divider = new MGUI_object;
//   m_divider->setObject(divider);
//   memcpy(m_divider->getType(), (const char*)root[kv.key()]["type"]["resolvedName"], strlen((const char*)root[kv.key()]["type"]["resolvedName"]));
//   memcpy(m_divider->getParent(), kv.key().c_str(), strlen(kv.key().c_str()));
//   memcpy(m_divider->getEvent(), "NoInput", strlen("NoInput"));

//   dividers.add(m_divider);
//   lv_obj_set_user_data(divider, m_divider);

//   lv_obj_set_style_line_width(divider, (int)root[kv.key()]["props"]["thickness"], 0);
//   lv_obj_set_style_line_color(divider, lv_color_make(root[kv.key()]["props"]["color"]["r"], root[kv.key()]["props"]["color"]["g"], root[kv.key()]["props"]["color"]["b"]), 0);
//   lv_obj_set_style_pad_all(divider, 0, 0);
//   lv_obj_set_style_line_rounded(divider, false, 0);
//   lv_line_set_points(divider, divider_points[divIndex], 2);     /*Set the points*/

//   divIndex++;
// }

/* Render a red border around the GUI to indicate something, currently used for indication disconnected WiFi */
void mgui_render_border() {
  /*Create an array for the points of the line*/
  static lv_point_t line_points[] = { {4, 4}, {(lv_coord_t)(screenWidth-4), 4}, {(lv_coord_t)(screenWidth-4), (lv_coord_t)(screenHeight-4)}, {4, (lv_coord_t)(screenHeight-4)}, {4, 4} };

  /*Create style*/
  static lv_style_t style_line;
  lv_style_init(&style_line);
  lv_style_set_line_width(&style_line, 4);
  lv_style_set_line_color(&style_line, lv_palette_main(LV_PALETTE_RED));
  lv_style_set_line_rounded(&style_line, true);

  /*Create a line and apply the new style*/
  border = lv_line_create(lv_scr_act());
  lv_line_set_points(border, line_points, 5);     /*Set the points*/
  lv_obj_add_style(border, &style_line, 0);
  lv_obj_center(border);
}

/* Show border */
void mgui_show_border() {
  if(!border_vis) {
    mgui_render_border();
    border_vis = true;
  }
}

/* Hide border */
void mgui_hide_border() {
  if(border_vis) {
    lv_obj_del(border);
    border_vis = false;
  }
}


/** Display functions */

/* Display callback to flush the buffer to screen */
void display_flush(lv_disp_drv_t * disp, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  lcd.startWrite();
  lcd.setAddrWindow(area->x1, area->y1, w, h);
  lcd.pushPixels((uint16_t *)&color_p->full, w * h, true);
  lcd.endWrite();

  lv_disp_flush_ready(disp);
}

/* Touchpad callback to read the touchpad */
void touchpad_read(lv_indev_drv_t * indev_driver, lv_indev_data_t * data) {
  uint16_t touchX, touchY;
  bool touched = lcd.getTouch(&touchX, &touchY);

  if (!touched){
    data->state = LV_INDEV_STATE_REL;
  }
  else{
    data->state = LV_INDEV_STATE_PR;

    /*Set the coordinates*/
    data->point.x = touchX;
    data->point.y = touchY;
  }
}