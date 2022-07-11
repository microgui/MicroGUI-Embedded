#include <MicroGUI.h>
#include <RemoteMicroGUI.h>

const char * ssid = "";
const char * password = "";

// Store your MicroGUI in this json array
char json[] = "{\"ROOT\":{\"type\":{\"resolvedName\":\"CanvasArea\"},\"isCanvas\":true,\"props\":{\"id\":\"canvasElement\",\"width\":480,\"height\":320,\"background\":{\"r\":245,\"g\":166,\"b\":35,\"a\":1}},\"displayName\":\"Canvas\",\"custom\":{},\"hidden\":false,\"nodes\":[\"Textfield_1\",\"Switch_1\",\"Textfield_2\",\"Button_1\",\"Textfield_3\",\"Slider_1\",\"Textfield_4\",\"Checkbox_1\",\"Textfield_5\",\"Switch_2\",\"Button_2\"],\"linkedNodes\":{}},\"Textfield_1\":{\"type\":{\"resolvedName\":\"Textfield\"},\"isCanvas\":false,\"props\":{\"text\":\"This is some text\",\"fontSize\":15,\"textAlign\":\"left\",\"fontWeight\":500,\"width\":40,\"height\":30,\"color\":{\"r\":255,\"g\":255,\"b\":255,\"a\":1},\"pageX\":181,\"pageY\":28},\"displayName\":\"Textfield\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}},\"Switch_1\":{\"type\":{\"resolvedName\":\"Switch\"},\"isCanvas\":false,\"props\":{\"state\":true,\"size\":\"small\",\"color\":{\"r\":63,\"g\":181,\"b\":104,\"a\":1},\"event\":\"flyingspaghetti\",\"pageX\":130,\"pageY\":81},\"displayName\":\"Switch\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}},\"Textfield_2\":{\"type\":{\"resolvedName\":\"Textfield\"},\"isCanvas\":false,\"props\":{\"text\":\"These are\nswitches\",\"fontSize\":15,\"textAlign\":\"left\",\"fontWeight\":500,\"width\":40,\"height\":30,\"color\":{\"r\":0,\"g\":0,\"b\":0,\"a\":1},\"pageX\":29,\"pageY\":90},\"displayName\":\"Textfield\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}},\"Button_1\":{\"type\":{\"resolvedName\":\"Button\"},\"isCanvas\":false,\"props\":{\"text\":\"Knapp\",\"size\":\"small\",\"variant\":\"contained\",\"background\":{\"r\":63,\"g\":181,\"b\":104,\"a\":1},\"color\":{\"r\":255,\"g\":255,\"b\":255,\"a\":1},\"event\":\"snigelpuff\",\"pageX\":368,\"pageY\":76},\"displayName\":\"Button\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}},\"Textfield_3\":{\"type\":{\"resolvedName\":\"Textfield\"},\"isCanvas\":false,\"props\":{\"text\":\"and these\nare buttons\",\"fontSize\":15,\"textAlign\":\"left\",\"fontWeight\":500,\"width\":40,\"height\":30,\"color\":{\"r\":0,\"g\":0,\"b\":0,\"a\":1},\"pageX\":242,\"pageY\":90},\"displayName\":\"Textfield\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}},\"Slider_1\":{\"type\":{\"resolvedName\":\"Slider\"},\"isCanvas\":false,\"props\":{\"size\":\"small\",\"width\":204,\"value\":34,\"min\":0,\"max\":100,\"color\":{\"r\":241,\"g\":27,\"b\":245,\"a\":1},\"valueLabelDisplay\":\"auto\",\"event\":\"\",\"pageX\":238,\"pageY\":179},\"displayName\":\"Slider\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}},\"Textfield_4\":{\"type\":{\"resolvedName\":\"Textfield\"},\"isCanvas\":false,\"props\":{\"text\":\"Here we have a slider\",\"fontSize\":15,\"textAlign\":\"left\",\"fontWeight\":500,\"width\":40,\"height\":30,\"color\":{\"r\":0,\"g\":0,\"b\":0,\"a\":1},\"pageX\":31,\"pageY\":178},\"displayName\":\"Textfield\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}},\"Checkbox_1\":{\"type\":{\"resolvedName\":\"Checkbox\"},\"isCanvas\":false,\"props\":{\"state\":true,\"size\":\"small\",\"color\":{\"r\":255,\"g\":0,\"b\":0,\"a\":1},\"event\":\"\",\"pageX\":310,\"pageY\":246},\"displayName\":\"Checkbox\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}},\"Textfield_5\":{\"type\":{\"resolvedName\":\"Textfield\"},\"isCanvas\":false,\"props\":{\"text\":\"And last but not least, a checkbox\",\"fontSize\":15,\"textAlign\":\"left\",\"fontWeight\":500,\"width\":40,\"height\":30,\"color\":{\"r\":0,\"g\":0,\"b\":0,\"a\":1},\"pageX\":33,\"pageY\":246},\"displayName\":\"Textfield\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}},\"Switch_2\":{\"type\":{\"resolvedName\":\"Switch\"},\"isCanvas\":false,\"props\":{\"state\":false,\"size\":\"small\",\"color\":{\"r\":63,\"g\":81,\"b\":181,\"a\":1},\"event\":\"potato\",\"pageX\":130,\"pageY\":118},\"displayName\":\"Switch\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}},\"Button_2\":{\"type\":{\"resolvedName\":\"Button\"},\"isCanvas\":false,\"props\":{\"text\":\"Button\",\"size\":\"small\",\"variant\":\"contained\",\"background\":{\"r\":63,\"g\":81,\"b\":181,\"a\":1},\"color\":{\"r\":255,\"g\":255,\"b\":255,\"a\":1},\"event\":\"brrr\",\"pageX\":368,\"pageY\":116},\"displayName\":\"Button\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}},\"Textfield_6\":{\"type\":{\"resolvedName\":\"Textfield\"},\"isCanvas\":false,\"props\":{\"text\":\"IP: N/A\",\"fontSize\":15,\"textAlign\":\"left\",\"fontWeight\":500,\"width\":40,\"height\":30,\"color\":{\"r\":255,\"g\":255,\"b\":255,\"a\":1},\"pageX\":0,\"pageY\":0},\"displayName\":\"Textfield\",\"custom\":{},\"parent\":\"ROOT\",\"hidden\":false,\"nodes\":[],\"linkedNodes\":{}}}";

void setup()
{
  Serial.begin(115200);

  mgui_init(json, MGUI_LANDSCAPE_FLIPPED);

  mgui_remote_init(ssid, password, "Textfield_6");
}

void loop()
{
  MGUI_event latest = mgui_run();
  
  if (!mgui_compare(latest.getEvent(), "Default")) 
  {
    if (mgui_compare(latest.getParent(), "Button_1")) 
    {
      mgui_set_value("Switch_2", true);
      mgui_set_value("Slider_1", 32);
    }
    else if (mgui_compare(latest.getParent(), "Button_2")) 
    {
      mgui_set_value("Switch_2", false);
      mgui_set_value("Slider_1", 84);
    }
  }
}