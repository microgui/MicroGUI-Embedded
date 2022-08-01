// COPY THE CODE BELOW INTO YOUR OWN MAIN SKETCH

// MicroGUI example_1
// GUI containing all different types of objects
// and some stuff happening based on events

// Made by Linus Johansson
// 14 July 2022

#include <MicroGUI.h>
#include <RemoteMicroGUI.h>

// Store your MicroGUI in this json array
char json[] = "{\"ROOT\":{\"type\":{\"resolvedName\":\"CanvasArea\"},\"isCanvas\":true,\"props\":{\"id\":\"canvasElement\",\"width\":480,\"height\":320,\"persistant\":false,\"background\":{\"r\":245,\"g\":166,\"b\":35,\"a\":1}},\"displayName\":\"Canvas\",\"custom\":{},\"hidden\":false,\"nodes\":[\"Textfield_1\",\"Textfield_2\",\"Switch_1\",\"Switch_2\",\"Textfield_3\",\"Button_1\",\"Button_2\",\"Textfield_4\",\"Textfield_5\",\"Slider_1\",\"Checkbox_1\",\"Checkbox_2\",\"Textfield_6\"],\"linkedNodes\":{}},\"Textfield_1\":{\"type\":{\"resolvedName\":\"Textfield\"},\"isCanvas\":false,\"props\":{\"text\":\"This is some text\",\"fontSize\":15,\"textAlign\":\"left\",\"fontWeight\":500,\"width\":40,\"height\":30,\"color\":{\"r\":255,\"g\":255,\"b\":255,\"a\":1},\"pageX\":169,\"pageY\":33},\"displayName\":\"Textfield\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}},\"Textfield_2\":{\"type\":{\"resolvedName\":\"Textfield\"},\"isCanvas\":false,\"props\":{\"text\":\"These are\nswitches\",\"fontSize\":15,\"textAlign\":\"left\",\"fontWeight\":500,\"width\":40,\"height\":30,\"color\":{\"r\":0,\"g\":0,\"b\":0,\"a\":1},\"pageX\":35,\"pageY\":99},\"displayName\":\"Textfield\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}},\"Switch_1\":{\"type\":{\"resolvedName\":\"Switch\"},\"isCanvas\":false,\"props\":{\"state\":true,\"size\":\"small\",\"color\":{\"r\":65,\"g\":117,\"b\":5,\"a\":1},\"event\":\"flyingspaghettimonster\",\"pageX\":143,\"pageY\":84},\"displayName\":\"Switch\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}},\"Switch_2\":{\"type\":{\"resolvedName\":\"Switch\"},\"isCanvas\":false,\"props\":{\"state\":false,\"size\":\"small\",\"color\":{\"r\":63,\"g\":81,\"b\":181,\"a\":1},\"event\":\"potato\",\"pageX\":143,\"pageY\":127},\"displayName\":\"Switch\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}},\"Textfield_3\":{\"type\":{\"resolvedName\":\"Textfield\"},\"isCanvas\":false,\"props\":{\"text\":\"and these\nare buttons\",\"fontSize\":15,\"textAlign\":\"left\",\"fontWeight\":500,\"width\":40,\"height\":30,\"color\":{\"r\":0,\"g\":0,\"b\":0,\"a\":1},\"pageX\":245,\"pageY\":99},\"displayName\":\"Textfield\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}},\"Button_1\":{\"type\":{\"resolvedName\":\"Button\"},\"isCanvas\":false,\"props\":{\"text\":\"Knapp\",\"size\":\"small\",\"variant\":\"contained\",\"background\":{\"r\":90,\"g\":162,\"b\":7,\"a\":1},\"color\":{\"r\":255,\"g\":255,\"b\":255,\"a\":1},\"event\":\"snigelpuff\",\"pageX\":360,\"pageY\":81},\"displayName\":\"Button\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}},\"Button_2\":{\"type\":{\"resolvedName\":\"Button\"},\"isCanvas\":false,\"props\":{\"text\":\"Button\",\"size\":\"small\",\"variant\":\"contained\",\"background\":{\"r\":63,\"g\":81,\"b\":181,\"a\":1},\"color\":{\"r\":255,\"g\":255,\"b\":255,\"a\":1},\"event\":\"brrr\",\"pageX\":360,\"pageY\":127},\"displayName\":\"Button\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}},\"Textfield_4\":{\"type\":{\"resolvedName\":\"Textfield\"},\"isCanvas\":false,\"props\":{\"text\":\"Here we have a slider\",\"fontSize\":15,\"textAlign\":\"left\",\"fontWeight\":500,\"width\":40,\"height\":30,\"color\":{\"r\":0,\"g\":0,\"b\":0,\"a\":1},\"pageX\":50,\"pageY\":200},\"displayName\":\"Textfield\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}},\"Textfield_5\":{\"type\":{\"resolvedName\":\"Textfield\"},\"isCanvas\":false,\"props\":{\"text\":\"And last but not least, checkboxes\",\"fontSize\":15,\"textAlign\":\"left\",\"fontWeight\":500,\"width\":40,\"height\":30,\"color\":{\"r\":0,\"g\":0,\"b\":0,\"a\":1},\"pageX\":52,\"pageY\":259},\"displayName\":\"Textfield\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}},\"Slider_1\":{\"type\":{\"resolvedName\":\"Slider\"},\"isCanvas\":false,\"props\":{\"size\":\"small\",\"width\":227,\"value\":25,\"min\":0,\"max\":100,\"color\":{\"r\":63,\"g\":81,\"b\":181,\"a\":1},\"valueLabelDisplay\":\"auto\",\"event\":\"woopie\",\"pageX\":222,\"pageY\":200},\"displayName\":\"Slider\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}},\"Checkbox_1\":{\"type\":{\"resolvedName\":\"Checkbox\"},\"isCanvas\":false,\"props\":{\"state\":true,\"size\":\"small\",\"color\":{\"r\":248,\"g\":231,\"b\":28,\"a\":1},\"event\":\"lemontree\",\"pageX\":333,\"pageY\":258},\"displayName\":\"Checkbox\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}},\"Checkbox_2\":{\"type\":{\"resolvedName\":\"Checkbox\"},\"isCanvas\":false,\"props\":{\"state\":false,\"size\":\"small\",\"color\":{\"r\":65,\"g\":117,\"b\":5,\"a\":1},\"event\":\"tomato\",\"pageX\":392,\"pageY\":259},\"displayName\":\"Checkbox\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}},\"Textfield_6\":{\"type\":{\"resolvedName\":\"Textfield\"},\"isCanvas\":false,\"props\":{\"text\":\"IP: N/A\",\"fontSize\":15,\"textAlign\":\"left\",\"fontWeight\":500,\"width\":40,\"height\":30,\"color\":{\"r\":255,\"g\":255,\"b\":255,\"a\":1},\"pageX\":13,\"pageY\":9},\"displayName\":\"Textfield\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}}}";

void setup() {
  Serial.begin(115200);

  // Attach an LED to GPIO 12
  ledcSetup(0, 5000, 8);
  ledcAttachPin(12, 0);

  mgui_init(json, MGUI_LANDSCAPE_FLIPPED);

  mgui_remote_init("Textfield_6");
}

void loop() {
  MGUI_event * latest = mgui_run();
  
  if(mgui_compare(latest->getParent(), "Button_1")) {
    static uint8_t i = 0;
    mgui_set_value("Switch_2", true);
    mgui_set_value("Textfield_1", i);
    i++;

    mgui_set_value("Slider_1", 5);
    ledcWrite(0, mgui_get_value("Slider_1"));
  }
  else if(mgui_compare(latest->getParent(), "Switch_1")) {
    mgui_set_value("Switch_2", false);

    mgui_set_value("Slider_1", 200);
    ledcWrite(0, mgui_get_value("Slider_1"));
  }
  else if(mgui_compare(latest->getParent(), "Slider_1")) {
    ledcWrite(0, latest->getValue());
  }
}