//
//   Extension of MicroGUI Embedded library to enable remote monitoring and
//   control of an embedded display running MicroGUI as well as
//   remote GUI upload via the editor
//
//   Utilized libraries: AsyncTCP, ESPAsyncWebServer
// 
//   written by Linus Johansson 2022 for MicroGUI
//

#ifndef RemoteMicroGUI_h
#define RemoteMicroGUI_h

#include <Preferences.h>

extern Preferences preferences;

/* MicroGUI Remote functions */
void mgui_remote_init();
void mgui_remote_init(const char * textfield);
void mgui_remote_init(const char * ssid, const char * password);
void mgui_remote_init(const char * ssid, const char * password, const char * textfield);
bool getRemoteInit();

void mgui_send(const char * msg);

void mgui_run_captive();

bool mgui_remote_connected();

#endif