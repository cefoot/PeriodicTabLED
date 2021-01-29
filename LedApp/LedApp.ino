// A basic everyday NeoPixel strip test program.

// NEOPIXEL BEST PRACTICES for most reliable operation:
// - Add 1000 uF CAPACITOR between NeoPixel strip's + and - connections.
// - MINIMIZE WIRING LENGTH between microcontroller board and first pixel.
// - NeoPixel strip's DATA-IN should pass through a 300-500 OHM RESISTOR.
// - AVOID connecting NeoPixels on a LIVE CIRCUIT. If you must, ALWAYS
//   connect GROUND (-) first, then +, then data.
// - When using a 3.3V microcontroller with a 5V-powered NeoPixel strip,
//   a LOGIC-LEVEL CONVERTER on the data line is STRONGLY RECOMMENDED.
// (Skipping these may work OK on your workbench but can fail in the field)

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "Arduino.h"
#include "Ticker.h"
#include "WifiUpdate.h"
#include "Index_Html.h"

#ifndef APSSID
#define APSSID "PriodicTable"
#define APPSK  "thereisnospoon"
#endif
/* Set these to your desired credentials. */
const char *ssid = APSSID;
const char *password = APPSK;

ESP8266WebServer server(80);

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define LED_PIN    D8

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 10

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:

Ticker scheduledTicker;
long firstPixelHue = 0;
bool rainbowRunning = true;

void handleRoot() {
  server.send(200, "text/html", INDEX_Page);
}

void handleUpdate1(){
  handleUpdate(server);
}

void handleUpload1(){
  handleUpload(server);
}

void disableRainbow(){
  if (!rainbowRunning){
    return;
  }
  scheduledTicker.detach();
  rainbowRunning = false;
}

void handleRainbow(){
  if (rainbowRunning){
    return;
  }
  rainbow();  
  rainbowRunning = true;
  scheduledTicker.attach_ms_scheduled(200, rainbow);
}

void handleBlack(){
  disableRainbow();
  for(int i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(0, 0, 0));
    }
    strip.show();
}

void handleWhite(){
  disableRainbow();
  for(int i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(200, 200, 200));
    }
    strip.show();
}

void handleBlue(){
  disableRainbow();
  for(int i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(0, 0, 150));
    }
    strip.show();
}

void handleRed(){
  disableRainbow();
  for(int i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(150, 0, 0));
    }
    strip.show();
}

void handleGreen(){
  disableRainbow();
  for(int i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(0, 150, 0));
    }
    strip.show();
}

// setup() function -- runs once at startup --------------------------------

void setup() {
  // These lines are specifically to support the Adafruit Trinket 5V 16 MHz.
  // Any other board, you can remove this part (but no harm leaving it):
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif
  // END of Trinket-specific code.

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)
  
  scheduledTicker.attach_ms_scheduled(200, rainbow);

  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.begin(115200);
  Serial.println();
  Serial.print("Configuring access point...");
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.on("/", handleRoot);
  
  server.on("/led/rainbow", handleRainbow);
  server.on("/led/black", handleBlack);
  server.on("/led/blue", handleBlue);
  server.on("/led/green", handleGreen);
  server.on("/led/red", handleRed);
  server.on("/led/white", handleWhite);
  server.on("/update", HTTP_POST, handleUpdate1, handleUpload1);
  server.begin();
}


// loop() function -- runs repeatedly as long as board is on ---------------

void loop() {
  server.handleClient();
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void rainbow() {
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this outer loop:
//  for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {
    for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the single-argument hue variant. The result
      // is passed through strip.gamma32() to provide 'truer' colors
      // before assigning to each pixel:
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show(); // Update strip with new contents
    //instead of gooing through increment each time this is called
    firstPixelHue += 256;
    if(firstPixelHue >= 5*65536){
      firstPixelHue = 0;
    }
//  }
}
