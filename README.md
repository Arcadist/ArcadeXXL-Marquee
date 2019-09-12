# Marquee
Handles all aspects of the marquee. The marquee is a built out of 8x112 LEDs and is driven by an ESP32 with a webserver. The user is able to upload animated GIFs which will eventually be displayed in the marquee.

## How to create an animated GIF with Gimp
- Create a new image 8 x 112 px
- Use black as the background color
- Create a new transparent layer and rename it with a name and add many ms the image should be shown in brackets. Example:  "Layer 1 (1000ms)"
- Be aware since the layers are transparent the animation will be additive
- The lowest layer is the first image shown
- Export image as gif and select animation in options

## PCB
http://easyeda.com/sschueller/arcadist