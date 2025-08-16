#include "UsbMsc.h"

#if CONFIG_TINYUSB_MSC_ENABLED

SDCard* UsbMsc::_card = nullptr;

UsbMsc::UsbMsc(void) {}

void UsbMsc::begin(SDCard* card) {
    _card = card;
    if (!_card || _card->getSectorCount() == 0) {
        return;
    }

    msc.vendorID("HIDForge");
    msc.productID("SD_MSC");
    msc.productRevision("1.0");
    msc.onRead(onRead);
    msc.onWrite(onWrite);
    msc.onStartStop(onStartStop);
    msc.mediaPresent(true);
    msc.begin(_card->getSectorCount(), _card->getSectorSize());
}

void UsbMsc::end(void) {
    // msc.end(); // This can be problematic, manage eject state if needed.
}

int32_t UsbMsc::onWrite(uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize) {
    if (_card && _card->writeSectors(buffer, lba, bufsize / _card->getSectorSize())) {
        return bufsize;
    }
    return -1;
}

int32_t UsbMsc::onRead(uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize) {
    if (_card && _card->readSectors((uint8_t *)buffer, lba, bufsize / _card->getSectorSize())) {
        return bufsize;
    }
    return -1;
}

bool UsbMsc::onStartStop(uint8_t power_condition, bool start, bool load_eject) {
    // The host is asking to eject the media.
    return true;
}

#endif // CONFIG_TINYUSB_MSC_ENABLED