# Hexels
Hexels is named after the hexagonal pixels that make up an individually, addressable LED matrix in the shape of a 6-pixels-per-side hexagon.
!()[clock.gif]
!()[line.gif]
!()[sheet.gif]
Hexels is written in C++, uploaded OTA to an ESP32, and the result of many frustrations with matrix transformations / color interpolation. For now it is hanging
in my bedroom as a wall clock, although there are many possible applications and improvements that I would like to make:

## Possible improvements:
* Weather montages (rain, sun, thunder) that update with the real weather
* Sky montages 
    * sunrise, sunset
* Something with calendars / live updates with phone notifications
* Something with Satellite view over your current location
* A rough view of the current cape peninsular with weather/clouds
* Neural network to learn and play chess/checkers/tetris/snake/pong
* Hex based maze survival game?
* [Genghis visualiser](https://github.com/beyarkay/genghis) (with the bot at the centre, the rest of the world just moves round it)
* virtual lava lamp
* live updating map for [property/car prices](https://github.com/beyarkay/PurchaseAnalysis)?
* live update of some online game?
* live update of satellites overhead/flights?
* low res memes?
* DVD logo thing, but it always hits the corners
* ~~synced with spotify~~ There isn't much support for a live feed from the spotify API, and I'm not sure the ESP32 could handle the processing required to render the album artwork
* Program a proper graphics renderer, with a z value to indicate depth / layers
* Make the minecraft logo?

