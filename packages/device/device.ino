#include "esp_camera.h"
#include <WiFiClientSecure.h>
#include "config.h"
#include "secrets.h"
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>

//
// WARNING!!! PSRAM IC required for UXGA resolution and high JPEG quality
//            Ensure ESP32 Wrover Module or other board with PSRAM is selected
//            Partial images will be transmitted if image exceeds buffer size
//

// Select camera model
//#define CAMERA_MODEL_WROVER_KIT // Has PSRAM
//#define CAMERA_MODEL_ESP_EYE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_PSRAM // Has PSRAM
//#define CAMERA_MODEL_M5STACK_V2_PSRAM // M5Camera version B Has PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_ESP32CAM // No PSRAM
#define CAMERA_MODEL_AI_THINKER // Has PSRAM
//#define CAMERA_MODEL_TTGO_T_JOURNAL // No PSRAM

#include "camera_pins.h"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

// The MQTT topics that this device should publish/subscribe
#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"

WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient(512);

void startCameraServer();

void messageHandler(String &topic, String &payload) {
  Serial.println("INFO: Incoming message");
  Serial.println("INFO: Incoming: " + topic + " - " + payload);

  if(topic == "$aws/things/" + String(AWS_IOT_THING_NAME) + "/shadow/update/delta") {
    Serial.println("INFO: Shadow delta received");
    DynamicJsonDocument delta(512);
    deserializeJson(delta, payload);

    updateShadow(delta["state"]);
  } else {
    Serial.println("I don't really know what to do with this message");
  }

//  DynamicJsonDocument doc;
//  deserializeJson(doc, payload);
//  const char* message = doc["message"];
}

void connectAWS()
{
  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.begin(AWS_IOT_ENDPOINT, 8883, net);

  // Create a message handler
  client.onMessage(messageHandler);

  Serial.println("INFO: Connecting to AWS IOT");

  while (!client.connect(AWS_IOT_THING_NAME)) {
    Serial.print(".");
    delay(100);
  }

  if(!client.connected()){
    Serial.println("ERROR: AWS IoT Timeout!");
    return;
  }

  Serial.println("INFO: AWS IoT Connected!");

  Serial.println("INFO: Subscribing to shadow Deltas");
  client.subscribe("$aws/things/" + String(AWS_IOT_THING_NAME) + "/shadow/update/delta");
}

void publishMessage()
{
  DynamicJsonDocument doc(512);
  doc["time"] = millis();
  doc["sensor_a0"] = analogRead(0);
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client

  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}

DynamicJsonDocument getCameraConfiguration() {
  sensor_t * s = esp_camera_sensor_get();
  int pixformat = s->pixformat;
  int framesize = s->status.framesize;
  int quality = s->status.quality;
  int brightness = s->status.brightness;
  int contrast = s->status.contrast;
  int saturation = s->status.saturation;
  int sharpness = s->status.sharpness;
  int special_effect = s->status.special_effect;
  int wb_mode = s->status.wb_mode;
  int awb = s->status.awb;
  int awb_gain = s->status.awb_gain;
  int aec = s->status.aec;
  int aec2 = s->status.aec2;
  int ae_level = s->status.ae_level;
  int aec_value = s->status.aec_value;
  int agc = s->status.agc;
  int agc_gain = s->status.agc_gain;
  int gainceiling = s->status.gainceiling;
  int bpc = s->status.bpc;
  int wpc = s->status.wpc;
  int raw_gma = s->status.raw_gma;
  int lenc = s->status.lenc;
  int hmirror = s->status.hmirror;
  int dcw = s->status.dcw;
  int colorbar = s->status.colorbar;

  DynamicJsonDocument doc(512);
  doc["time"] = millis();
  doc["pixformat"] = pixformat;
  doc["framesize"] = framesize;
  doc["quality"] = quality;
  doc["brightness"] = brightness;
  doc["contrast"] = contrast;
  doc["saturation"] = saturation;
  doc["sharpness"] = sharpness;
  doc["special_effect"] = special_effect;
  doc["wb_mode"] = wb_mode;
  doc["awb"] = awb;
  doc["awb_gain"] = awb_gain;
  doc["aec"] = aec;
  doc["aec2"] = aec2;
  doc["ae_level"] = ae_level;
  doc["aec_value"] = aec_value;
  doc["agc"] = agc;
  doc["agc_gain"] = agc_gain;
  doc["gainceiling"] = gainceiling;
  doc["bpc"] = bpc;
  doc["wpc"] = wpc;
  doc["raw_gma"] = raw_gma;
  doc["lenc"] = lenc;
  doc["hmirror"] = hmirror;
  doc["dcw"] = dcw;
  doc["colorbar"] = colorbar;

  return doc;
}

DynamicJsonDocument getComponentConfiguration () {
  DynamicJsonDocument doc(512);

  doc["camera"] = getCameraConfiguration();
  doc["flashLight"] = digitalRead(4);

  return doc;
}

void updateShadow(DynamicJsonDocument delta) {
  Serial.println("INFO: Updating device based on delta");

  if (delta.containsKey("camera")) {
    Serial.println("Camera object has changed");
    
    DynamicJsonDocument cameraDelta = delta["camera"];
    sensor_t * s = esp_camera_sensor_get();

    if (cameraDelta.containsKey("pixformat")) {
      int value = cameraDelta["pixformat"];
      Serial.println("Setting pixformat to desired value");
      Serial.print(value);
      s->set_pixformat(s, cameraDelta["pixformat"]);
    }

    if (cameraDelta.containsKey("framesize")) {
      int value = cameraDelta["framesize"];
      Serial.println("Setting framesize to desired value");
      Serial.print(value);
      s->set_framesize(s, cameraDelta["framesize"]);
    }

    if (cameraDelta.containsKey("quality")) {
      int value = cameraDelta["quality"];
      Serial.println("Setting quality to desired value");
      Serial.print(value);
      s->set_quality(s, cameraDelta["quality"]);
    }

    if (cameraDelta.containsKey("brightness")) {
      int value = cameraDelta["brightness"];
      Serial.println("Setting brightness to desired value");
      Serial.print(value);
      s->set_brightness(s, cameraDelta["brightness"]);
    }

    if (cameraDelta.containsKey("contrast")) {
      int value = cameraDelta["contrast"];
      Serial.println("Setting contrast to desired value");
      Serial.print(value);
      s->set_contrast(s, cameraDelta["contrast"]);
    }

    if (cameraDelta.containsKey("saturation")) {
      int value = cameraDelta["saturation"];
      Serial.println("Setting saturation to desired value");
      Serial.print(value);
      s->set_saturation(s, cameraDelta["saturation"]);
    }

    if (cameraDelta.containsKey("sharpness")) {
      int value = cameraDelta["sharpness"];
      Serial.println("Setting sharpness to desired value");
      Serial.print(value);
      // s->set_sharpness(s, cameraDelta["sharpness"]);
    }

    if (cameraDelta.containsKey("special_effect")) {
      int value = cameraDelta["special_effect"];
      Serial.println("Setting special_effect to desired value");
      Serial.print(value);
      s->set_special_effect(s, cameraDelta["special_effect"]);
    }

    if (cameraDelta.containsKey("wb_mode")) {
      int value = cameraDelta["wb_mode"];
      Serial.println("Setting wb_mode to desired value");
      Serial.print(value);
      s->set_wb_mode(s, cameraDelta["wb_mode"]);
    }

  //  if (delta.containsKey("awb")) {
  //    s->set_awb(s, delta["awb"]);
  //  }

    if (cameraDelta.containsKey("awb_gain")) {
      int value = cameraDelta["awb_gain"];
      Serial.println("Setting awb_gain to desired value");
      Serial.print(value);
      s->set_awb_gain(s, cameraDelta["awb_gain"]);
    }

  //  if (delta.containsKey("aec")) {
  //    s->set_aec(s, delta["aec"]);
  //  }

    if (cameraDelta.containsKey("aec2")) {
      int value = cameraDelta["aec2"];
      Serial.println("Setting aec2 to desired value");
      Serial.print(value);
      s->set_aec2(s, cameraDelta["aec2"]);
    }

    if (cameraDelta.containsKey("ae_level")) {
      int value = cameraDelta["ae_level"];
      Serial.println("Setting ae_level to desired value");
      Serial.print(value);
      s->set_ae_level(s, cameraDelta["ae_level"]);
    }

    if (cameraDelta.containsKey("aec_value")) {
      int value = cameraDelta["aec_value"];
      Serial.println("Setting aec_value to desired value");
      Serial.print(value);
      // s->set_aec_value(s, cameraDelta["aec_value"]);
    }

  //  if (delta.containsKey("agc")) {
  //    s->set_agc(s, delta["agc"]);
  //  }

    if (cameraDelta.containsKey("agc_gain")) {
      int value = cameraDelta["agc_gain"];
      Serial.println("Setting agc_gain to desired value");
      Serial.print(value);
      s->set_agc_gain(s, cameraDelta["agc_gain"]);
    }

    if (cameraDelta.containsKey("gainceiling")) {
      int value = cameraDelta["gainceiling"];
      Serial.println("Setting gainceiling to desired value");
      Serial.print(value);
      s->set_gainceiling(s, cameraDelta["gainceiling"]);
    }

    if (cameraDelta.containsKey("bpc")) {
      int value = cameraDelta["bpc"];
      Serial.println("Setting bpc to desired value");
      Serial.print(value);
      s->set_bpc(s, cameraDelta["bpc"]);
    }

    if (cameraDelta.containsKey("wpc")) {
      int value = cameraDelta["wpc"];
      Serial.println("Setting wpc to desired value");
      Serial.print(value);
      s->set_wpc(s, cameraDelta["wpc"]);
    }

    if (cameraDelta.containsKey("raw_gma")) {
      int value = cameraDelta["raw_gma"];
      Serial.println("Setting raw_gma to desired value");
      Serial.print(value);
      s->set_raw_gma(s, cameraDelta["raw_gma"]);
    }

    if (cameraDelta.containsKey("lenc")) {
      int value = cameraDelta["lenc"];
      Serial.println("Setting lenc to desired value");
      Serial.print(value);
      s->set_lenc(s, cameraDelta["lenc"]);
    }

    if (cameraDelta.containsKey("hmirror")) {
      int value = cameraDelta["hmirror"];
      Serial.println("Setting hmirror to desired value");
      Serial.print(value);
      s->set_hmirror(s, cameraDelta["hmirror"]);
    }

    if (cameraDelta.containsKey("dcw")) {
      int value = cameraDelta["dcw"];
      Serial.println("Setting dcw to desired value");
      Serial.print(value);
      s->set_dcw(s, cameraDelta["dcw"]);
    }

    if (cameraDelta.containsKey("colorbar")) {
      int value = cameraDelta["colorbar"];
      Serial.println("Setting colorbar to desired value");
      Serial.print(value);
      s->set_colorbar(s, cameraDelta["colorbar"]);
    }
  }

  // Flashlight
  if (delta.containsKey("flashLight")) {
    digitalWrite(4, LOW);
    
    if (delta["flashLight"] == 1) {
      digitalWrite(4, HIGH);
    } 
  }

  DynamicJsonDocument status = getComponentConfiguration();
  reportStatus(status);

  
//  s->set_brightness(s, 0);     // -2 to 2
//  s->set_contrast(s, 0);       // -2 to 2
//  s->set_saturation(s, 0);     // -2 to 2
//  s->set_special_effect(s, 0); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
//  s->set_whitebal(s, 1);       // 0 = disable , 1 = enable
//  s->set_awb_gain(s, 1);       // 0 = disable , 1 = enable
//  s->set_wb_mode(s, 0);        // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
//  s->set_exposure_ctrl(s, 1);  // 0 = disable , 1 = enable
//  s->set_aec2(s, 0);           // 0 = disable , 1 = enable
//  s->set_ae_level(s, 0);       // -2 to 2
//  s->set_aec_value(s, 300);    // 0 to 1200
//  s->set_gain_ctrl(s, 1);      // 0 = disable , 1 = enable
//  s->set_agc_gain(s, 0);       // 0 to 30
//  s->set_gainceiling(s, (gainceiling_t)0);  // 0 to 6
//  s->set_bpc(s, 0);            // 0 = disable , 1 = enable
//  s->set_wpc(s, 1);            // 0 = disable , 1 = enable
//  s->set_raw_gma(s, 1);        // 0 = disable , 1 = enable
//  s->set_lenc(s, 1);           // 0 = disable , 1 = enable
//  s->set_hmirror(s, 0);        // 0 = disable , 1 = enable
//  s->set_vflip(s, 0);          // 0 = disable , 1 = enable
//  s->set_dcw(s, 1);            // 0 = disable , 1 = enable
//  s->set_colorbar(s, 0);       // 0 = disable , 1 = enable
}

void reportStatus(DynamicJsonDocument input) {  
  char jsonBuffer[512];
  serializeJson(input, jsonBuffer); // print to client
  Serial.println(jsonBuffer);

  String shadowUpdateTopic = "$aws/things/" + String(AWS_IOT_THING_NAME) + "/shadow/update";
  String finalState = "{ \"state\": { \"reported\":" + String(jsonBuffer) + " } }";
  client.publish(shadowUpdateTopic, finalState, false, 1);
  Serial.println("INFO: Device status reported successfully");
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  Serial.println('INFO: Aftersignals prototyping device booting');
  Serial.println('INFO: Device model: Camera+PIR, Device version: v1.0');
  Serial.println('INFO: Initializing booting sequence');

  // Configure flash LED
  pinMode (4, OUTPUT);

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1); // flip it back
    s->set_brightness(s, 1); // up the brightness just a bit
    s->set_saturation(s, -2); // lower the saturation
  }
  // drop down frame size for higher initial frame rate
  s->set_framesize(s, FRAMESIZE_QVGA);

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

  Serial.println("INFO: Initializing WiFi connection");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("INFO: WiFi connected");

  startCameraServer();

  Serial.print("INFO: Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");

  Serial.println("INFO: Connecting to AWS Iot");
  connectAWS();

  Serial.println("INFO: Subscribing to shadow's Delta");
  

  Serial.println("INFO: Reporting camera status");
  DynamicJsonDocument status = getComponentConfiguration();
  reportStatus(status);
}

int it = 0;
void loop() {
  client.loop();
  
  // put your main code here, to run repeatedly:
  delay(1);

//  Serial.println("Publishing message");
//  client.publish("dummy/message", "{ \"keepalive\": 1 }");
}
