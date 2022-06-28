///////////////////////////////////////////////////////////
//                                                       //
//   Rendering motor for generating GUIs created with    //
//       MicroGUI on embedded hardware (displays)        //
//                                                       //
//   Utilized libraries: LovyanGFX, LVGL, ArduinoJSON    //
//                                                       //
//       written by Linus Johansson, 2022 @ Plejd        //
//                                                       //
///////////////////////////////////////////////////////////

#include <Arduino.h>

#include "MicroGUI.h"

#define LGFX_AUTODETECT // Autodetect board
#define LGFX_USE_V1     // set to use new version of library

#include <LovyanGFX.hpp> // main library

#include <lvgl.h>
#include "lv_conf.h"

#include <ArduinoJson.h>
#include <ArduinoJson.hpp>

#include <string.h>


static LGFX lcd; // declare display variable

/*** Setup screen resolution for LVGL ***/
static const uint16_t screenWidth = 480;
static const uint16_t screenHeight = 320;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * 10];


/* MicroGUI event class functions */

MGUI_event::MGUI_event(lv_obj_t * obj, int val) {
  object = obj;
  value = val;
}

MGUI_event::MGUI_event(lv_obj_t * obj) {
  object = obj;
  value = 0;
}

lv_obj_t* MGUI_event::getObject() {
  return object;
}

char* MGUI_event::getName() {
  return (char*)lv_obj_get_user_data(object);
}

bool MGUI_event::getState() {
  return value;
}

int MGUI_event::getValue() {
  return value;
}

/* Queue for MicroGUI events */



void mgui_init(char json[]) {
  lcd.init(); // Initialize LovyanGFX
  lv_init();  // Initialize lvgl

  // Setting display to landscape
  if (lcd.width() < lcd.height())
    lcd.setRotation(lcd.getRotation() ^ 1);

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

  mgui_render(json);
}

// Returnera ett event
void mgui_run() {
  lv_timer_handler();   /* let the GUI do its work */
}


static void widget_cb(lv_event_t * e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t * object = lv_event_get_target(e);
  if (code == LV_EVENT_CLICKED) {
    MGUI_event event(object);
    Serial.println(event.getName());
    Serial.println(event.getValue());
  }
  else if (code == LV_EVENT_VALUE_CHANGED) {
    if(lv_obj_check_type(object, &lv_slider_class)) {
      MGUI_event event(object, lv_slider_get_value(object));
      Serial.println(event.getName());
      Serial.println(event.getValue());
    } 
    else if (lv_obj_check_type(object, &lv_switch_class) || lv_obj_check_type(object, &lv_checkbox_class)) {
      MGUI_event event(object, (int)lv_obj_get_state(object) & LV_STATE_CHECKED ? 1 : 0);
      Serial.println(event.getName());
      Serial.println(event.getValue());
    }
  }
}


void mgui_render(char json[])
{
  DynamicJsonDocument doc(strnlen(json, 100000) + 1000);    // Length of JSON plus some slack

  DeserializationError error = deserializeJson(doc, json);

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  JsonObject root = doc.as<JsonObject>();

  // List all keys
  for (JsonPair kv : root) {
    //Serial.println(kv.key().c_str());

    // Find object type
    String type = root[kv.key()]["type"]["resolvedName"];

    // If object is the Canvas, i.e background
    if(type.equals("CanvasArea")) {
      lv_obj_t *square;
      square = lv_obj_create(lv_scr_act());
      lv_obj_set_size(square, screenWidth, screenHeight);
      lv_obj_align(square, LV_ALIGN_CENTER, 0, 0);
      lv_obj_set_style_bg_color(square, lv_color_make(root[kv.key()]["props"]["background"]["r"], root[kv.key()]["props"]["background"]["g"], root[kv.key()]["props"]["background"]["b"]), 0);
      lv_obj_set_style_border_width(square, 0, 0);
      lv_obj_set_style_border_width(square, 0, 0);
      lv_obj_set_style_radius(square, 0, 0);
    }
    // If object is a button
    else if(type.equals("Button")) {
      lv_obj_t * button = lv_btn_create(lv_scr_act());

      //Serial.println((int)button);      // Pointer to the created button

      lv_obj_set_user_data(button, (char*)kv.key().c_str());    // Store the name of the widget as user data

      lv_obj_add_event_cb(button, widget_cb, LV_EVENT_CLICKED, (char*)kv.key().c_str());

      lv_obj_set_pos(button, root[kv.key()]["props"]["pageX"], root[kv.key()]["props"]["pageY"]);
      lv_obj_set_height(button, LV_SIZE_CONTENT);
      lv_obj_set_style_bg_color(button, lv_color_make(root[kv.key()]["props"]["background"]["r"], root[kv.key()]["props"]["background"]["g"], root[kv.key()]["props"]["background"]["b"]), 0);
      
      lv_obj_t * label = lv_label_create(button);
      const char* text = root[kv.key()]["props"]["text"];
      lv_label_set_text(label, text);
      lv_obj_center(label);
      lv_obj_set_style_text_color(label, lv_color_make(root[kv.key()]["props"]["color"]["r"], root[kv.key()]["props"]["color"]["g"], root[kv.key()]["props"]["color"]["b"]), 0);
    }
    // If object is a switch
    else if(type.equals("Switch")) {
      lv_obj_t * sw = lv_switch_create(lv_scr_act());

      lv_obj_set_user_data(sw, (char*)kv.key().c_str());    // Store the name of the widget as user data

      lv_obj_add_event_cb(sw, widget_cb, LV_EVENT_VALUE_CHANGED, NULL);

      //lv_obj_add_state(switches[numSwitches], LV_STATE_CHECKED);
      //lv_obj_add_event_cb(switches[numSwitches], counter_event_handler, LV_EVENT_ALL, NULL);
      lv_obj_set_pos(sw, root[kv.key()]["props"]["pageX"], root[kv.key()]["props"]["pageY"]);
      lv_obj_set_style_bg_color(sw, lv_color_make(188, 188, 188), LV_PART_MAIN | LV_STATE_DEFAULT);
      //lv_obj_set_style_bg_color(switches[numSwitches], lv_color_make(root[kv.key()]["props"]["color"]["r"], root[kv.key()]["props"]["color"]["g"], root[kv.key()]["props"]["color"]["b"]), LV_PART_KNOB);
      lv_obj_set_style_bg_color(sw, lv_color_make(root[kv.key()]["props"]["color"]["r"], root[kv.key()]["props"]["color"]["g"], root[kv.key()]["props"]["color"]["b"]), LV_PART_INDICATOR | LV_STATE_CHECKED);
    }
    // If object is a slider
    else if(type.equals("Slider")) {
      lv_obj_t * slider = lv_slider_create(lv_scr_act());

      lv_obj_set_user_data(slider, (char*)kv.key().c_str());    // Store the name of the widget as user data

      lv_obj_add_event_cb(slider, widget_cb, LV_EVENT_VALUE_CHANGED, NULL);

      lv_obj_set_width(slider, root[kv.key()]["props"]["width"]);
      lv_obj_set_pos(slider, root[kv.key()]["props"]["pageX"], root[kv.key()]["props"]["pageY"]);
      lv_slider_set_range(slider, root[kv.key()]["props"]["min"], root[kv.key()]["props"]["max"]);
      lv_obj_set_style_bg_color(slider, lv_color_make(root[kv.key()]["props"]["color"]["r"], root[kv.key()]["props"]["color"]["g"], root[kv.key()]["props"]["color"]["b"]), LV_PART_INDICATOR);
      lv_obj_set_style_bg_color(slider, lv_color_make(root[kv.key()]["props"]["color"]["r"], root[kv.key()]["props"]["color"]["g"], root[kv.key()]["props"]["color"]["b"]), LV_PART_KNOB);
    }
    // If object is a textfield
    else if(type.equals("Textfield")) {
      lv_obj_t * textfield = lv_label_create(lv_scr_act());

      lv_obj_set_user_data(textfield, (char*)kv.key().c_str());    // Store the name of the widget as user data

      lv_obj_set_pos(textfield, root[kv.key()]["props"]["pageX"], root[kv.key()]["props"]["pageY"]);
      const char* text = root[kv.key()]["props"]["text"];
      lv_label_set_text(textfield, text);
      lv_obj_set_style_text_color(textfield, lv_color_make(root[kv.key()]["props"]["color"]["r"], root[kv.key()]["props"]["color"]["g"], root[kv.key()]["props"]["color"]["b"]), 0);
    }
    // If object is a checkbox
    else if(type.equals("Checkbox")) {
      lv_obj_t * checkbox = lv_checkbox_create(lv_scr_act());

      lv_obj_set_user_data(checkbox, (char*)kv.key().c_str());    // Store the name of the widget as user data

      lv_obj_add_event_cb(checkbox, widget_cb, LV_EVENT_VALUE_CHANGED, NULL);
      lv_obj_set_pos(checkbox, root[kv.key()]["props"]["pageX"], root[kv.key()]["props"]["pageY"]);
      lv_checkbox_set_text(checkbox, "");
      lv_obj_set_style_border_color(checkbox, lv_color_make(root[kv.key()]["props"]["color"]["r"], root[kv.key()]["props"]["color"]["g"], root[kv.key()]["props"]["color"]["b"]), LV_PART_INDICATOR);
      lv_obj_set_style_bg_color(checkbox, lv_color_make(255, 255, 255), LV_PART_INDICATOR | LV_STATE_DEFAULT);
      lv_obj_set_style_bg_color(checkbox, lv_color_make(root[kv.key()]["props"]["color"]["r"], root[kv.key()]["props"]["color"]["g"], root[kv.key()]["props"]["color"]["b"]), LV_PART_INDICATOR | LV_STATE_CHECKED);
    }
  }
}


/*** Display callback to flush the buffer to screen ***/
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

/*** Touchpad callback to read the touchpad ***/
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