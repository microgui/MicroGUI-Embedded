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

static LGFX lcd; // declare display variable

/*** Setup screen resolution for LVGL ***/
static const uint16_t screenWidth = 480;
static const uint16_t screenHeight = 320;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * 10];

// Make the storing of widgets dynamic instead of static like this
lv_obj_t * buttons[16];
short numButtons = 0;

lv_obj_t * switches[16];
short numSwitches = 0;

lv_obj_t * textfields[16];
short numTextfields = 0;

lv_obj_t * sliders[16];
short numSliders = 0;

lv_obj_t * checkboxes[16];
short numCheckboxes = 0;


void mgui_init() {
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
}

static void button1(lv_event_t * e) {
  lv_event_code_t code = lv_event_get_code(e);
  //lv_obj_t *btn = lv_event_get_target(e);
  if (code == LV_EVENT_CLICKED)
  {
    if (lv_event_get_target(e) == buttons[0]) {
        Serial.println("Button1 clicked");
    } else {
        Serial.println("Button2 clicked");
    }
  }
}

static void button2(lv_event_t * e) {
  lv_event_code_t code = lv_event_get_code(e);
  //lv_obj_t *btn = lv_event_get_target(e);
  if (code == LV_EVENT_CLICKED)
  {
    Serial.println("Button2 clicked");
  }
}

static void no_button_func(lv_event_t * e) {
  lv_event_code_t code = lv_event_get_code(e);
  //lv_obj_t *btn = lv_event_get_target(e);
  if (code == LV_EVENT_CLICKED)
  {
    Serial.println("No function defined for this button!");
  }
}

fptr find_button_cb(int itemIndex) {
  // Return 
  switch(itemIndex) {
    case 1:
      return button1;
    case 2:
      return button1;
  }

  // Default function return, if no callback function is defined for 
  return no_button_func;
}




static void switch1(lv_event_t * e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t * obj = lv_event_get_target(e);
  if(code == LV_EVENT_VALUE_CHANGED) {
      //const char * txt = lv_checkbox_get_text(obj);
      const char * state = lv_obj_get_state(obj) & LV_STATE_CHECKED ? "On" : "Off";
      Serial.println(state);
  }
}

static void switch2(lv_event_t * e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_VALUE_CHANGED)
  {
    Serial.println("Switch2 toggled");
  }
}

static void no_switch_func(lv_event_t * e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_VALUE_CHANGED)
  {
    Serial.println("No function defined for this switch!");
  }
}

fptr find_switch_cb(int itemIndex) {
  // Return 
  switch(itemIndex) {
    case 1:
      return switch1;
    case 2:
      return switch2;
  }

  // Default function return, if no callback function is defined for 
  return no_switch_func;
}



static void checkbox1(lv_event_t * e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t * obj = lv_event_get_target(e);
  if(code == LV_EVENT_VALUE_CHANGED) {
      //const char * txt = lv_checkbox_get_text(obj);
      const char * state = lv_obj_get_state(obj) & LV_STATE_CHECKED ? "Checked" : "Unchecked";
      Serial.println(state);
  }
}

static void checkbox2(lv_event_t * e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t * obj = lv_event_get_target(e);
  if(code == LV_EVENT_VALUE_CHANGED) {
      //const char * txt = lv_checkbox_get_text(obj);
      const char * state = lv_obj_get_state(obj) & LV_STATE_CHECKED ? "Checked" : "Unchecked";
      Serial.println(state);
  }
}


static void no_checkbox_func(lv_event_t * e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t * obj = lv_event_get_target(e);
  if (code == LV_EVENT_VALUE_CHANGED)
  {
    Serial.println("No function defined for this checkbox!");
  }
}

fptr find_checkbox_cb(int itemIndex) {
  // Return 
  switch(itemIndex) {
    case 1:
      return checkbox1;
    case 2:
      return checkbox2;
  }

  // Default function return, if no callback function is defined for 
  return no_checkbox_func;
}



static void slider1(lv_event_t * e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t * slider = lv_event_get_target(e);
  if (code == LV_EVENT_VALUE_CHANGED)
  {
    char buf[8];
    lv_snprintf(buf, sizeof(buf), "%d%%", (int)lv_slider_get_value(slider));
    Serial.println(buf);
  }
}

static void slider2(lv_event_t * e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t * slider = lv_event_get_target(e);
  if (code == LV_EVENT_VALUE_CHANGED)
  {
    char buf[8];
    lv_snprintf(buf, sizeof(buf), "%d%%", (int)lv_slider_get_value(slider));
    Serial.println(buf);
  }
}

static void no_slider_func(lv_event_t * e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t * obj = lv_event_get_target(e);
  if (code == LV_EVENT_VALUE_CHANGED)
  {
    Serial.println("No function defined for this slider!");
  }
}

fptr find_slider_cb(int itemIndex) {
  // Return 
  switch(itemIndex) {
    case 1:
      return slider1;
    case 2:
      return slider2;
  }

  // Default function return, if no callback function is defined for 
  return no_slider_func;
}

void mgui_render(char json[])
{
  //StaticJsonDocument<5000> doc; //<sizeof json + 400>
  DynamicJsonDocument doc(8192);

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
    //Serial.println(kv.value().as<char*>());

    // Find object type
    String type = root[kv.key()]["type"]["resolvedName"];
    int itemIndex = kv.key().c_str()[strlen(kv.key().c_str())-1]  - '0';
    //Serial.println(itemIndex);

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
      buttons[numButtons] = lv_btn_create(lv_scr_act());

      lv_obj_add_event_cb(buttons[numButtons], find_button_cb(itemIndex), LV_EVENT_ALL, NULL);

      lv_obj_set_pos(buttons[numButtons], root[kv.key()]["props"]["pageX"], root[kv.key()]["props"]["pageY"]);
      lv_obj_set_height(buttons[numButtons], LV_SIZE_CONTENT);
      lv_obj_set_style_bg_color(buttons[numButtons], lv_color_make(root[kv.key()]["props"]["background"]["r"], root[kv.key()]["props"]["background"]["g"], root[kv.key()]["props"]["background"]["b"]), 0);
      
      lv_obj_t * label = lv_label_create(buttons[numButtons]);
      const char* text = root[kv.key()]["props"]["text"];
      lv_label_set_text(label, text);
      lv_obj_center(label);
      lv_obj_set_style_text_color(label, lv_color_make(root[kv.key()]["props"]["color"]["r"], root[kv.key()]["props"]["color"]["g"], root[kv.key()]["props"]["color"]["b"]), 0);
      
      numButtons++;
    }
    // If object is a switch
    else if(type.equals("Switch")) {
      switches[numSwitches] = lv_switch_create(lv_scr_act());

      lv_obj_add_event_cb(switches[numSwitches], find_switch_cb(itemIndex), LV_EVENT_ALL, NULL);

      //lv_obj_add_state(switches[numSwitches], LV_STATE_CHECKED);
      //lv_obj_add_event_cb(switches[numSwitches], counter_event_handler, LV_EVENT_ALL, NULL);
      lv_obj_set_pos(switches[numSwitches], root[kv.key()]["props"]["pageX"], root[kv.key()]["props"]["pageY"]);
      lv_obj_set_style_bg_color(switches[numSwitches], lv_color_make(188, 188, 188), LV_PART_MAIN | LV_STATE_DEFAULT);
      //lv_obj_set_style_bg_color(switches[numSwitches], lv_color_make(root[kv.key()]["props"]["color"]["r"], root[kv.key()]["props"]["color"]["g"], root[kv.key()]["props"]["color"]["b"]), LV_PART_KNOB);
      lv_obj_set_style_bg_color(switches[numSwitches], lv_color_make(root[kv.key()]["props"]["color"]["r"], root[kv.key()]["props"]["color"]["g"], root[kv.key()]["props"]["color"]["b"]), LV_PART_INDICATOR | LV_STATE_CHECKED);
    
      numSwitches++;
    }
    // If object is a slider
    else if(type.equals("Slider")) {
      sliders[numSliders] = lv_slider_create(lv_scr_act());
      lv_obj_add_event_cb(sliders[numSliders], find_slider_cb(itemIndex), LV_EVENT_VALUE_CHANGED, NULL);
      lv_obj_set_width(sliders[numSliders], root[kv.key()]["props"]["width"]);
      lv_obj_set_pos(sliders[numSliders], root[kv.key()]["props"]["pageX"], root[kv.key()]["props"]["pageY"]);
      lv_slider_set_range(sliders[numSliders], root[kv.key()]["props"]["min"], root[kv.key()]["props"]["max"]);
      lv_obj_set_style_bg_color(sliders[numSliders], lv_color_make(root[kv.key()]["props"]["color"]["r"], root[kv.key()]["props"]["color"]["g"], root[kv.key()]["props"]["color"]["b"]), LV_PART_INDICATOR);
      lv_obj_set_style_bg_color(sliders[numSliders], lv_color_make(root[kv.key()]["props"]["color"]["r"], root[kv.key()]["props"]["color"]["g"], root[kv.key()]["props"]["color"]["b"]), LV_PART_KNOB);
      
      numSliders++;
    }
    // If object is a textfield
    else if(type.equals("Textfield")) {
      textfields[numTextfields] = lv_label_create(lv_scr_act());
      lv_obj_set_pos(textfields[numTextfields], root[kv.key()]["props"]["pageX"], root[kv.key()]["props"]["pageY"]);
      const char* text = root[kv.key()]["props"]["text"];
      lv_label_set_text(textfields[numTextfields], text);
      lv_obj_set_style_text_color(textfields[numTextfields], lv_color_make(root[kv.key()]["props"]["color"]["r"], root[kv.key()]["props"]["color"]["g"], root[kv.key()]["props"]["color"]["b"]), 0);
    
      numTextfields++;
    }
    // If object is a checkbox
    else if(type.equals("Checkbox")) {
      checkboxes[numCheckboxes] = lv_checkbox_create(lv_scr_act());
      lv_obj_add_event_cb(checkboxes[numCheckboxes], find_checkbox_cb(itemIndex), LV_EVENT_ALL, NULL);
      lv_obj_set_pos(checkboxes[numCheckboxes], root[kv.key()]["props"]["pageX"], root[kv.key()]["props"]["pageY"]);
      lv_checkbox_set_text(checkboxes[numCheckboxes], "");
      lv_obj_set_style_border_color(checkboxes[numCheckboxes], lv_color_make(root[kv.key()]["props"]["color"]["r"], root[kv.key()]["props"]["color"]["g"], root[kv.key()]["props"]["color"]["b"]), LV_PART_INDICATOR);
      lv_obj_set_style_bg_color(checkboxes[numCheckboxes], lv_color_make(255, 255, 255), LV_PART_INDICATOR | LV_STATE_DEFAULT);
      lv_obj_set_style_bg_color(checkboxes[numCheckboxes], lv_color_make(root[kv.key()]["props"]["color"]["r"], root[kv.key()]["props"]["color"]["g"], root[kv.key()]["props"]["color"]["b"]), LV_PART_INDICATOR | LV_STATE_CHECKED);

      numCheckboxes++;
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
