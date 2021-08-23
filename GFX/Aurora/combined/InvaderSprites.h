#define MOTHERSHIP_HEIGHT 4
#define MOTHERSHIP_WIDTH 16

const byte MotherShipGfx[][8] PROGMEM = {
{ 0xfc, 0x3f, 0xb6, 0x6d, 0xff, 0xff, 0x9c, 0x39},
{ 0xfc, 0x00, 0x4a, 0x01, 0xff, 0x03, 0xb5, 0x02}
};

void drawInvader(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color) {
    drawXBitmap(x, y, bitmap, w, h, color);
}
