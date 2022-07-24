//
//   Library for making MicroGUI functional on embedded displays
//
//   Utilized libraries: LovyanGFX, LVGL, ArduinoJSON, LinkedList
// 
//   written by Linus Johansson, 2022 @ Plejd
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
bool newEvent = false;

/* Linked lists for storing object pointers, for later access */
LinkedList<MGUI_object*> buttons;
LinkedList<MGUI_object*> switches;
LinkedList<MGUI_object*> sliders;
LinkedList<MGUI_object*> checkboxes;
LinkedList<MGUI_object*> textfields;

/* For storing the initial json document internally */
char document[20000];

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


/* MicroGUI functions */

/* Parse json for important data in the beginning */
void mgui_parse(char json[]) {
  //document = json;    // Store a reference to the GUI document
  memcpy(document, json, strlen(json));

  DynamicJsonDocument doc(strnlen(document, 100000)*1.5);    // Length of JSON plus some slack

  DeserializationError error = deserializeJson(doc, (const char*)document);
  if(error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  JsonObject root = doc.as<JsonObject>();

  screenWidth = root["ROOT"]["props"]["width"];
  screenHeight = root["ROOT"]["props"]["height"];

  doc.clear();
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

  mgui_render(document);
}

/* Let the display do its' work, returns a MicroGUI event */
MGUI_event * mgui_run() {
  lv_timer_handler();

  if(getRemoteInit()) {
    mgui_run_captive();
  }

  if(newEvent) {      // Only return new events
    newEvent = false;

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
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t * object = lv_event_get_target(e);

  delete latest;

  int value;

  if(code == LV_EVENT_CLICKED) {     // If button short-click, more events available from LVGL, implement later?
    value = 1;
    latest = new MGUI_event(((MGUI_object*)lv_obj_get_user_data(object))->getEvent(), 
                            ((MGUI_object*)lv_obj_get_user_data(object))->getParent(), 
                            value);
  }
  else if(code == LV_EVENT_VALUE_CHANGED) {
    if(lv_obj_check_type(object, &lv_slider_class)) {     // If slider
      value = lv_slider_get_value(object);
      latest = new MGUI_event(((MGUI_object*)lv_obj_get_user_data(object))->getEvent(), 
                              ((MGUI_object*)lv_obj_get_user_data(object))->getParent(), 
                              value);
    } 
    else if(lv_obj_check_type(object, &lv_switch_class) || lv_obj_check_type(object, &lv_checkbox_class)) {    // If switch or checkbox
      value = (int)lv_obj_get_state(object) & LV_STATE_CHECKED ? 1 : 0;
      latest = new MGUI_event(((MGUI_object*)lv_obj_get_user_data(object))->getEvent(), 
                              ((MGUI_object*)lv_obj_get_user_data(object))->getParent(), 
                              value);
    }
  }
  newEvent = true;

  if(getRemoteInit()) {
      char buf[100];
      sprintf(buf, "{\"%s\": %i}", ((MGUI_object*)lv_obj_get_user_data(object))->getParent(), value);
      //Serial.println(buf);
      mgui_send(buf);
  }
}

/* Render MicroGUI from json */
void mgui_render(char json[]) {
  DynamicJsonDocument doc(strnlen(json, 100000)*1.5);    // Length of JSON plus some slack

  DeserializationError error = deserializeJson(doc, (const char*)json);
  if(error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

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
  }

  doc.clear();
}

/* Search for an object in a list with corresponding name */
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
    Serial.print(F("Could not change the value of "));
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
    Serial.println("Updating text of buttons is not yet supported");
    return;
  } 
  else {
    Serial.print(F("Could not change the text of "));
    Serial.print(F(obj_name));
    return;
  }
}

int mgui_get_value(const char * obj_name) {
  MGUI_object * object = mgui_find_object(obj_name, &sliders);
  if(strcmp(object->getType(), "None") == 0) {
    object = mgui_find_object(obj_name, &switches);
  }
  if(strcmp(object->getType(), "None") == 0) {
    object = mgui_find_object(obj_name, &checkboxes);
  }

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
    Serial.print(F("Could not get the value of "));
    Serial.print(F(obj_name));
    return 0;
  }
}

/* Update GUI document with latest values/states */
void mgui_update_doc() {
  DynamicJsonDocument doc(strnlen(document, 100000)*1.5);    // Length of JSON plus some slack

  DeserializationError error = deserializeJson(doc, (const char*)document);
  if(error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  JsonObject root = doc.as<JsonObject>();
  
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

  serializeJson(root, document);
  doc.clear();
}

/* Function for rendering a canvas */
void mgui_render_canvas(JsonPair kv, JsonObject root) {
  lv_obj_t *square = lv_obj_create(lv_scr_act());
  lv_obj_set_size(square, screenWidth, screenHeight);
  lv_obj_align(square, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_bg_color(square, lv_color_make(root[kv.key()]["props"]["background"]["r"], root[kv.key()]["props"]["background"]["g"], root[kv.key()]["props"]["background"]["b"]), 0);
  lv_obj_set_style_border_width(square, 0, 0);
  lv_obj_set_style_border_width(square, 0, 0);
  lv_obj_set_style_radius(square, 0, 0);
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
  lv_obj_add_event_cb(slider, widget_cb, LV_EVENT_VALUE_CHANGED, NULL);

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

  if (!touched)
  {
    data->state = LV_INDEV_STATE_REL;
  }
  else
  {
    data->state = LV_INDEV_STATE_PR;

    /*Set the coordinates*/
    data->point.x = touchX;
    data->point.y = touchY;

    // Serial.printf("Touch (x,y): (%03d,%03d)\n",touchX,touchY );
  }
}