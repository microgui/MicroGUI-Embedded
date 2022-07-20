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

/* Remote MicroGUI */
void mgui_remote_init(const char * ssid, const char * password);
void mgui_remote_init(const char * ssid, const char * password, const char * textfield);
bool getRemoteInit();
// Below is just for a quick test!!!
void mgui_send(const char * msg);

#endif