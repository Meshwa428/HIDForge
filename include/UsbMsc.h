#ifndef HIDFORGE_USB_MSC_H
#define HIDFORGE_USB_MSC_H

#include "sdkconfig.h"
#if CONFIG_TINYUSB_MSC_ENABLED

#include "USBMSC.h"
#include "msc/SDCard.h"
#include <functional> // <-- ADD THIS

class UsbMsc
{
private:
    USBMSC msc;
    static SDCard* _card;
    static std::function<void()> onEjectCallback_; // <-- ADD THIS

    static int32_t onRead(uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize);
    static int32_t onWrite(uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize);
    static bool onStartStop(uint8_t power_condition, bool start, bool load_eject);

public:
    UsbMsc(void);
    void begin(SDCard* card, const char* vendor_id = "HIDForge", const char* product_id = "MSC", const char* product_revision = "1.0");
    void end(void);
    
    // --- ADD THIS PUBLIC METHOD ---
    void onEject(std::function<void()> callback);
};

#endif // CONFIG_TINYUSB_MSC_ENABLED
#endif // HIDFORGE_USB_MSC_H