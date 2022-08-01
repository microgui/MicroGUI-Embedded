// COPY THE CODE BELOW INTO YOUR OWN MAIN SKETCH

// MicroGUI example_2
// GUI created for portrait orientation

// Made by Linus Johansson
// 1 August 2022

#include <MicroGUI.h>

// Store your MicroGUI in this json array
char json[] = "{\"ROOT\":{\"type\":{\"resolvedName\":\"CanvasArea\"},\"isCanvas\":true,\"props\":{\"id\":\"canvasElement\",\"width\":320,\"height\":480,\"persistant\":false,\"background\":{\"r\":184,\"g\":233,\"b\":134,\"a\":1}},\"displayName\":\"Canvas\",\"custom\":{},\"hidden\":false,\"nodes\":[\"Textfield_1\",\"Switch_1\",\"Switch_2\",\"Button_1\",\"Slider_1\",\"Checkbox_1\",\"Textfield_2\",\"Textfield_3\"],\"linkedNodes\":{}},\"Textfield_1\":{\"type\":{\"resolvedName\":\"Textfield\"},\"isCanvas\":false,\"props\":{\"text\":\"Portrait orientation\",\"fontSize\":15,\"textAlign\":\"left\",\"fontWeight\":500,\"width\":40,\"height\":30,\"color\":{\"r\":0,\"g\":0,\"b\":0,\"a\":1},\"pageX\":95,\"pageY\":30},\"displayName\":\"Textfield\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}},\"Switch_1\":{\"type\":{\"resolvedName\":\"Switch\"},\"isCanvas\":false,\"props\":{\"state\":false,\"size\":\"small\",\"color\":{\"r\":126,\"g\":211,\"b\":33,\"a\":1},\"event\":\"\",\"pageX\":75,\"pageY\":126},\"displayName\":\"Switch\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}},\"Switch_2\":{\"type\":{\"resolvedName\":\"Switch\"},\"isCanvas\":false,\"props\":{\"state\":true,\"size\":\"small\",\"color\":{\"r\":63,\"g\":81,\"b\":181,\"a\":1},\"event\":\"\",\"pageX\":201,\"pageY\":127},\"displayName\":\"Switch\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}},\"Button_1\":{\"type\":{\"resolvedName\":\"Button\"},\"isCanvas\":false,\"props\":{\"text\":\"HELLO\",\"size\":\"small\",\"variant\":\"contained\",\"background\":{\"r\":208,\"g\":2,\"b\":27,\"a\":1},\"color\":{\"r\":255,\"g\":255,\"b\":255,\"a\":1},\"event\":\"\",\"pageX\":129,\"pageY\":214},\"displayName\":\"Button\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}},\"Slider_1\":{\"type\":{\"resolvedName\":\"Slider\"},\"isCanvas\":false,\"props\":{\"size\":\"small\",\"width\":247,\"value\":100,\"min\":0,\"max\":100,\"color\":{\"r\":65,\"g\":117,\"b\":5,\"a\":1},\"valueLabelDisplay\":\"auto\",\"event\":\"\",\"pageX\":28,\"pageY\":412},\"displayName\":\"Slider\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}},\"Checkbox_1\":{\"type\":{\"resolvedName\":\"Checkbox\"},\"isCanvas\":false,\"props\":{\"state\":true,\"size\":\"small\",\"color\":{\"r\":63,\"g\":81,\"b\":181,\"a\":1},\"event\":\"\",\"pageX\":88,\"pageY\":340},\"displayName\":\"Checkbox\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}},\"Textfield_2\":{\"type\":{\"resolvedName\":\"Textfield\"},\"isCanvas\":false,\"props\":{\"text\":\"Wow, very cool\",\"fontSize\":15,\"textAlign\":\"left\",\"fontWeight\":500,\"width\":40,\"height\":30,\"color\":{\"r\":0,\"g\":0,\"b\":0,\"a\":1},\"pageX\":117,\"pageY\":341},\"displayName\":\"Textfield\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}},\"Textfield_3\":{\"type\":{\"resolvedName\":\"Textfield\"},\"isCanvas\":false,\"props\":{\"text\":\"MicroGUI\",\"fontSize\":15,\"textAlign\":\"left\",\"fontWeight\":500,\"width\":40,\"height\":30,\"color\":{\"r\":65,\"g\":117,\"b\":5,\"a\":1},\"pageX\":128,\"pageY\":56},\"displayName\":\"Textfield\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}}}";

void setup() {
  Serial.begin(115200);

  mgui_init(json);
}

void loop() {
  MGUI_event * latest = mgui_run();
}