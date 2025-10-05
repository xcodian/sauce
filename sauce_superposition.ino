#include <WiFi.h>
#include <ArduinoOTA.h>
#include <WebServer.h>
#include <ArduinoJson.h>

const char* WIFI_SSID = "superposition";
const char* WIFI_PASS = "amongus123";

TaskHandle_t bgTaskHandle;
WebServer server(80);

void setup() {
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf("net: connecting to network \"%s\"\n", WIFI_SSID);
    delay(1000);
  }

  Serial.print("net: connected to network with ip ");
  Serial.println(WiFi.localIP());

  Serial.println("api: http server initialising");
  server.enableCORS();
  server.on("/", onIndex);
  server.on("/gamble", onGamble);
  server.on("/state", onStatus);
  server.begin();

  Serial.println("api: server started port 80");


  // Create background task to handle WiFi and OTA updates
  //                      fn     name      stk   arg  prio  handle       core
  xTaskCreatePinnedToCore(background, "bgtask", 10000, NULL, 0, &bgTaskHandle, 0);

  // Set up ketchup and hand sanitizer servo: 8 bit resolution, 200hz pwm
  ledcAttach(7, 200, 8);
}

void onIndex() {
  StaticJsonDocument<128> json;
  json["superposition"] = "v1";

  String jsonString;
  serializeJson(json, jsonString);
  server.send(200, "application/json", jsonString);
}

// 0 = Left, 100 = Right
int odds = 50;

void onStatus() {
  StaticJsonDocument<128> json;
  json["odds"] = odds;

  String jsonString;
  serializeJson(json, jsonString);
  server.send(200, "application/json", jsonString);
}

void onGamble() {
  int result = random(100);
  odds = 50;
  bool is_left = result > 50;

  StaticJsonDocument<128> json;
  json["roll"] = result;

  if (is_left) {
    json["dispensing"] = "right";
  } else {
    json["dispensing"] = "left";
  }

  json["secs_until"] = 5;

  String jsonString;
  serializeJson(json, jsonString);

  server.send(200, "application/json", jsonString);

  delay(5000);

  if (result > 50) {
    for (int i = 0; i < 3; i++) {
      servoRight();
      delay(1000);
      servoCenter();
      delay(1000);
    }
  } else {
    for (int i = 0; i < 3; i++) {
      servoLeft();
      delay(1000);
      servoCenter();
      delay(1000);
    }
  }
}

void servoCenter() {
  ledcWrite(7, 50);
}
void servoLeft() {
  ledcWrite(7, 30);
}
void servoRight() {
  ledcWrite(7, 70);
}

void loop() {
  server.handleClient();
}

void background(void* arg) {
  ArduinoOTA.onStart([]() {
    Serial.printf("ota: updating %s\n", ArduinoOTA.getCommand() == U_FLASH ? "program" : "fs");
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("\nota: finished");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("ota: progress: %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onError([&](ota_error_t error) {
    char* errorMessage = "unknown error";

    if (error == OTA_AUTH_ERROR) {
      errorMessage = "auth error";
    } else if (error == OTA_BEGIN_ERROR) {
      errorMessage = "begin error";
    } else if (error == OTA_CONNECT_ERROR) {
      errorMessage = "connect error";
    } else if (error == OTA_RECEIVE_ERROR) {
      errorMessage = "receive error";
    } else if (error == OTA_END_ERROR) {
      errorMessage = "end error";
    }

    Serial.printf("ota: error %u: %s\n", error, errorMessage);
  });

  Serial.println("ota: update server initialising");
  ArduinoOTA.begin();

  while (true) {
    ArduinoOTA.handle();
  }
}