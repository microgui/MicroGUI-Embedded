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
    
  public:
    MGUI_object(lv_obj_t * obj, const char * obj_name, const char * obj_event);
    const char* getEvent();
    const char* getParent();
    int getValue();
    bool getState();
};

/* New MicroGUI event class */
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

/* MicroGUI event class 
class MGUI_event {
  private:
    lv_obj_t * object;
    int value;
    char * action;

  public:
    MGUI_event(lv_obj_t * obj, int val);
    MGUI_event(lv_obj_t * obj);
    MGUI_event();
    lv_obj_t* getObject();
    char* getName();
    bool getState();
    int getValue();
    char * getAction();
};
*/

/* MicroGUI functions */
void mgui_init(char json[], int rotation);
void mgui_render(char json[]);
MGUI_event mgui_run();

/* Display functions */
void display_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
void touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data);

#endif