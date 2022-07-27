#include <Adafruit_Protomatter.h>

uint8_t rgbPins[]  = {7, 8, 9, 10, 11, 12};
uint8_t addrPins[] = {17, 18, 19, 20};
uint8_t clockPin   = 14;
uint8_t latchPin   = 15;
uint8_t oePin      = 16;

Adafruit_Protomatter matrix(
  64, 4, 1, rgbPins, 4, addrPins, clockPin, latchPin, oePin, false);

uint16_t red123 = matrix.color565(238, 10, 10);
uint16_t green456 = matrix.color565(0, 147, 60);
uint16_t magenta7 = matrix.color565(185, 51, 173);
uint16_t blueACE = matrix.color565(40, 80, 173);
uint16_t orangeBDFM = matrix.color565(255, 75, 0);
uint16_t greenG = matrix.color565(108, 190, 169);
uint16_t brownJZ = matrix.color565(153, 102, 51);
uint16_t grayL = matrix.color565(167, 169, 172);
uint16_t yellowNQRW = matrix.color565(252, 204, 10);
uint16_t grayS = matrix.color565(128, 129, 131);
uint16_t black = matrix.color565(0, 0, 0);
uint16_t white = matrix.color565(255, 255, 255);

struct LineColor {
    String routeId;
    uint16_t color;
};

uint16_t getLineColor(String routeId) {
    LineColor lineColors[23] = {
        { "1", red123 },
        { "2", red123 },
        { "3", red123 },
        { "4", green456 },
        { "5", green456 },
        { "6", green456 },
        { "7", magenta7 },
        { "A", blueACE },
        { "C", blueACE },
        { "E", blueACE },
        { "B", orangeBDFM },
        { "D", orangeBDFM },
        { "F", orangeBDFM },
        { "M", orangeBDFM },
        { "G", greenG },
        { "J", brownJZ },
        { "Z", brownJZ },
        { "L", grayL },
        { "N", yellowNQRW },
        { "Q", yellowNQRW },
        { "R", yellowNQRW },
        { "W", yellowNQRW },
        { "S", grayS }
    };

    for (int i = 0; i < 23; i ++) {
        if (lineColors[i].routeId == routeId) {
            return lineColors[i].color;
        }
    }

    return matrix.color565(255, 255, 255);
}