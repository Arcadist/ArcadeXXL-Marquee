#define FASTLED_ALLOW_INTERRUPTS 0
// External Libs
#include <Arduino.h>
#include <FS.h>
#include <WiFi.h>
#include <SPI.h>
#include <AsyncTCP.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>
#include <FastLED.h>
#include <FastLED_NeoMatrix.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>

// Private Libs
#include <GifDecoder.h>
#include <FilenameFunctions.h>

#include "config.h"

int num_files;
int selected_index = 0;

boolean ready = false;

File fileSave;
DNSServer dns;

extern StringArray fileList;

const uint8_t kMatrixWidth = MATRIX_WIDTH;
const uint8_t kMatrixHeight = MATRIX_HEIGHT;

const String upload_directory = String(GIF_DIRECTORY) + "/";

#define NUMMATRIX (kMatrixWidth * kMatrixHeight)

CRGB leds[NUMMATRIX];

FastLED_NeoMatrix *matrix = new FastLED_NeoMatrix(leds, kMatrixWidth, kMatrixHeight, MATRIX_TILE_H, MATRIX_TILE_V, MATRIX_LAYOUT);

GifDecoder<kMatrixWidth, kMatrixHeight, 12> decoder;

AsyncWebServer server(80);
AsyncWiFiManager wifiManager(&server, &dns);

static void handle_update_progress_cb(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{

  if (!index)
  {
    Serial.println("Begin upload");
    fileSave = SPIFFS.open(upload_directory + filename, FILE_WRITE);
  }

  if (fileSave.write(data, len) != len)
  {
    Serial.println("len error");
  }

  if (final)
  {
    Serial.println("done");
    fileSave.close();
    num_files = enumerateGIFFiles(GIF_DIRECTORY, false);
  }
}

void drawPixelCallback(int16_t x, int16_t y, uint8_t red, uint8_t green, uint8_t blue)
{
  matrix->drawPixel(x, y, {red, green, blue});
}

void screenClearCallback(void)
{
  matrix->clear();
}

void updateScreenCallback(void)
{
  matrix->show();
}

void matrix_clear()
{
  // clear does not work properly with multiple matrices connected via parallel inputs
  memset(leds, 0, sizeof(leds));
}

void display_scrollText(String text)
{
  int16_t textWidth = text.length() * kMatrixWidth;

  matrix_clear();
  matrix->setTextWrap(false); // we don't wrap text so it scrolls nicely
  matrix->setTextSize(1);
  matrix->setRotation(MATRIX_ROTATION);

  for (int16_t x = kMatrixWidth; x >= -(textWidth); x--)
  {
    yield();
    matrix_clear();
    matrix->setCursor(x, 0);
    //matrix->setTextColor(currentColor);
    matrix->print(text);
    matrix->show();
    FastLED.delay(100);
//    delay(100);
  }
}

String imageElement(String imagePath, int imageId)
{

  return

      String("<div class=\"img-wrapper\">") +
      String("<img ") +

      String("width=\"336‬\" ") +
      String("height=\"24\" ") +

      String("src=\"") +
      imagePath +
      String("\">") +

      String("<a href=\"/delete?filename=") +
      imagePath +
      String("\" >delete") +
      String("</a>") +

      String(" | ") +

      String("<a href=\"/set?id=") +
      String(imageId) +
      String("\" >set") +
      String("</a>") +

      String("</div>");
}

String processor(const String &var)
{

  // if (var == "TOTAL_SPACE_TEMPLATE")
  // {
  //   return String(SPIFFS.totalBytes());
  // }

  // if (var == "USED_SPACE_TEMPLATE")
  // {
  //   return String(SPIFFS.usedBytes());
  // }

  // if (var == "FILE_LIST_TEMPLATE")
  // {

  //   String output;

  //   for (int i = 0; i < fileList.length(); i++)
  //   {
  //     const String *out = fileList.nth(i);
  //     if (*out)
  //     {
  //       output = output + imageElement(*out, i);
  //     }
  //   }
  //   return output;
  // }

  return String();
}

void configModeCallback(AsyncWiFiManager *wifiManager)
{
  matrix->clear();
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());

  display_scrollText(String("Connect to: " + wifiManager->getConfigPortalSSID()));

  matrix->clear();


  matrix->drawChar(0, 0, 'W', 255, 0, 1);

  matrix->show();

  //display_scrollText(String("Browse to: " + WiFi.softAPIP()));
}

void resetWifiConfig()
{

  Serial.println("Wifi Settings reset");
  wifiManager.resetSettings();
  WiFi.disconnect(false, true);
  WiFi.begin("0", "0");
  WiFi.persistent(false); // Do not memorise new connections
  //ESP.flashEraseSector(0x3fe);
  Serial.println("Rebooting in 10 seconds");
  delay(10000);
  ESP.restart();
}

void setupServer()
{
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  server.on("/random", HTTP_GET, [](AsyncWebServerRequest *request) {
    selected_index = 0;
    request->redirect("/");
  });

  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request) {
    resetWifiConfig();
  });

  server.on("/set", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("id"))
    {
      AsyncWebParameter *p = request->getParam("id");
      Serial.print("set: ");
      Serial.println(p->value());

      selected_index = p->value().toInt();

      request->redirect("/");
    }
  });

  server.on("/ajax", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();


    root["ssid"] = WiFi.SSID();
    root["ipaddress"] = WiFi.localIP().toString();
    root["disk_total"] = SPIFFS.totalBytes();
    root["disk_used"] = SPIFFS.usedBytes();

    JsonArray &array = jsonBuffer.createArray();

    for (int i = 0; i < fileList.length(); i++)
    {
      const String *out = fileList.nth(i);
      if (*out)
      {
        JsonObject &node = jsonBuffer.createObject();
        node["id"] = i;
        node["active"] = (i == selected_index);
        node["path"] = *out;
        array.add(node);
      }
    }

    root["gifs"] = array;

    root.printTo(*response);
    request->send(response);

    //    request->send(200, "application/json", serializeJson(doc));
  });

  server.on("/delete", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("filename"))
    {
      AsyncWebParameter *p = request->getParam("filename");
      Serial.print("delete: ");
      Serial.println(p->value());

      SPIFFS.remove(p->value());
      num_files = enumerateGIFFiles(GIF_DIRECTORY, false);

      request->redirect("/");
    }
  });

  // handler for the /update form POST (once file upload finishes)
  server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request) {
    request->redirect("/");
  },
            handle_update_progress_cb);

  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404);
  });

  server.serveStatic("/", SPIFFS, "/").setCacheControl("max-age=600");

  server.begin();
}

void setup()
{

  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUMMATRIX).setCorrection(TypicalSMD5050);
  FastLED.setBrightness(BRIGHTNESS);

  matrix->setRotation(MATRIX_ROTATION);
  matrix->clear();

  Serial.begin(115200);

  SPIFFS.begin();
  decoder.setScreenClearCallback(screenClearCallback);
  decoder.setUpdateScreenCallback(updateScreenCallback);
  decoder.setDrawPixelCallback(drawPixelCallback);

  decoder.setFileSeekCallback(fileSeekCallback);
  decoder.setFilePositionCallback(filePositionCallback);
  decoder.setFileReadCallback(fileReadCallback);
  decoder.setFileReadBlockCallback(fileReadBlockCallback);

  num_files = enumerateGIFFiles(GIF_DIRECTORY, false);
  Serial.print("number of files: ");
  Serial.println(num_files);

  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setConfigPortalTimeout(WIFIPORTALTIMEOUT);
  wifiManager.autoConnect(WIFIAPNAME);

  if (WiFi.isConnected())
  {
    Serial.println("Wifi Connected");
    setupServer();
  }
  else
  {
    Serial.println("Wifi failed");
  }


}

void loop()
{


  if (!ready)
  {
    ready = true;
    Serial.println("Ready");
//    if (WiFi.isConnected())
//    {
//      String currentIp = WiFi.localIP().toString();
//      display_scrollText(String(currentIp));
//      display_scrollText(String(currentIp));
//    }
  }
  else
  {

    //int index = random(num_files);
    static unsigned long futureTime;
    if (selected_index >= 0)
    {
      // index = selected_index;
    }

    if (futureTime < millis())
    {
      //if (openGifFilenameByIndex(GIF_DIRECTORY, index) >= 0)
      if (openGifFilenameByIndex(GIF_DIRECTORY, selected_index) >= 0)
      {
        decoder.startDecoding();

        // Calculate time in the future to terminate animation
        futureTime = millis() + (DISPLAY_TIME_SECONDS * 1000);
        // get the next index
        selected_index = (selected_index + 1) % num_files;
      }
    }

    decoder.decodeFrame();
  }
}