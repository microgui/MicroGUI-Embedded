/*
    Simple Demo with WT32-SC01 + LovyanGFX + LVGL8.x
*/
#define LGFX_AUTODETECT // Autodetect board
#define LGFX_USE_V1     // set to use new version of library
//#define LV_CONF_INCLUDE_SIMPLE

/* Uncomment below line to draw on screen with touch */
//#define DRAW_ON_SCREEN

#include <LovyanGFX.hpp> // main library
static LGFX lcd; // declare display variable

#include <lvgl.h>
#include "lv_conf.h"


#include <ArduinoJson.h>
#include <ArduinoJson.hpp>

/*** Setup screen resolution for LVGL ***/
static const uint16_t screenWidth = 480;
static const uint16_t screenHeight = 320;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * 10];



lv_obj_t * buttons[16];
short numButtons = 0;

lv_obj_t * switches[16];
short numSwitches = 0;

lv_obj_t * textfields[16];
short numTextfields = 0;

lv_obj_t * sliders[16];
short numSliders = 0;



// Variables for touch x,y
#ifdef DRAW_ON_SCREEN
static int32_t x, y;
#endif

/*** Function declaration ***/
void display_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
void touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data);
void lv_button_demo(void);
void parsejson(void);

void setup(void)
{

  Serial.begin(115200); /* prepare for possible serial debug */

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

  /*** Create simple label and show LVGL version ***/
  String LVGL_Arduino = "WT32-SC01 with LVGL ";
  LVGL_Arduino += String('v') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  lv_obj_t *label = lv_label_create(lv_scr_act()); // full screen as the parent
  lv_label_set_text(label, LVGL_Arduino.c_str());  // set label text
  lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 20);      // Center but 20 from the top

  //lv_button_demo();
  parsejson();
}

void loop()
{
  lv_timer_handler(); /* let the GUI do its work */
  delay(5);
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

/* Counter button event handler */
static void counter_event_handler(lv_event_t * e)
{
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *btn = lv_event_get_target(e);
  if (code == LV_EVENT_CLICKED)
  {
    static uint8_t cnt = 0;
    cnt++;

    /*Get the first child of the button which is the label and change its text*/
    lv_obj_t *label = lv_obj_get_child(btn, 0);
    lv_label_set_text_fmt(label, "Button: %d", cnt);
    LV_LOG_USER("Clicked");
    Serial.println("Clicked");
  }
}

/* Toggle button event handler */
static void toggle_event_handler(lv_event_t * e)
{
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_VALUE_CHANGED)
  {
    LV_LOG_USER("Toggled");
    Serial.println("Toggled");
  }
}

void lv_button_demo(void)
{
  lv_obj_t *label;

  // Button with counter
  lv_obj_t *btn1 = lv_btn_create(lv_scr_act());
  lv_obj_add_event_cb(btn1, counter_event_handler, LV_EVENT_ALL, NULL);

  lv_obj_set_pos(btn1, 100, 100);   /*Set its position*/
  lv_obj_set_size(btn1, 120, 50);   /*Set its size*/


  label = lv_label_create(btn1);
  lv_label_set_text(label, "Button");
  lv_obj_center(label);

  // Toggle button
  lv_obj_t *btn2 = lv_btn_create(lv_scr_act());
  lv_obj_add_event_cb(btn2, toggle_event_handler, LV_EVENT_ALL, NULL);
  lv_obj_add_flag(btn2, LV_OBJ_FLAG_CHECKABLE);
  lv_obj_set_pos(btn2, 250, 100);   /*Set its position*/
  lv_obj_set_size(btn2, 120, 50);   /*Set its size*/

  label = lv_label_create(btn2);
  lv_label_set_text(label, "Toggle Button");
  lv_obj_center(label);
}

void parsejson(void)
{
  //char json[] = "{\"ROOT\": {\"type\": {\"resolvedName\": \"CanvasArea\"},\"isCanvas\": true,\"props\": {\"id\": \"canvasElement\",\"width\": 320,\"height\": 240,\"background\": {\"r\": 0,\"g\": 192,\"b\": 80,\"a\": 1}},\"displayName\": \"Canvas\",\"custom\": {},\"hidden\": false,\"nodes\": [\"Switch_1\",\"Textfield_1\",\"Button_1\"],\"linkedNodes\": {}},\"Switch_1\": {\"type\": {\"resolvedName\": \"Switch\"},\"isCanvas\": false,\"props\": {\"size\": \"small\",\"color\": {\"r\": 63,\"g\": 81,\"b\": 181,\"a\": 1},\"pageX\": 215,\"pageY\": 130.99998474121094},\"displayName\": \"Switch\",\"custom\": {},\"parent\": \"ROOT\",\"hidden\": false,\"nodes\": [],\"linkedNodes\": {}},\"Textfield_1\": {\"type\": {\"resolvedName\": \"Textfield\"},\"isCanvas\": false,\"props\": {\"text\": \"Wow it works!\",\"fontSize\": 15,\"textAlign\": \"left\",\"fontWeight\": 500,\"width\": 40,\"height\": 30,\"color\": {\"r\": 0,\"g\": 0,\"b\": 0,\"a\": 1},\"pageX\": 106,\"pageY\": 46},\"displayName\": \"Textfield\",\"custom\": {},\"parent\": \"ROOT\",\"hidden\": false,\"nodes\": [],\"linkedNodes\": {}},\"Button_1\": {\"type\": {\"resolvedName\": \"Button\"},\"isCanvas\": false,\"props\": {\"text\": \"Button\",\"size\": \"small\",\"variant\": \"contained\",\"background\": {\"r\": 63,\"g\": 81,\"b\": 181,\"a\": 1},\"color\": {\"r\": 255,\"g\": 255,\"b\": 255,\"a\": 1},\"pageX\": 34,\"pageY\": 127.99998474121094},\"displayName\": \"Button\",\"custom\": {},\"parent\": \"ROOT\",\"hidden\": false,\"nodes\": [],\"linkedNodes\": {}}}";
  char json[] = "{\"ROOT\": {\"type\": {\"resolvedName\": \"CanvasArea\"},\"isCanvas\": true,\"props\": {\"id\": \"canvasElement\",\"width\": 320,\"height\": 240,\"background\": {\"r\": 0,\"g\": 192,\"b\": 80,\"a\": 1}},\"displayName\": \"Canvas\",\"custom\": {},\"hidden\": false,\"nodes\": [\"Switch_1\",\"Textfield_1\",\"Button_1\",\"Button_2\",\"Switch_1\",\"Switch_2\"],\"linkedNodes\": {}},\"Switch_1\": {\"type\": {\"resolvedName\": \"Switch\"},\"isCanvas\": false,\"props\": {\"size\": \"small\",\"color\": {\"r\": 63,\"g\": 81,\"b\": 181,\"a\": 1},\"pageX\": 218,\"pageY\": 126},\"displayName\": \"Switch\",\"custom\": {},\"parent\": \"ROOT\",\"hidden\": false,\"nodes\": [],\"linkedNodes\": {}},\"Textfield_1\": {\"type\": {\"resolvedName\": \"Textfield\"},\"isCanvas\": false,\"props\": {\"text\": \"Wow it works!\",\"fontSize\": 15,\"textAlign\": \"left\",\"fontWeight\": 500,\"width\": 40,\"height\": 30,\"color\": {\"r\": 0,\"g\": 0,\"b\": 0,\"a\": 1},\"pageX\": 113,\"pageY\": 48},\"displayName\": \"Textfield\",\"custom\": {},\"parent\": \"ROOT\",\"hidden\": false,\"nodes\": [],\"linkedNodes\": {}},\"Button_1\": {\"type\": {\"resolvedName\": \"Button\"},\"isCanvas\": false,\"props\": {\"text\": \"Button\",\"size\": \"small\",\"variant\": \"contained\",\"background\": {\"r\": 63,\"g\": 81,\"b\": 181,\"a\": 1},\"color\": {\"r\": 255,\"g\": 255,\"b\": 255,\"a\": 1},\"pageX\": 34,\"pageY\": 128},\"displayName\": \"Button\",\"custom\": {},\"parent\": \"ROOT\",\"hidden\": false,\"nodes\": [],\"linkedNodes\": {}},\"Button_2\": {\"type\": {\"resolvedName\": \"Button\"},\"isCanvas\": false,\"props\": {\"text\": \"HEJ! :D\",\"size\": \"small\",\"variant\": \"contained\",\"background\": {\"r\": 63,\"g\": 81,\"b\": 181,\"a\": 1},\"color\": {\"r\": 255,\"g\": 255,\"b\": 255,\"a\": 1},\"pageX\": 39,\"pageY\": 184},\"displayName\": \"Button\",\"custom\": {},\"parent\": \"ROOT\",\"hidden\": false,\"nodes\": [],\"linkedNodes\": {}},\"Switch_2\": {\"type\": {\"resolvedName\": \"Switch\"},\"isCanvas\": false,\"props\": {\"size\": \"small\",\"color\": {\"r\": 63,\"g\": 81,\"b\": 181,\"a\": 1},\"pageX\": 218,\"pageY\": 187},\"displayName\": \"Switch\",\"custom\": {},\"parent\": \"ROOT\",\"hidden\": false,\"nodes\": [],\"linkedNodes\": {}}}";
  //char json[] = "{\"ROOT\": {\"type\": {\"resolvedName\": \"CanvasArea\"},\"isCanvas\": true,\"props\": {\"id\": \"canvasElement\",\"width\": 320,\"height\": 240,\"background\": {\"r\": 255,\"g\": 255,\"b\": 255,\"a\": 1}},\"displayName\": \"Canvas\",\"custom\": {},\"hidden\": false,\"nodes\": [\"Slider_1\",\"Slider_2\"],\"linkedNodes\": {}},\"Slider_1\": {\"type\": {\"resolvedName\": \"Slider\"},\"isCanvas\": false,\"props\": {\"size\": \"small\",\"width\": 174,\"value\": 61,\"min\": 0,\"max\": 100,\"color\": {\"r\": 63,\"g\": 81,\"b\": 181,\"a\": 1},\"valueLabelDisplay\": \"auto\",\"pageX\": 63,\"pageY\": 72},\"displayName\": \"Slider\",\"custom\": {},\"parent\": \"ROOT\",\"hidden\": false,\"nodes\": [],\"linkedNodes\": {}},\"Slider_2\": {\"type\": {\"resolvedName\": \"Slider\"},\"isCanvas\": false,\"props\": {\"size\": \"small\",\"width\": 100,\"value\": 71,\"min\": 0,\"max\": 100,\"color\": {\"r\": 63,\"g\": 81,\"b\": 181,\"a\": 1},\"valueLabelDisplay\": \"auto\",\"pageX\": 98,\"pageY\": 146.99998474121094},\"displayName\": \"Slider\",\"custom\": {},\"parent\": \"ROOT\",\"hidden\": false,\"nodes\": [],\"linkedNodes\": {}}}";

  StaticJsonDocument<sizeof json + 400> doc;

  DeserializationError error = deserializeJson(doc, json);

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  JsonObject root = doc.as<JsonObject>();

  // List all keys
  for (JsonPair kv : root) {
    Serial.println(kv.key().c_str());
    //Serial.println(kv.value().as<char*>());

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
      buttons[numButtons] = lv_btn_create(lv_scr_act());
      //lv_obj_add_event_cb(buttons[numButtons], event_handler, LV_EVENT_ALL, NULL);
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
      //lv_obj_add_state(switches[numSwitches], LV_STATE_CHECKED);
      //lv_obj_add_event_cb(switches[numSwitches], event_handler, LV_EVENT_ALL, NULL);
      lv_obj_set_pos(switches[numSwitches], root[kv.key()]["props"]["pageX"], root[kv.key()]["props"]["pageY"]);
    
      numSwitches++;
    }
    // If object is a slider
    else if(type.equals("Slider")) {
      sliders[numSliders] = lv_slider_create(lv_scr_act());
      //lv_obj_set_event_cb(slider, slider_event_cb);
      lv_obj_set_width(sliders[numSliders], root[kv.key()]["props"]["width"]);
      lv_obj_set_pos(sliders[numSliders], root[kv.key()]["props"]["pageX"], root[kv.key()]["props"]["pageY"]);
      lv_slider_set_range(sliders[numSliders], root[kv.key()]["props"]["min"], root[kv.key()]["props"]["max"]);
      lv_obj_set_style_bg_color(sliders[numSliders], lv_color_make(root[kv.key()]["props"]["color"]["r"], root[kv.key()]["props"]["color"]["g"], root[kv.key()]["props"]["color"]["b"]), 0);
      
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

    }
  }
}