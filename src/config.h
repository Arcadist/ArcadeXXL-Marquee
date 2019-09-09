#ifndef CONFIG_H
#define CONFIG_H

// setup the dimensions of the LED matrix
// size of each tile
#define MATRIX_WIDTH   112
#define MATRIX_HEIGHT  8

// number of tiles
#define MATRIX_TILE_H  1
#define MATRIX_TILE_V  1

// layout https://learn.adafruit.com/adafruit-neopixel-uberguide/neomatrix-library
#define MATRIX_LAYOUT NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG

#define MATRIX_ROTATION 0

// the digital pin for controlling the WS2812 LED Matrix
#define LED_PIN  5
#define COLOR_ORDER GRB
#define CHIPSET WS2811

// data directory (no trailing slash!)
#define GIF_DIRECTORY "/gifs"

// default brightness
#define BRIGHTNESS 16

// Wifi access point name
#define WIFIAPNAME "ArcadeXXL"

// Time to wait (in seconds) for connection to portal
#define WIFIPORTALTIMEOUT 30

// How long to play animated gif
#define DISPLAY_TIME_SECONDS 10

#endif