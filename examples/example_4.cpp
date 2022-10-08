// COPY THE CODE BELOW INTO YOUR OWN MAIN SKETCH

// MicroGUI example_4
// Simple home automation interface

// Made by Linus Johansson
// 8 October 2022

#include <MicroGUI.h>

// Store your MicroGUI in this json array
char json[] = "{\"ROOT\":{\"type\":{\"resolvedName\":\"CanvasArea\"},\"isCanvas\":true,\"props\":{\"id\":\"canvasElement\",\"width\":480,\"height\":320,\"persistant\":false,\"background\":{\"r\":255,\"g\":255,\"b\":255,\"a\":1}},\"displayName\":\"Canvas\",\"custom\":{},\"hidden\":false,\"nodes\":[\"Divider_1\",\"Textfield_1\",\"Switch_1\",\"Divider_2\",\"Divider_3\",\"Switch_2\",\"Switch_3\",\"Textfield_2\",\"Textfield_3\"],\"linkedNodes\":{}},\"Divider_1\":{\"type\":{\"resolvedName\":\"Divider\"},\"isCanvas\":false,\"props\":{\"length\":140,\"thickness\":140,\"orientation\":\"horizontal\",\"color\":{\"r\":74,\"g\":144,\"b\":226,\"a\":1},\"pageX\":11,\"pageY\":89},\"displayName\":\"Divider\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}},\"Textfield_1\":{\"type\":{\"resolvedName\":\"Textfield\"},\"isCanvas\":false,\"props\":{\"text\":\"Kitchen\",\"fontSize\":24,\"textAlign\":\"left\",\"fontWeight\":500,\"width\":40,\"height\":30,\"color\":{\"r\":0,\"g\":0,\"b\":0,\"a\":1},\"pageX\":36,\"pageY\":111},\"displayName\":\"Textfield\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}},\"Switch_1\":{\"type\":{\"resolvedName\":\"Switch\"},\"isCanvas\":false,\"props\":{\"state\":false,\"size\":\"medium\",\"color\":{\"r\":126,\"g\":211,\"b\":33,\"a\":1},\"event\":\"\",\"pageX\":52,\"pageY\":170},\"displayName\":\"Switch\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}},\"Divider_2\":{\"type\":{\"resolvedName\":\"Divider\"},\"isCanvas\":false,\"props\":{\"length\":140,\"thickness\":140,\"orientation\":\"horizontal\",\"color\":{\"r\":126,\"g\":211,\"b\":33,\"a\":1},\"pageX\":170,\"pageY\":89},\"displayName\":\"Divider\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}},\"Divider_3\":{\"type\":{\"resolvedName\":\"Divider\"},\"isCanvas\":false,\"props\":{\"length\":140,\"thickness\":140,\"orientation\":\"horizontal\",\"color\":{\"r\":65,\"g\":117,\"b\":5,\"a\":1},\"pageX\":327,\"pageY\":89},\"displayName\":\"Divider\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}},\"Switch_2\":{\"type\":{\"resolvedName\":\"Switch\"},\"isCanvas\":false,\"props\":{\"state\":true,\"size\":\"medium\",\"color\":{\"r\":208,\"g\":2,\"b\":27,\"a\":1},\"event\":\"\",\"pageX\":210,\"pageY\":170},\"displayName\":\"Switch\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}},\"Switch_3\":{\"type\":{\"resolvedName\":\"Switch\"},\"isCanvas\":false,\"props\":{\"state\":true,\"size\":\"medium\",\"color\":{\"r\":248,\"g\":231,\"b\":28,\"a\":1},\"event\":\"\",\"pageX\":367,\"pageY\":170},\"displayName\":\"Switch\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}},\"Textfield_2\":{\"type\":{\"resolvedName\":\"Textfield\"},\"isCanvas\":false,\"props\":{\"text\":\"Living\nroom\",\"fontSize\":24,\"textAlign\":\"left\",\"fontWeight\":500,\"width\":40,\"height\":30,\"color\":{\"r\":0,\"g\":0,\"b\":0,\"a\":1},\"pageX\":204,\"pageY\":99},\"displayName\":\"Textfield\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}},\"Textfield_3\":{\"type\":{\"resolvedName\":\"Textfield\"},\"isCanvas\":false,\"props\":{\"text\":\"Bedroom\",\"fontSize\":24,\"textAlign\":\"left\",\"fontWeight\":500,\"width\":40,\"height\":30,\"color\":{\"r\":0,\"g\":0,\"b\":0,\"a\":1},\"pageX\":342,\"pageY\":109},\"displayName\":\"Textfield\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}}}";

void setup() {
  Serial.begin(115200);

  mgui_init(json);
}

void loop() {
  MGUI_event * latest = mgui_run();
}