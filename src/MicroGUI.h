//
//   Library for making MicroGUI functional on embedded displays
//
//   Utilized libraries: LovyanGFX, LVGL, ArduinoJSON, LinkedList
// 
//   written by Linus Johansson 2022 for MicroGUI
//

#ifndef MicroGUI_h
#define MicroGUI_h

#include <lvgl.h>
#include "lv_conf.h"

#include <asyncHTTPrequest.h>

#include <ArduinoJson.h>
#include <ArduinoJson.hpp>

/* MicroGUI object class */
class MGUI_object {
  private:
    lv_obj_t * object;
    char parent_id[100]={0};
    char event_id[100]={0};
    char type[30]={0};
    
  public:
    MGUI_object(lv_obj_t * obj, const char * obj_type, const char * obj_name, const char * obj_event);
    MGUI_object();
    void setObject(lv_obj_t * obj);
    lv_obj_t * getObject();
    char* getEvent();
    char* getParent();
    char* getType();
};

/* MicroGUI event class */
class MGUI_event {
  private:
    char event_id[100]={0};
    char parent_id[100]={0};
    int value;
  public:
    MGUI_event(const char * event, const char * parent, int val);
    MGUI_event();
    char* getEvent();
    char* getParent();
    int getValue();
};

/* Display orientations */
typedef enum {
  MGUI_PORTRAIT,
  MGUI_LANDSCAPE,
  MGUI_PORTRAIT_FLIPPED,
  MGUI_LANDSCAPE_FLIPPED  
}MGUI_orientation;

/* Variables used in MicroGUI Core and extensions */

extern MGUI_event * latest;
extern bool new_event;
extern char document[];
extern bool from_persistant;

/* MicroGUI functions */

void mgui_init();
void mgui_init(char json[]);
void mgui_init(char json[], int rotation);

void mgui_render(char json[]);

MGUI_event * mgui_run();

void mgui_update_doc();

void mgui_show_border();
void mgui_hide_border();

bool mgui_compare(const char * string1, const char * string2);

// Set value/text of components. Return void for now but may want to return error in the future??

void mgui_set_value(const char * obj_name, int value, bool send);
void mgui_set_value(const char * obj_name, int value);

void mgui_set_text(const char * obj_name, const char * text, bool send);
void mgui_set_text(const char * obj_name, const char * text);

int mgui_get_value(const char * obj_name);

// API functions /* Incomplete as of now */

void callAPI(char * url, int value);

void sendRequest(char * url);

void requestCB(void* optParm, asyncHTTPrequest *request, int readyState);

#endif