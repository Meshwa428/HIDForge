#ifndef HIDFORGE_USB_MSC_H
#define HIDFORGE_USB_MSC_H

#include "sdkconfig.h"
#if CONFIG_TINYUSB_MSC_ENABLED

#include "USBMSC.h"
#include "msc/SDCard.h"

class UsbMsc
{
private:
    USBMSC msc;
    static SDCard* _card;
    static int32_t onRead(uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize);
    static int32_t onWrite(uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize);
    static bool onStartStop(uint8_t power_condition, bool start, bool load_eject);

public:
    UsbMsc(void);
    void begin(SDCard* card);
    void end(void);
};

#endif // CONFIG_TINYUSB_MSC_ENABLED
#endif // HIDFORGE_USB_MSC_H