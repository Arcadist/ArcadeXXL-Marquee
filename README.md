# Marquee
Handles all aspects of the marquee. The marquee is a built out of 8x112 LEDs and is driven by an ESP32 with a webserver. The user is able to upload animated GIFs which will eventually be displayed in the marquee.

Project files of a gif animation such as individual images or scripts are located in
```
resources/
```
Animated gifs are stored in 
```
data/gifs
```

## How to create an animated GIF with Gimp
- Create a new image 8 x 112 px
- Use black as the background color
- Create a new transparent layer and rename it with a name followed by a number in brackets which represents how long the image should be shown. Example: "001 - Layer 1 (1000ms)"
- Be aware since the layers are transparent the animation will be additive
- The lowest layer is the first image shown
- Export image as gif and select animation in options. Save animation in `data/gifs/`
- Save Gimpproject in `resources/`

## How to create an animated GIF from multiple images
If you have a bunch of individal images you'd like to put together as an animated gif you can use ImageMagicks convert utility:
```
$ convert -delay 20 -loop 0 *png animated.gif
```

## PCB
http://easyeda.com/sschueller/arcadist