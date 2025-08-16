#ifndef KEYBOARD_LAYOUT_H
#define KEYBOARD_LAYOUT_H

#include <Arduino.h>

#define SHIFT 0x80
#define ALT_GR 0xc0
#define ISO_KEY 0x64
#define ISO_REPLACEMENT 0x32

extern const uint8_t KeyboardLayout_de_DE[] PROGMEM;
extern const uint8_t KeyboardLayout_en_US[] PROGMEM;
extern const uint8_t KeyboardLayout_en_UK[] PROGMEM;
extern const uint8_t KeyboardLayout_es_ES[] PROGMEM;
extern const uint8_t KeyboardLayout_fr_FR[] PROGMEM;
extern const uint8_t KeyboardLayout_it_IT[] PROGMEM;
extern const uint8_t KeyboardLayout_pt_PT[] PROGMEM;
extern const uint8_t KeyboardLayout_pt_BR[] PROGMEM;
extern const uint8_t KeyboardLayout_sv_SE[] PROGMEM;
extern const uint8_t KeyboardLayout_da_DK[] PROGMEM;
extern const uint8_t KeyboardLayout_hu_HU[] PROGMEM;
extern const uint8_t KeyboardLayout_tr_TR[] PROGMEM;
extern const uint8_t KeyboardLayout_si_SI[] PROGMEM;

#endif // KEYBOARD_LAYOUT_H