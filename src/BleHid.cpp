#include "BleHid.h"
#include "BleKeyboard.h"

BleHid::BleHid(BleKeyboard* keyboard) : bleKeyboard_(keyboard), _asciimap(nullptr) {
}

BleHid::~BleHid() {
}

void BleHid::begin(const uint8_t *layout) {
    if (bleKeyboard_) {
        setLayout(layout);
    }
}

void BleHid::end() {
    // The BleManager handles the lifecycle of the keyboard object.
}

size_t BleHid::press(uint8_t k) {
    if (bleKeyboard_) {
        return bleKeyboard_->press(k);
    }
    return 0;
}

size_t BleHid::pressRaw(uint8_t k) {
    return press(k);
}

size_t BleHid::press(const MediaKeyReport k) {
    if (bleKeyboard_) {
        return bleKeyboard_->press(k);
    }
    return 0;
}

size_t BleHid::release(uint8_t k) {
    if (bleKeyboard_) {
        return bleKeyboard_->release(k);
    }
    return 0;
}

size_t BleHid::releaseRaw(uint8_t k) {
    return release(k);
}

void BleHid::releaseAll() {
    if (bleKeyboard_) {
        bleKeyboard_->releaseAll();
    }
}

size_t BleHid::write(uint8_t k) {
    if (bleKeyboard_) {
        return bleKeyboard_->write(k);
    }
    return 0;
}

size_t BleHid::write(const MediaKeyReport c) {
    if (bleKeyboard_) {
        return bleKeyboard_->write(c);
    }
    return 0;
}

size_t BleHid::write(const uint8_t *buffer, size_t size) {
    if (bleKeyboard_) {
        return bleKeyboard_->write(buffer, size);
    }
    return 0;
}

bool BleHid::isConnected() {
    if (bleKeyboard_) {
        return bleKeyboard_->isConnected();
    }
    return false;
}

void BleHid::setLayout(const uint8_t *layout) {
    if (bleKeyboard_) {
        bleKeyboard_->setLayout(layout);
    }
    _asciimap = layout;
}
