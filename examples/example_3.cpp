// COPY THE CODE BELOW INTO YOUR OWN MAIN SKETCH

// MicroGUI example_3
// GUI containing all different types of objects
// and some stuff happening based on events

// Made by Linus Johansson
// 4 August 2022

#include <MicroGUI.h>
#include <RemoteMicroGUI.h>

// Store your MicroGUI in this json array
char json[] = "{\"ROOT\":{\"type\":{\"resolvedName\":\"CanvasArea\"},\"isCanvas\":true,\"props\":{\"id\":\"canvasElement\",\"width\":480,\"height\":320,\"persistant\":false,\"background\":{\"r\":184,\"g\":233,\"b\":134,\"a\":1}},\"displayName\":\"Canvas\",\"custom\":{},\"hidden\":false,\"nodes\":[\"Switch_1\",\"Textfield_1\",\"Textfield_2\",\"Slider_1\",\"Textfield_3\"],\"linkedNodes\":{}},\"Switch_1\":{\"type\":{\"resolvedName\":\"Switch\"},\"isCanvas\":false,\"props\":{\"state\":false,\"size\":\"small\",\"color\":{\"r\":63,\"g\":81,\"b\":181,\"a\":1},\"event\":\"LED\",\"pageX\":108,\"pageY\":185},\"displayName\":\"Switch\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}},\"Textfield_1\":{\"type\":{\"resolvedName\":\"Textfield\"},\"isCanvas\":false,\"props\":{\"text\":\"LED Control Panel\",\"fontSize\":15,\"textAlign\":\"left\",\"fontWeight\":500,\"width\":40,\"height\":30,\"color\":{\"r\":0,\"g\":0,\"b\":0,\"a\":1},\"pageX\":177,\"pageY\":79},\"displayName\":\"Textfield\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}},\"Textfield_2\":{\"type\":{\"resolvedName\":\"Textfield\"},\"isCanvas\":false,\"props\":{\"text\":\"OFF / ON\",\"fontSize\":15,\"textAlign\":\"left\",\"fontWeight\":500,\"width\":40,\"height\":30,\"color\":{\"r\":0,\"g\":0,\"b\":0,\"a\":1},\"pageX\":94,\"pageY\":154},\"displayName\":\"Textfield\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}},\"Slider_1\":{\"type\":{\"resolvedName\":\"Slider\"},\"isCanvas\":false,\"props\":{\"size\":\"small\",\"width\":182,\"value\":0,\"min\":0,\"max\":100,\"color\":{\"r\":63,\"g\":81,\"b\":181,\"a\":1},\"valueLabelDisplay\":\"auto\",\"event\":\"LED\",\"pageX\":218,\"pageY\":181},\"displayName\":\"Slider\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}},\"Textfield_3\":{\"type\":{\"resolvedName\":\"Textfield\"},\"isCanvas\":false,\"props\":{\"text\":\"Brightness\",\"fontSize\":15,\"textAlign\":\"left\",\"fontWeight\":500,\"width\":40,\"height\":30,\"color\":{\"r\":0,\"g\":0,\"b\":0,\"a\":1},\"pageX\":292,\"pageY\":154},\"displayName\":\"Textfield\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}}}";

bool LED_state = false;
int LED_duty = 0;

void setup() {
  Serial.begin(115200);

  ledcSetup(0, 5000, 8);
  ledcAttachPin(12, 0);

  mgui_init();

  mgui_remote_init();
}

void loop() {
  MGUI_event * latest = mgui_run();
  
  if(mgui_compare(latest->getParent(), "Switch_1")) 
  {
    LED_state = latest->getValue();
  }
  else if(mgui_compare(latest->getParent(), "Slider_1")) 
  {
    LED_duty = latest->getValue();    
  }
  if(LED_state == true)
  {
    analogWrite(12, LED_duty);
  }
  else {
    analogWrite(12, 0);
  }
}