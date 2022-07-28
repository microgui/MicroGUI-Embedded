//
//   Extension of MicroGUI embedded library to enable remote monitoring and
//   control of an embedded display running MicroGUI
//
//   Utilized libraries: ESPAsyncWebServer
// 
//   written by Linus Johansson, 2022 @ Plejd
//

#ifndef RemoteMicroGUI_h
#define RemoteMicroGUI_h

#include <Preferences.h>

extern Preferences preferences;

/* Remote MicroGUI */
void mgui_remote_init();
void mgui_remote_init(const char * textfield);
bool getRemoteInit();

void mgui_send(const char * msg);
void mgui_run_captive();
bool mgui_remote_connected();

#endif