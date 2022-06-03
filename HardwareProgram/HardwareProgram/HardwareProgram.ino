/*
  ESP32-CAM Base64
  Author : ChungYi Fu (Kaohsiung, Taiwan)  2019-8-14 21:00
  https://www.facebook.com/francefu
*/
#include <WiFi.h>
#include <HTTPClient.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "Base64.h"
#include "esp_camera.h"



// variabls 
const int trigPin = 12;
const int echoPin = 4;

long duration;
float distanceCm;
#define SOUND_SPEED 0.034  //define sound speed in cm/uS

boolean SlotNewStatus;
boolean SlotPreviousStatus;
boolean temp;

unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 5 seconds (5000)
unsigned long timerDelay = 5000;

const char* ssid = "hessa";
const char* password = "77777777";

// Domain Name with full URL Path for HTTP POST Request
 String serverPath = "https://smartparkingsystem-api-pd3.conveyor.cloud/api/Spaces/VacantParkingSpace/s5/5";
 String url = "https://smartparkingsystem-api-pd3.conveyor.cloud/api/Spaces/PostImageBase64";
WiFiServer server(443);



// WARNING!!! Make sure that you have either selected ESP32 Wrover Module,
//            or another board which has PSRAM enabled

//CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22



String Photo2Base64();

void setup(){
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

   Serial.begin(115200); // Starts the serial communication
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input


  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  delay(10);

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
  //init with high specs to pre-allocate larger buffers
  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;  //0-63 lower number means higher quality
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;  //0-63 lower number means higher quality
    config.fb_count = 1;
  }

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    delay(1000);
    ESP.restart();
  }

  //drop down frame size for higher initial frame rate
  sensor_t * s = esp_camera_sensor_get(); +
  s->set_framesize(s, FRAMESIZE_QQVGA);  // UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA
  server.begin();
  
    SlotPreviousStatus = true;
}

void loop(){
  delay(3000); 
  digitalWrite(trigPin, LOW);   // Clears the trigPin
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);   // Sets the trigPin on HIGH state for 10 micro seconds
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  
    duration = pulseIn(echoPin, HIGH);   // Reads the echoPin, returns the sound wave travel time in microseconds
  distanceCm = duration * SOUND_SPEED/2;  // Calculate the distance

  Serial.println( distanceCm );

if(distanceCm > 5) {
 SlotNewStatus = true;
  Serial.print( distanceCm );
  Serial.println ("   : more than 5 ");
  
     if (SlotNewStatus != SlotPreviousStatus){
      
  //Send an HTTP POST request every 10 minutes
  if ((millis() - lastTime) > timerDelay) {
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;
   //   String serverPath = "https://smartparkingsystem-api.conveyor.cloud/api/Spaces/VacantParkingSpace/s1/1";
      
      // Your Domain name with URL path or IP address with path
      http.begin(serverPath.c_str());

http.addHeader("Content-Type", "application/json");
int httpResponseCode = http.PUT("{\"SpaceNumber\":\"s5\",\"ParkingId\":\"5\"}");

      if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String payload = http.getString();
        Serial.println(payload);
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}
}
  else if (distanceCm < 5) { 
  Serial.print( distanceCm );
  Serial.print("   : less than 5");
    SlotNewStatus = false;
    
     if (SlotNewStatus != SlotPreviousStatus){
  
  if ((millis() - lastTime) > timerDelay) {

    if (WiFi.status() == WL_CONNECTED) //Check WiFi connection status {
    
      WiFiClient client = server.available();
      HTTPClient http;
      // Your Domain name with URL path or IP address with path
   //  String url = "https://smartparkingsystem-api.conveyor.cloud/api/Spaces/PostImageBase64";
      http.begin(url);
      //  HTTP request with a content type: application/json, use the following:
      http.addHeader("Content-Type", "application/json");
      Serial.println();
      // JSON data(image and parking id) to send with HTTP POST
      int httpCode = http.POST("{\"image\":\"" + Photo2Base64() + "\",\"SpaceNumber\":\"s5\",\"ParkingId\":\"5\"}");
   if (httpCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpCode);
        String payload = http.getString();
        Serial.println(payload);
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpCode);
      }
      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
 } 
   temp = SlotNewStatus;
   SlotNewStatus = SlotPreviousStatus;
   SlotPreviousStatus = temp;
    
    delay(1000);
}


String Photo2Base64() {
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return "";
  }
      Serial.println("Camera capture");

  String imageFile = "data:image/jpeg;base64,";
  char *input = (char *)fb->buf;
  char output[base64_enc_len(3)];
  for (int i = 0; i < fb->len; i++) {
    base64_encode(output, (input++), 3);
    if (i % 3 == 0) imageFile += urlencode(String(output));
  }

  esp_camera_fb_return(fb);
  return imageFile;
}

//https://github.com/zenmanenergy/ESP8266-Arduino-Examples/
String urlencode(String str){
  String encodedString = "";
  char c;
  char code0;
  char code1;
  char code2;
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (c == ' ') {
      encodedString += '+';
    } else if (isalnum(c)) {
      encodedString += c;
    } else {
      code1 = (c & 0xf) + '0';
      if ((c & 0xf) > 9) {
        code1 = (c & 0xf) - 10 + 'A';
      }
      c = (c >> 4) & 0xf;
      code0 = c + '0';
      if (c > 9) {
        code0 = c - 10 + 'A';
      }
      code2 = '\0';
      encodedString += '%';
      encodedString += code0;
      encodedString += code1;
      //encodedString+=code2;
    }
    yield();
  }
  return encodedString;
}
