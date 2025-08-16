#ifndef HIDFORGE_H
#define HIDFORGE_H

#include <Arduino.h>
#include "USB.h"

// Main public interface classes
#include "UsbHid.h"
#include "BleHid.h"
#include "UsbMouse.h"
#include "BleMouse.h"
#include "UsbMsc.h"
#include "HIDInterface.h"
#include "MouseInterface.h"

// Common definitions and layouts
#include "common/keys.h"
#include "layouts/KeyboardLayout.h"

// SD Card related includes for UsbMsc
#include "msc/SDCard.h"
#include "msc/SDCardArduino.h"
#include "msc/SDCardIdf.h"
#include "msc/SDCardLazyWrite.h"
#include "msc/SDCardMultiSector.h"

#endif // HIDFORGE_H