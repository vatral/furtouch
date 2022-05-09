#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>

                                                                                                                                                  const char* ssid = "vt";
                                                                                                                                                  const char* password = "UpbsOelOskmOeuVzzNn:";

WebServer server(80);

const int led = 13;

const int PWM_FREQ       = 1000; // Hz
const int PWM_RESOLUTION = 16;   // bits

const int LEFT_EAR_CHANNEL  = 0;
const int RIGHT_EAR_CHANNEL = 1;
const int TAIL_CHANNEL      = 2;
const int CHANNEL_COUNT     = 3;

const int LEFT_EAR_PIN      = 4;
const int RIGHT_EAR_PIN     = 16;
const int TAIL_PIN          = 17;



unsigned long disable_channel_at[] = {0,0,0};

void handleRoot() {
  
  digitalWrite(led, 1);
  server.send(200, "text/plain", "FurTouch is ready!");
  digitalWrite(led, 0);
}

void handleLeftEar() {
  handleVibrate(LEFT_EAR_CHANNEL);
}

void handleRightEar() {
  handleVibrate(RIGHT_EAR_CHANNEL);
}

void handleTail() {
  handleVibrate(TAIL_CHANNEL);
}

void handleVibrate(int channel) {
  int duration = 0;
  float strength = 0.0;

  duration = server.arg("duration").toInt();
  strength = server.arg("strength").toFloat();
  
  
  
  Serial.print("Triggering motor on channel ");
  Serial.print(channel);
  Serial.print(" with strength ");
  Serial.print(strength);
  Serial.print(" for ");
  Serial.print(duration);
  Serial.println(" ms.");

  server.send(200, "text/plain", "OK");

  ledcWrite(channel, 65535 * strength);
  disable_channel_at[channel] = millis() + duration;
    
}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void setup(void) {
  ledcSetup(LEFT_EAR_CHANNEL , PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(RIGHT_EAR_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(TAIL_CHANNEL     , PWM_FREQ, PWM_RESOLUTION);

  ledcAttachPin(LEFT_EAR_PIN , LEFT_EAR_CHANNEL);
  ledcAttachPin(RIGHT_EAR_PIN, RIGHT_EAR_CHANNEL);
  ledcAttachPin(TAIL_PIN     , TAIL_CHANNEL);
  
  
  
  
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);

  
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Connected to WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.on("/vibrate/left_ear", handleLeftEar);
  server.on("/vibrate/right_ear", handleRightEar);
  server.on("/vibrate/tail", handleTail);
  

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();

  long int cur_time = millis();
  
  for(int channel=0;channel<CHANNEL_COUNT;channel++) {
    if ( disable_channel_at[channel] == 0 ) {
      continue;
    }
    
    if ( cur_time >= disable_channel_at[channel] ) {
      ledcWrite(channel, 0);
      disable_channel_at[channel] = 0;
    }
  }
  
}
