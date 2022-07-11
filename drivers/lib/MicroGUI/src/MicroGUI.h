//
//   Library for making MicroGUI functional on embedded displays
//
//   Utilized libraries: LovyanGFX, LVGL, ArduinoJSON, LinkedList
// 
//   written by Linus Johansson, 2022 @ Plejd
//

#ifndef MicroGUI_h
#define MicroGUI_h

#include <lvgl.h>
#include "lv_conf.h"

#include <ArduinoJson.h>
#include <ArduinoJson.hpp>

/* MicroGUI object class */
class MGUI_object {
  private:
    lv_obj_t * object;
    const char * name;
    const char * event;
    // Store object type e.g button / slider
    // redo set_value/set_text
    
  public:
    MGUI_object(lv_obj_t * obj, const char * obj_name, const char * obj_event);
    lv_obj_t * getObject();
    const char* getEvent();
    const char* getParent();
    int getValue();
    bool getState();
};

/* MicroGUI event class */
class MGUI_event {
  private:
    const char * event_id;
    const char * parent_id;
    int value;
    //bool state;
  public:
    MGUI_event(const char * event, const char * parent, int val);
    MGUI_event();
    // Either getter functions like below or make class variables like event_id public
    const char* getEvent();
    const char* getParent();
    int getValue();
    bool getState();
};

typedef enum {
  MGUI_PORTRAIT,
  MGUI_LANDSCAPE,
  MGUI_PORTRAIT_FLIPPED,
  MGUI_LANDSCAPE_FLIPPED  
}MGUI_orientation;

extern MGUI_event latest;
extern bool newEvent;

/* MicroGUI functions */
void mgui_parse(char json[]);
void mgui_init(char json[]);
void mgui_init(char json[], int rotation);
void mgui_render(char json[]);
MGUI_event mgui_run();

void mgui_render_canvas(JsonPair kv, JsonObject root);
void mgui_render_button(JsonPair kv, JsonObject root);
void mgui_render_switch(JsonPair kv, JsonObject root);
void mgui_render_slider(JsonPair kv, JsonObject root);
void mgui_render_checkbox(JsonPair kv, JsonObject root);
void mgui_render_textfield(JsonPair kv, JsonObject root);

// Return void for now, but may want to return error in the future??
void mgui_set_value(const char * obj_name, int value);
void mgui_set_text(const char * obj_name, const char * text);

bool mgui_compare(const char * string1, const char * string2);

/* Display functions */
void display_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
void touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data);

void lv_port_indev_init(void);

#endif