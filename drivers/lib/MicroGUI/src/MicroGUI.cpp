//
//   Library for making MicroGUI functional on embedded displays
//
//   Utilized libraries: LovyanGFX, LVGL, ArduinoJSON, LinkedList
// 
//   written by Linus Johansson, 2022 @ Plejd
//

#include <Arduino.h>

#include "MicroGUI.h"
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

/*** Setup screen resolution for LVGL ***/
// screenHeight and screenWidth would need to be 
// switched for the display to work properly in portrait mode
// Fix later
static uint16_t screenWidth;
static uint16_t screenHeight;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[4800 * 1];


/* Variables/objects for MicroGUI events */
static MGUI_event default_event("Default", "None", 0);
static MGUI_event latest;
static bool newEvent = false;

/* Linked lists for storing object pointers, for later access */
LinkedList<MGUI_object*> buttons;
LinkedList<MGUI_object*> switches;
LinkedList<MGUI_object*> sliders;
LinkedList<MGUI_object*> checkboxes;
LinkedList<MGUI_object*> textfields;

/* For storing the initial json document internally */
char * document;


/* MicroGUI object class functions */

MGUI_object::MGUI_object(lv_obj_t * obj, const char * obj_name, const char * obj_event) {
  object = obj;
  name = obj_name;
  event = obj_event;
}

lv_obj_t * MGUI_object::getObject() {
  return object;
}

const char* MGUI_object::getEvent() {
  return event;
}

const char* MGUI_object::getParent() {
  return name;
}

/* MicroGUI event class functions */

MGUI_event::MGUI_event(const char * event, const char * parent, int val) {
  event_id = event;
  parent_id = parent;
  value = val;
}

MGUI_event::MGUI_event() {
  event_id = "";
  parent_id = "";
  value = 0;
}

const char* MGUI_event::getEvent() {
  return event_id;
}

const char* MGUI_event::getParent() {
  return parent_id;
}

bool MGUI_event::getState() {
  return value;
}

int MGUI_event::getValue() {
  return value;
}


/* MicroGUI functions */

/* Parse json for important data in the beginning */
void mgui_parse(char json[]) {
  document = json;    // Make a copy of input array, stored for internal use

  DynamicJsonDocument doc(strnlen(document, 100000)*1.5);    // Length of JSON plus some slack

  DeserializationError error = deserializeJson(doc, (const char*)document);
  if (error) {
    // TODO: Render a text telling the user that the deserialization failed
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  JsonObject root = doc.as<JsonObject>();

  screenWidth = root["ROOT"]["props"]["width"];
  screenHeight = root["ROOT"]["props"]["height"];

  doc.clear();
}

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

  /*** LVGL : Setup & Initialize the display device driver ***/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = display_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  /*** LVGL : Setup & Initialize the input device driver ***/
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = touchpad_read;
  lv_indev_drv_register(&indev_drv);

  mgui_render(document);

  mgui_run();
  delay(5);
  mgui_run();
}

/* Let the display do its' work, returns an MGUI_event object */
MGUI_event mgui_run() {
  delay(2);
  lv_timer_handler();

  if(newEvent) {      // Only return new events
    newEvent = false;

    #if 1
    Serial.print(F("\n[MicroGUI Event] Parent: "));
    Serial.println(latest.getParent());
    Serial.print(F("[MicroGUI Event] Type: "));
    Serial.println(latest.getEvent());
    Serial.print(F("[MicroGUI Event] Value: "));
    Serial.println(latest.getValue());
    #endif

    return latest;
  }
  return default_event;
}

/* Callback function for all widget actions on the display, turn them into MGUI events */
static void widget_cb(lv_event_t * e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t * object = lv_event_get_target(e);

  if (code == LV_EVENT_CLICKED) {     // If button short-click, more events available from LVGL, implement later?
    latest = MGUI_event(((MGUI_object*)lv_obj_get_user_data(object))->getEvent(), 
                        ((MGUI_object*)lv_obj_get_user_data(object))->getParent(), 1);
  }
  else if (code == LV_EVENT_VALUE_CHANGED) {
    if(lv_obj_check_type(object, &lv_slider_class)) {     // If slider
      latest = MGUI_event(((MGUI_object*)lv_obj_get_user_data(object))->getEvent(), 
                          ((MGUI_object*)lv_obj_get_user_data(object))->getParent(), 
                          lv_slider_get_value(object));
    } 
    else if (lv_obj_check_type(object, &lv_switch_class) || lv_obj_check_type(object, &lv_checkbox_class)) {    // If switch or checkbox
      latest = MGUI_event(((MGUI_object*)lv_obj_get_user_data(object))->getEvent(), 
                          ((MGUI_object*)lv_obj_get_user_data(object))->getParent(), 
                          (int)lv_obj_get_state(object) & LV_STATE_CHECKED ? 1 : 0);
    }
  }
  newEvent = true;
}

/* Render MicroGUI from json */
void mgui_render(char json[]) {
  DynamicJsonDocument doc(strnlen(json, 100000)*1.5);    // Length of JSON plus some slack

  DeserializationError error = deserializeJson(doc, (const char*)json);
  if (error) {
    // TODO: Render a text telling the user that the deserialization failed
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  JsonObject root = doc.as<JsonObject>();

  // List all keys
  for (JsonPair kv : root) {
    // Find object type
    String type = root[kv.key()]["type"]["resolvedName"];

    // If object is the Canvas, i.e background
    if(type.equals("CanvasArea")) {
      mgui_render_canvas(kv, root);
    }
    // If object is a button
    else if(type.equals("Button")) {
      mgui_render_button(kv, root);
    }
    // If object is a switch
    else if(type.equals("Switch")) {
      mgui_render_switch(kv, root);
    }
    // If object is a slider
    else if(type.equals("Slider")) {
      mgui_render_slider(kv, root);
    }
    // If object is a checkbox
    else if(type.equals("Checkbox")) {
      mgui_render_checkbox(kv, root);
    }
    // If object is a textfield
    else if(type.equals("Textfield")) {
      mgui_render_textfield(kv, root);
    }
  }

  doc.clear();
}

MGUI_object * mgui_find_object(const char * obj_name, LinkedList<MGUI_object*> *list) {
  for (int i = 0; i < list->size(); i++) {
    if (strcmp(obj_name, list->get(i)->getParent()) == 0) {
      return list->get(i);
    }
  }
  return new MGUI_object(new lv_obj_t, "None", "None");   // Questionable way of exiting function if object was not found
}

/* Returns true if strings are equal and false if not, for strings less than 50 characters */
bool mgui_compare(const char * string1, const char * string2) {
  return !(strncmp(string1, string2, 50));
}

void mgui_set_value(const char * obj_name, int value) {
  String object_name = obj_name;
  object_name = object_name.substring(0, 4);

  MGUI_object * object;

  if (object_name.equals("Text")) {
    String val = String(value);
    char buf[16];
    val.toCharArray(buf, 16);
    mgui_set_text(obj_name, buf);
    return;
  } 
  else if (object_name.equals("Slid")) {
    object = mgui_find_object(obj_name, &sliders);
    if (object->getParent() != "None") {
      lv_slider_set_value(object->getObject(), value, LV_ANIM_OFF);
      return;
    }
  } 
  else if (object_name.equals("Swit")) {
    object = mgui_find_object(obj_name, &switches);
    if (object->getParent() != "None") {
      if (value) lv_obj_add_state(object->getObject(), LV_STATE_CHECKED);
      else lv_obj_clear_state(object->getObject(), LV_STATE_CHECKED);
      return;
    }
  } 
  else if (object_name.equals("Chec")) {
    object = mgui_find_object(obj_name, &checkboxes);
    if (object->getParent() != "None") {
      if (value) lv_obj_add_state(object->getObject(), LV_STATE_CHECKED);
      else lv_obj_clear_state(object->getObject(), LV_STATE_CHECKED);
      return;
    }
  } 
  else {
    Serial.print(F("It makes no sense to change the value of"));
    Serial.println(obj_name);
    return;
  }
  Serial.print(F("Couldn't find "));
  Serial.print(F(obj_name));
  Serial.println(" :(");
}

void mgui_set_text(const char * obj_name, const char * text) {
  String object_name = obj_name;
  object_name = object_name.substring(0, 4);

  MGUI_object * object;

  if (object_name.equals("Text")) {
    Serial.print(F("Let's try to find "));
    Serial.println(obj_name);

    object = mgui_find_object(obj_name, &textfields);

    if (object->getParent() != "None") {
      Serial.print(F("Found it! Let's set that label to: "));
      Serial.println(text);
      lv_label_set_text(object->getObject(), text);
    } else {
      Serial.print(F("Couldn't find "));
      Serial.print(F(obj_name));
      Serial.println(" :(");
    }
  } 
  else if (object_name.equals("Butt")) {
    Serial.println("Changing text of buttons is not yet supported");
    return;
  } 
  else {
    Serial.print(F("It makes no sense to change the text of"));
    Serial.println(obj_name);
    return;
  }
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
  MGUI_object * m_button = new MGUI_object(button, (char*)kv.key().c_str(), root[kv.key()]["props"]["event"]);

  // Store MGUI_object pointer in linked list
  buttons.add(m_button);
  // Below is how we access MGUI objects later
  //Serial.println(buttons.get(0)->getParent());
  //Serial.println(buttons.get(1)->getParent());

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
  MGUI_object * m_switch = new MGUI_object(sw, (char*)kv.key().c_str(), root[kv.key()]["props"]["event"]);
  switches.add(m_switch);
  lv_obj_set_user_data(sw, m_switch);
  lv_obj_add_event_cb(sw, widget_cb, LV_EVENT_VALUE_CHANGED, NULL);
  
  if (root[kv.key()]["props"]["state"]) {
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
  MGUI_object * m_slider = new MGUI_object(slider, (char*)kv.key().c_str(), root[kv.key()]["props"]["event"]);
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
  MGUI_object * m_checkbox = new MGUI_object(checkbox, (char*)kv.key().c_str(), root[kv.key()]["props"]["event"]);
  checkboxes.add(m_checkbox);
  lv_obj_set_user_data(checkbox, m_checkbox);
  lv_obj_add_event_cb(checkbox, widget_cb, LV_EVENT_VALUE_CHANGED, NULL);

  if (root[kv.key()]["props"]["state"]) {
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
  MGUI_object * m_textfield = new MGUI_object(textfield, (char*)kv.key().c_str(), "NoInput");
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
void display_flush(lv_disp_drv_t * disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  lcd.startWrite();
  lcd.setAddrWindow(area->x1, area->y1, w, h);
  lcd.pushPixels((uint16_t *)&color_p->full, w * h, true);
  lcd.endWrite();

  lv_disp_flush_ready(disp);
}

/* Touchpad callback to read the touchpad */
void touchpad_read(lv_indev_drv_t * indev_driver, lv_indev_data_t * data)
{
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