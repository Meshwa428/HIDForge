#include "UsbMsc.h"

#if CONFIG_TINYUSB_MSC_ENABLED

// --- INITIALIZE THE NEW STATIC MEMBERS ---
SDCard* UsbMsc::_card = nullptr;
std::function<void()> UsbMsc::onEjectCallback_ = nullptr;

UsbMsc::UsbMsc(void) {}

// --- MODIFIED: Added default arguments to match the header ---
void UsbMsc::begin(SDCard* card, const char* vendor_id, const char* product_id, const char* product_revision) {
    _card = card;
    if (!_card || _card->getSectorCount() == 0) {
        return;
    }

    msc.vendorID(vendor_id);
    msc.productID(product_id);
    msc.productRevision(product_revision);
    msc.onRead(onRead);
    msc.onWrite(onWrite);
    msc.onStartStop(onStartStop);
    msc.mediaPresent(true);
    msc.begin(_card->getSectorCount(), _card->getSectorSize());
}

// --- ADD THIS METHOD IMPLEMENTATION ---
void UsbMsc::onEject(std::function<void()> callback) {
    onEjectCallback_ = callback;
}


void UsbMsc::end(void) {
    msc.end();
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
    // --- MODIFIED: Call the callback on eject ---
    if (load_eject && !start) { // This condition signifies an eject event
        if (onEjectCallback_) {
            onEjectCallback_();
        }
    }
    return true;
}

#endif // CONFIG_TINYUSB_MSC_ENABLED