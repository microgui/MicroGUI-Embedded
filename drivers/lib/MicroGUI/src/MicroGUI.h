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

#define LGFX_AUTODETECT // Autodetect board
#define LGFX_USE_V1     // set to use new version of library

#include <LovyanGFX.hpp> // main library


#include <lvgl.h>
#include "lv_conf.h"

#include <ArduinoJson.h>
#include <ArduinoJson.hpp>

void mgui_init(void);
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

#endif