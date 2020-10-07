/*
   TODO
    - Clock
    - Weather montages (rain, sun, thunder)
    - Sky montages - sunrise, sunset
    - Something with calendars
    - Something with Satellite view over your current location
      - A rough view of the current cape peninsular with weather/clouds
    - Some message while searching for WiFi
    - A startup screen - BoydOS
    - Change the clock to be able to display floating point seconds
    - Neural network to learn and play chess/checkers/tetris/snake/pong
    - Hex based maze survival game?
    - Genghis visualiser (with the bot at the centre, the rest of the world just moves round it)
    - virtual lava lamp
    - live updating map for property prices?
    - live update of some online game?
    - live update of satellites overhead/flights?
    - low res memes?
    - DVD logo thing, but it always hits the corners
    - synced with spotify
    - Add in a rotation amount to the renderer
    - Program a proper graphics renderer, with a z value to indicate depth / layers
    - Make the minecraft logo?

*/

#include <ArduinoOTA.h>       // OTA
#include <ArduinoJson.h>      // Read JSON streams
#include <ESPmDNS.h>          // OTA
#include <HTTPClient.h>       // HTTP connection
#include <NeoPixelAnimator.h> // Neopixels
#include <NeoPixelBus.h>      // Neopixels
#include <Wire.h>             // hmmm
#include <WiFi.h>             // Wifi connection
#include <WiFiUdp.h>          // OTA

#define PIXEL_PIN 4
#define PIXEL_COUNT 91
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PIXEL_COUNT, PIXEL_PIN);

const char* ssid = "MOSSAD";
const char* password =  "28Woodlands";
float h = 0.0;
float m = 0.0;
float s = 0.0;
int DAY_INTERVAL = 50;
int NIGHT_INTERVAL = 10;
int interval = DAY_INTERVAL;
long startTimer = millis();
float X_CENTER = 5.0;
float Y_CENTER = 2.5;
float ROTATION = 3 / 6 * PI;
long delta = 0;


const int hex[11][11] = {
  { 0,  1,  2,  3,  4,  5, -1, -1, -1, -1, -1 },
  { 12, 11, 10,  9,  8,  7,  6, -1, -1, -1, -1 },
  { 13, 14, 15, 16, 17, 18, 19, 20, -1, -1, -1 },
  { 29, 28, 27, 26, 25, 24, 23, 22, 21, -1, -1 },
  { 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, -1 },
  { 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40 },
  { -1, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60 },
  { -1, -1, 69, 68, 67, 66, 65, 64, 63, 62, 61 },
  { -1, -1, -1, 70, 71, 72, 73, 74, 75, 76, 77 },
  { -1, -1, -1, -1, 84, 83, 82, 81, 80, 79, 78 },
  { -1, -1, -1, -1, -1, 85, 86, 87, 88, 89, 90 }
};

const int centre_ring[7] = {35, 44, 56, 55, 46, 34, 45};

StaticJsonDocument<600> doc;

void setup() {
  HsbColor col = HsbColor(0.3, 1.0, 0.1);
  pinMode(PIXEL_PIN, OUTPUT);
  // Start up the Hexels
  strip.Begin();
  strip.Show();
  strip.SetPixelColor(0, col);
  strip.Show();
  Serial.begin(115200);
  while (!Serial); // wait for serial attach
  strip.SetPixelColor(1, col);
  strip.Show();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // wait until the connection is resolved into a pass/fail
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    col = HsbColor(0.0, 1.0, 0.1);
    strip.ClearTo(HsbColor(0.0, 1.0, 0.0));
    for (int i = 0; i < 7; i++) {
      strip.SetPixelColor(centre_ring[i], col);
      strip.Show();
      delay(500);
    }
    ESP.restart();
  }
  strip.SetPixelColor(2, col);
  strip.Show();
  ArduinoOTA.setHostname("ESP32-Hexel");
  //  ArduinoOTA.setPassword("admin");

  ArduinoOTA
  .onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else {// U_SPIFFS
      type = "filesystem";
    }
    Serial.println("Start updating " + type);
  })
  .onEnd([]() {
    Serial.println("\nEnd");
  })
  .onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  })
  .onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });

  ArduinoOTA.begin();
  strip.SetPixelColor(3, col);
  strip.Show();
  // Get and set the time

  String hms_str = get_HMS_string();
  Serial.println(hms_str);
  String hms[3] = {
    hms_str.substring(0, 2),
    hms_str.substring(3, 5),
    hms_str.substring(6, 8)
  };
  h = hms[0].toInt() % 24;
  m = hms[1].toInt() % 60;
  s = hms[2].toInt() % 60;

  strip.SetPixelColor(4, col);
  strip.Show();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

}

void loop() {
  ArduinoOTA.handle();
  interval = (8.0 < h && h < 22.0) ? DAY_INTERVAL : NIGHT_INTERVAL;
  if (millis() - startTimer > interval) {
    delta = millis() - startTimer;
    startTimer = millis();
    s += delta / 1000.0;
    //    m += delta / 60000.0;
    //    h += delta / 3600000.0;
    if (s >= 60.0) {
      s -= 60.0;
      m += 1;
    }
    if (m >= 60.0) {
      String hms_str = get_HMS_string();
      String hms[3] = {
        hms_str.substring(0, 2),
        hms_str.substring(3, 5),
        hms_str.substring(6, 8)
      };
      h = hms[0].toInt();
      m = hms[1].toInt();
      s = hms[2].toInt();
    }
    strip.ClearTo(HsbColor(0.0, 1.0, 0.0));
    drawClock(h, m, s);
//    strip.SetPixelColor(s, HsbColor(0.6, 1.0, 0.05));
//    strip.SetPixelColor(m, HsbColor(0.3, 1.0, 0.05));
//    strip.SetPixelColor(h, HsbColor(0.0, 1.0, 0.05));
    strip.Show();
  }
}

void drawClock(float h, float m, float s) {
  float brightness = (8.0 < h && h < 22.0) ? 1.0 : 0.05; // Dim the lights at night
  //  brightness = 1.0;
  HsbColor hr_col = HsbColor(0.13, 1.0, brightness);
  HsbColor mn_col = HsbColor(0.275, 1.0, brightness);
  HsbColor sc_col = HsbColor(0.99, 1.0, brightness);

  float s_frac = (s / 60.0);
  float m_frac = (m / 60.0) + s_frac / 60.0; 
  float h_frac = ((((int) h) % 12) / 12.0) + m_frac / 60.0;

  // Set the second hand
  drawLine(
    5, 2.5,
    5 + 6 * cos(s_frac * TWO_PI - HALF_PI),
    2.5 + 6 * sin(s_frac * TWO_PI - HALF_PI),
    sc_col
  );
  // Set the minute hand
  drawLine(
    5, 2.5,                                   // x0, y0
    5 + 4 * cos(m_frac * TWO_PI - HALF_PI),   // x1
    2.5 + 4 * sin(m_frac * TWO_PI - HALF_PI), // y1
    mn_col                   // color
  );
  // Set the hour hand
  drawLine(
    5, 2.5,
    5 + 2.5 * cos(h_frac * TWO_PI - HALF_PI),
    2.5 + 2.5 * sin(h_frac * TWO_PI - HALF_PI),
    hr_col
  );

  strip.SetPixelColor(45, HsbColor(0.0, 0.0, brightness));

}

//void drawPoint(float x, float y, HsbColor item_color) {
//  float s_val = s_from_xy(x, y);
//  float t_val = t_from_xy(x, y);
//
//  int s_arr[3] = { floor(round(s_val) - 0.01), round(s_val), ceil(round(s_val) + 0.01) };
//  int t_arr[3] = { floor(round(t_val) - 0.01), round(t_val), ceil(round(t_val) + 0.01) };
//
//  for (int i = 0; i < 3; i++) {
//    if (s_arr[i] < 11 && s_arr[i] > -1) {
//      for (int j = 0; j < 3; j++) {
//        if (t_arr[j] < 11 && t_arr[j] > -1) {
//          float dist = point_to_point(
//                         x_from_st(s_arr[i], t_arr[j]),
//                         y_from_st(s_arr[i], t_arr[j]),
//                         x,
//                         y
//                       );
//          float adjustment = adjustment_from_dist(dist);
//
//          HsbColor OG_color = strip.GetPixelColor(hex[s_arr[i]][t_arr[j]]);
//          HsbColor new_color = HsbColor(
//                                 item_color.H * adjustment + OG_color.H * (1 - adjustment),
//                                 1.0
////                                 item_color.S * adjustment + OG_color.S * (1 - adjustment),
//                                 item_color.B * adjustment + OG_color.B * (1 - adjustment)
//                               );
//          strip.SetPixelColor(hex[s_arr[i]][t_arr[j]], new_color);
//        }
//      }
//    }
//  }
//}

void drawLine(float x1, float y1, float x2, float y2, HsbColor item_color) {
  int s_start = floor(s_from_xy(min(x1, x2), min(y1, y2))) - 1;
  int t_start = floor(t_from_xy(min(x1, x2), min(y1, y2))) - 1;

  int s_end = ceil(s_from_xy(max(x1, x2), max(y1, y2))) + 1;
  int t_end = ceil(t_from_xy(max(x1, x2), max(y1, y2))) + 1;

  float s1 = s_from_xy(x1, y1);
  float t1 = t_from_xy(x1, y1);
  float s2 = s_from_xy(x2, y2);
  float t2 = t_from_xy(x2, y2);

  for (int s = s_start; s <= s_end; s++) {
    for (int t = t_start; t <= t_end; t++) {

      // now color the pixel at hex[s][t] according to the closest point of the line
      float dist = line_to_point(x1, y1, x2, y2, x_from_st(s, t), y_from_st(s, t));

      float adjustment = adjustment_from_dist(dist);
      if (item_color.B * adjustment > 0.01 && s >= 0 && s < 11 && t >= 0 && t < 11) {
        HsbColor OG_color = strip.GetPixelColor(hex[s][t]);
        HsbColor new_color;
        if (OG_color.B < 0.01) {
          new_color = HsbColor(
                        item_color.H,
                        item_color.S,
                        item_color.B * adjustment
                      );
        } else {
          new_color = HsbColor(
                        item_color.H * adjustment + OG_color.H * (1 - adjustment),
                        item_color.S * adjustment + OG_color.S * (1 - adjustment),
                        item_color.B * adjustment + OG_color.B * (1 - adjustment)
                      );
        }
        strip.SetPixelColor(hex[s][t], new_color);
      }
    }
  }
}

float point_to_point(float x1, float y1, float x2, float y2) {
  return sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));
}

float line_to_point(float A_x, float A_y, float B_x, float B_y, float x, float y) {
  if (A_x == B_x || A_y == B_y) {
    // Rotate by 45 degrees in order to offset horizontal/vertical lines
    // TODO is it possible to get rid of all this float arithmatic?
    float ROOT_TWO_OVER_TWO = 0.707106781;
    A_x = ROOT_TWO_OVER_TWO * (A_x - A_y);
    A_y = ROOT_TWO_OVER_TWO * (A_x + A_y);

    B_x = ROOT_TWO_OVER_TWO * (B_x - B_y);
    B_y = ROOT_TWO_OVER_TWO * (B_x + B_y);

    x = ROOT_TWO_OVER_TWO * (x - y);
    y = ROOT_TWO_OVER_TWO * (x + y);
  }
  if (A_y < B_y) { // Make sure point A is the higher of the two
    float temp = A_y;
    A_y = B_y;
    B_y = temp;

    temp = A_x;
    A_x = B_x;
    B_x = temp;
  }
  float m = (B_y - A_y) / (B_x - A_x);
  float c = A_y - m * A_x;
  float m_norm = -1 / m;
  float A_c_norm = A_y - m_norm * A_x;
  float B_c_norm = B_y - m_norm * B_x;

  if (y > (m_norm * x + A_c_norm)) {        // The point to draw is before the start of the line
    return point_to_point(A_x, A_y, x, y);
  } else if (y < (m_norm * x + B_c_norm)) { // The point to draw is after the end of the line
    return point_to_point(B_x, B_y, x, y);
  } else {
    return abs(c + m * x - y) / sqrt(pow(m, 2) + 1);
  }
}

float adjustment_from_dist(float dist) {
  // https://www.desmos.com/calculator/nlhki3mlce
  int steepness = 32;
  int flatness = 3;
  return pow(2, -steepness * abs( pow(dist, flatness)));
}

String get_HMS_string() {
  if ((WiFi.status() == WL_CONNECTED)) {
    HTTPClient http;
    http.begin("http://worldtimeapi.org/api/ip");
    int httpCode = http.GET();
    if (httpCode > 0) {
      String payload = http.getString();
      //Deserialize the JSON document
      DeserializationError error = deserializeJson(doc, payload);
      // Test if parsing succeeds.
      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return "ERROR";
      }
      String str = doc["datetime"];
      http.end(); //Free the resources
      return str.substring(11, 19);
    } else {
      http.end(); //Free the resources
      Serial.println("Error on HTTP request");
    }
    //    2020-05-22T21:52:47.608325+02:00
  }
}

void wifi() {

  //  if ((WiFi.status() == WL_CONNECTED)) { //Check the current connection status
  //    HTTPClient http;
  //    http.begin(endpoint); //Specify the URL
  //    int httpCode = http.GET();  //Make the request
  //
  //    if (httpCode > 0) { //Check for the returning code
  //        String payload = http.getString();
  //        //Deserialize the JSON document
  //        DeserializationError error = deserializeJson(doc, payload);
  //        // Test if parsing succeeds.
  //        if (error) {
  //          Serial.print(F("deserializeJson() failed: "));
  //          Serial.println(error.c_str());
  //          return;
  //        }
  //        String str = doc["datetime"];
  //        Serial.println(str);
  //        String str_h = str.substring(11,13);
  //        String str_m = str.substring(14,16);
  //        String str_s = str.substring(17,19);
  //        int h = str_h.toInt();
  //        int m = str_m.toInt();
  //        int s = str_s.toInt();
  //        clock(h, m, s);
  //        delay(1000);
  //      }
  //
  //    else {
  //      Serial.println("Error on HTTP request");
  //    }
  //    2020-05-22T21:52:47.608325+02:00

  //    http.end(); //Free the resources
  //  }
}

float s_from_xy(float x, float y) {
  // s is  the direction left to right
  //  return x;
  return s_from_xy(x, y, ROTATION);
}

float t_from_xy(float x, float y) {
  // t is  the direction diagonally up
  //  return x / 2.0 + y;
  return t_from_xy(x, y, ROTATION);

}

float x_from_st(float s, float t) {
  //  return s;
  return x_from_st(s, t, ROTATION);

}

float y_from_st(float s, float t) {
  //  return t - s / 2.0;
  return y_from_st(s, t, ROTATION);

}

float s_from_xy(float x, float y, float theta) {
  // s is  the direction left to right
  // The centre of rotation is at (x=5, y=2.5) or (s=5, t=0)
  float x_rotated = (x - X_CENTER) * cos(theta) - (y - Y_CENTER) * sin(theta) + X_CENTER;
  return x_rotated;
}

float t_from_xy(float x, float y, float theta) {
  // t is  the direction diagonally up
  float x_rotated = (x - X_CENTER) * cos(theta) - (y - Y_CENTER) * sin(theta) + X_CENTER;
  float y_rotated = (x - X_CENTER) * sin(theta) + (y - Y_CENTER) * cos(theta) + Y_CENTER;
  return x_rotated / 2.0 + y_rotated;
}

float x_from_st(float s, float t, float theta) {
  float y_rotated = t - s / 2.0;
  float x_rotated = s;
  float x = (x_rotated - X_CENTER) * cos(-theta) - (y_rotated - Y_CENTER) * sin(-theta) + X_CENTER;
  return x;
}

float y_from_st(float s, float t, float theta) {
  float y_rotated = t - s / 2.0;
  float x_rotated = s;
  float y = (x_rotated - X_CENTER) * sin(-theta) + (y_rotated - Y_CENTER) * cos(-theta) + Y_CENTER;
  return y;
}

void run_test(int DELAY) {
  Serial.println("=============LED TEST START=============");
  Serial.println("Testing individual LEDs with WHITE");
  for (int i = 0; i < PIXEL_COUNT; i++) {
    strip.ClearTo(HslColor(0, 1, 0));
    strip.SetPixelColor(i, HslColor(0.5, 0.5, 0.5));
    strip.Show();
    delay(DELAY);
  }

  Serial.println();
  Serial.print("Testing SAT 0-100: ");
  strip.ClearTo(HslColor(0, 1, 0));;
  for (float s = 0.0; s < 1.0f; s += 0.01) {
    strip.ClearTo(HslColor(0.5, s, 0.5));
    strip.Show();
    if (((int) s * 100) % 10 == 0) {
      Serial.print(s);
      Serial.print(" ");
    }
    delay(DELAY / 3);
  }

  Serial.println();
  Serial.print("Testing VAL 0-100: ");
  strip.ClearTo(HslColor(0, 1 , 0));;
  for (float l = 0.0; l < 1.0f; l += 0.01) {
    strip.ClearTo(HslColor(0.5, 0.5, l));
    strip.Show();
    if (((int) l * 100) % 10 == 0) {
      Serial.print(l);
      Serial.print(" ");
    }
    delay(DELAY / 3);
  }

  Serial.print("Testing HUE 0-360: ");
  strip.ClearTo(HslColor(0, 1, 0));;
  for (float h = 0.0; h < 1.0f; h += 0.01) {
    strip.ClearTo(HslColor(h, 1, 0.5));
    strip.Show();
    if (((int) h * 100) % 10 == 0) {
      Serial.print(h);
      Serial.print(" ");
    }
    delay(DELAY / 3);
  }

  strip.ClearTo(HslColor(0, 1, 0));;
  Serial.println();
  Serial.println("=============LED TEST END=============");

}
