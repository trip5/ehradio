#include "../core/options.h"
#if DSP_MODEL==DSP_SSD1327
#include "dspcore.h"
#include "../core/config.h"
#include "../core/logging.h"

#ifndef SCREEN_ADDRESS
  #define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; or scan it https://create.arduino.cc/projecthub/abdularbi17/how-to-scan-i2c-address-in-arduino-eaadda
#endif

#define CLR_ITEM1    0xA
#define CLR_ITEM2    0x8
#define CLR_ITEM3    0x5

// --- Gray Scale Table (16 значений, от GS0 до GS15) ---
// Стандартная линейная таблица:
// {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x10, 0x18, 0x20, 0x2F, 0x38, 0x3F}
// Нелинейная таблица для компенсации неравномерности:
static const uint8_t gamma_table[16] = {
  0x00, 0x01, 0x02, 0x03, 0x05, 0x07, 0x09, 0x0B,
  0x0D, 0x0F, 0x12, 0x15, 0x18, 0x1C, 0x20, 0x2A
};

// --- Общие статические функции ---
static void initCommonTheme() {
  config.theme.background = TFT_BG;
  config.theme.meta       = TFT_BG;
  config.theme.metabg     = TFT_LOGO;
  config.theme.metafill   = TFT_LOGO;
  config.theme.title1     = TFT_LOGO;
  config.theme.title2     = SILVER;
  config.theme.clock      = TFT_LOGO;
  config.theme.clockbg    = DARK_GRAY;
  config.theme.rssi       = TFT_FG;
  config.theme.weather    = ORANGE;
  config.theme.ip         = SILVER;
  config.theme.vol        = SILVER;
  config.theme.bitrate    = TFT_LOGO;
  config.theme.digit      = TFT_LOGO;
  config.theme.buffer     = TFT_FG;
  config.theme.volbarout  = TFT_FG;
  config.theme.volbarin   = SILVER;
  config.theme.playlist[0] = CLR_ITEM1;
  config.theme.playlist[1] = CLR_ITEM2;
  config.theme.playlist[2] = CLR_ITEM3;
  config.theme.playlist[3] = CLR_ITEM3;
  config.theme.playlist[4] = CLR_ITEM3;
}

// Auto-detect interface from pins
#if I2C_SDA!=255 && I2C_SCL!=255
#include <Wire.h>

#ifndef I2CFREQ_HZ
  #define I2CFREQ_HZ   6000000UL
#endif

TwoWire tw = TwoWire(0);

DspCore::DspCore(): Adafruit_SSD1327(DSP_WIDTH, DSP_HEIGHT, &tw, I2C_RST, I2CFREQ_HZ) {}

void DspCore::initDisplay() {
  tw.begin(I2C_SDA, I2C_SCL);
  if (!begin(SCREEN_ADDRESS)) {
    ERRORLOG("SSD1327 allocation failed");
    delay(100);
    ESP.restart();
  }
  
  // --- Gray Scale Table (встроено в метод класса) ---
  oled_command(0xB8);  // Set Gray Scale Table
  for (uint8_t i = 0; i < 16; i++) {
    oled_command(gamma_table[i]);
  }
  
  // Настройка контраста и VCOMH
  oled_command(0x81);  // Set Contrast Current
  oled_command(0xAA);  // Значение 0xAA (170) — компромисс
  oled_command(0xDB);  // Set VCOMH Deselect Level
  oled_command(0x20);  // Значение по умолчанию
  
  initCommonTheme();
  cp437(true);
  flip();
  invert();
  setTextWrap(false);
}
#else // SPI
#ifndef DEF_SPI_FREQ
  #define DEF_SPI_FREQ        8000000UL
#endif

DspCore::DspCore(): Adafruit_SSD1327(DSP_WIDTH, DSP_HEIGHT, &SPI, TFT_DC, TFT_RST, TFT_CS, DEF_SPI_FREQ) {}

void DspCore::initDisplay() {
  if (!begin(SCREEN_ADDRESS)) {
    ERRORLOG("SSD1327 allocation failed");
    delay(100);
    ESP.restart();
  }
  
  // --- Gray Scale Table (встроено в метод класса) ---
  oled_command(0xB8);  // Set Gray Scale Table
  for (uint8_t i = 0; i < 16; i++) {
    oled_command(gamma_table[i]);
  }
  
  // Настройка контраста и VCOMH
  oled_command(0x81);  // Set Contrast Current
  oled_command(0xAA);  // Значение 0xAA (170) — компромисс
  oled_command(0xDB);  // Set VCOMH Deselect Level
  oled_command(0x20);  // Значение по умолчанию
  
  initCommonTheme();
  cp437(true);
  flip();
  invert();
  setTextWrap(false);
}
#endif

void DspCore::clearDsp(bool black){ fillScreen(black?0:config.theme.background); }
void DspCore::flip(){
#if DSP_WIDTH==DSP_HEIGHT
  if(ROTATE_90){
    setRotation(config.store.flipscreen?3:1);
  }else{
    setRotation(config.store.flipscreen?2:0);
  }
#else
  setRotation(config.store.flipscreen?2:0);
#endif
}
void DspCore::invert(){ invertDisplay(config.displayIsInverted != DSP_INVERT_QUIRK); }
void DspCore::sleep(void){ oled_command(SSD1327_DISPLAYOFF); }
void DspCore::wake(void){ oled_command(SSD1327_DISPLAYON); }

#endif
