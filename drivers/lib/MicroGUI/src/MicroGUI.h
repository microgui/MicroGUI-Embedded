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

#ifndef MicroGUI_h
#define MicroGUI_h

#include <lvgl.h>
#include "lv_conf.h"

#include <ArduinoJson.h>
#include <ArduinoJson.hpp>

void mgui_init(char json[]);
void mgui_render(char json[]);


/*** Function declaration ***/
void display_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
void touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data);

// Custom return type for finding callback function when generating widgets
typedef void (*fptr)(lv_event_t * e);

fptr find_button_cb(char itemIndex);
fptr find_switch_cb(char itemIndex);
fptr find_slider_cb(char itemIndex);
fptr find_checkbox_cb(char itemIndex);

class MGUI_event {
  private:
    lv_obj_t * object;
    int value;

  public:
    MGUI_event(lv_obj_t * obj, int val);
    MGUI_event(lv_obj_t * obj);
    MGUI_event();
    lv_obj_t* getObject();
    char* getName();
    bool getState();
    int getValue();
};

MGUI_event mgui_run();

#endif