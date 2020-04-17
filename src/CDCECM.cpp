#include <CDCECM.h>

uint16_t CDCECM::getDescriptorLength() {
    return 70;
}

uint8_t CDCECM::getInterfaceCount() {
    return 2;
}

// 0x0C
#define ECM 0x06

uint32_t CDCECM::populateConfigurationDescriptor(uint8_t *buf) {
    uint8_t i = 0;

    buf[i++] =     8;          // bLength
    buf[i++] =     11;         // bDescriptorType
    buf[i++] =     _ifControl; // bFirstInterface
    buf[i++] =     2;          // bInterfaceCount
    buf[i++] =     0x02;       // bFunctionClass
    buf[i++] =     ECM;        // bFunctionSubClass
    buf[i++] =     0x00;       // bFunctionProtocol ?7
    buf[i++] =     0; // iFunction


    buf[i++] = 9;          // length
    buf[i++] = 0x04;       // interface descriptor
    buf[i++] = _ifControl; // interface number
    buf[i++] = 0x00;       // alternate
    buf[i++] = 0x01;       // num endpoints
    buf[i++] = 0x02;       // interface class (comm)
    buf[i++] = ECM;        // subclass (eem)
    buf[i++] = 0x00;       // protocol (eth) ?0
    buf[i++] = 0;          // iInterface

    buf[i++] = 5;          // length
    buf[i++] = 0x24;       // header functional descriptor
    buf[i++] = 0x00;
    buf[i++] = 0x10;
    buf[i++] = 0x01;

    buf[i++] = 5;          // length
    buf[i++] = 0x24;       // union functional descriptor
    buf[i++] = 0x06;
    buf[i++] = _ifControl;
    buf[i++] = _ifBulk;

    buf[i++] = 0x0D; /* 0 bLength - 13 bytes */
    buf[i++] = 0x24; /* 1 bDescriptortype, CS_INTERFACE ?24 */
    buf[i++] = 0x0F; /* 2 bDescriptorsubtype, ETHERNET NETWORKING */
    buf[i++] = _stringMac; /* 3 iMACAddress, Index of MAC address string */
    buf[i++] = 0x00;
    buf[i++] = 0x00;
    buf[i++] = 0x00;
    buf[i++] = 0x00; /* 4 bmEthernetStatistics - Handles None */
    buf[i++] = 0xEA;
    buf[i++] = 0x05; /* 8 wMaxSegmentSize - 1514 bytes */
    buf[i++] = 0x00;
    buf[i++] = 0x00; /* 10 wNumberMCFilters - No multicast filters */
    buf[i++] = 0x00; /* 12 bNumberPowerFilters - No wake-up feature */

    buf[i++] = 0x07; /* 0 bLength */
    buf[i++] = 0x05; /* 1 bDescriptorType */
    buf[i++] = _epControl | 0x80;/* 2 bEndpointAddress - IN endpoint*/
    buf[i++] = 0x03, /* 3 bmAttributes - Interrupt type */
    buf[i++] = 0x40, /* 4 wMaxPacketSize - Low */
    buf[i++] = 0x00, /* 5 wMaxPacketSize - High */
    buf[i++] = 0xFF, /* 6 bInterval */







    buf[i++] = 0x09; /* 0 bLength */
    buf[i++] = 0x04; /* 1 bDescriptorType */
    buf[i++] = _ifBulk; /* 2 bInterfacecNumber */
    buf[i++] = 0x00; /* 3 bAlternateSetting */
    buf[i++] = 0x02; /* 4 bNumEndpoints */
    buf[i++] = 0x0a; /* 5 bInterfaceClass */
    buf[i++] = 0x00; /* 6 bInterfaceSubClass */
    buf[i++] = 0x00; /* 7 bInterfaceProtocol*/
    buf[i++] = 0x00; /* 8 iInterface - No string descriptor*/

    buf[i++] = 0x07; /* 0 bLength */
    buf[i++] = 0x05; /* 1 bDescriptorType */
    buf[i++] = _epBulk; /* 2 bEndpointAddress - OUT endpoint */
    buf[i++] = 0x02; /* 3 bmAttributes - Bulk type */
    if (_manager->isHighSpeed()) {
        buf[i++] = 0x00;
        buf[i++] = 0x02;     // packet size
    } else {
        buf[i++] = 0x40;
        buf[i++] = 0x00;     // packet size
    }
    buf[i++] = 0x00; /* 6 bInterval */

    buf[i++] = 0x07; /* 0 bLength */
    buf[i++] = 0x05; /* 1 bDescriptorType */
    buf[i++] = _epBulk | 0x80; /* 2 bEndpointAddress - IN endpoint */
    buf[i++] = 0x02; /* 3 bmAttributes - Bulk type */
    if (_manager->isHighSpeed()) {
        buf[i++] = 0x00;
        buf[i++] = 0x02;     // packet size
    } else {
        buf[i++] = 0x40;
        buf[i++] = 0x00;     // packet size
    }
    buf[i++] = 0x01; /* 6 bInterval */

    return i;
}
void CDCECM::initDevice(USBManager *manager) {
    _manager = manager;
    _ifControl = _manager->allocateInterface();
    _ifBulk = _manager->allocateInterface();
    _epControl = _manager->allocateEndpoint();
    _epBulk = _manager->allocateEndpoint();
    _stringMac = _manager->allocateString();
}

bool CDCECM::getDescriptor(uint8_t __attribute__((unused)) ep, uint8_t __attribute__((unused)) target, uint8_t __attribute__((unused)) id, uint8_t __attribute__((unused)) maxlen) {
    return false;
}
void CDCECM::configureEndpoints() {
    _manager->addEndpoint(_epControl, EP_OUT, EP_CTL, 8, _ctlA, _ctlB);
    if (_manager->isHighSpeed()) {
        _manager->addEndpoint(_epBulk, EP_IN, EP_BLK, 512, _bulkRxA, _bulkRxB);
        _manager->addEndpoint(_epBulk, EP_OUT, EP_BLK, 512, _bulkTxA, _bulkTxB);
    } else {
        _manager->addEndpoint(_epBulk, EP_IN, EP_BLK, 64, _bulkRxA, _bulkRxB);
        _manager->addEndpoint(_epBulk, EP_OUT, EP_BLK, 64, _bulkTxA, _bulkTxB);
    }
}
bool CDCECM::onSetupPacket(uint8_t __attribute__((unused)) ep, uint8_t __attribute__((unused)) target, uint8_t *data, uint32_t __attribute__((unused)) l) {

    uint16_t signature = (data[0] << 8) | data[1];
    uint16_t wValue    = (data[3] << 8) | data[2];
    uint16_t wIndex    = (data[5] << 8) | data[4];
    uint16_t wLength   = (data[7] << 8) | data[6];

    if (wIndex != _ifControl) return false;

    switch (signature) {
        case 0x2140: // SET_ETHERNET_MULTICAST_FILTERS
            return false;
        case 0x2141: // SET_ETHERNET_POWER_MANAGEMENT_PATTERN FILTER
            return false;
        case 0x2142: // GET_ETHERNET_POWER_MANAGEMENT_PATTERN FILTER
            return false;
        case 0x2143: { // SET_ETHERNET_PACKET_FILTER
                _packetFilter = wValue;
                _manager->sendBuffer(0, NULL, 0);
                return true;
            }
        case 0x2144: // GET_ETHERNET_STATISTIC
            return false;
    }
    return false;
}

bool CDCECM::onInPacket(uint8_t ep, uint8_t target, uint8_t __attribute__((unused)) *data, uint32_t __attribute__((unused)) l) {
    if (ep == _epBulk) {
        return true;
    }
    return false;
}

static inline uint64_t macToUint64(const uint8_t *mac) {
    uint64_t out = 0;

    out |= ((uint64_t)mac[0] << 40);
    out |= ((uint64_t)mac[1] << 32);
    out |= ((uint64_t)mac[2] << 24);
    out |= ((uint64_t)mac[3] << 16);
    out |= ((uint64_t)mac[4] << 8);
    out |= ((uint64_t)mac[5] << 0);
    return out;
}

bool CDCECM::onOutPacket(uint8_t ep, uint8_t target, uint8_t *data, uint32_t l) {
    if (ep == _epBulk) {
        if (l > 0) {
            memcpy(&_rxFrame[_rxFramePos], data, l);
            _rxFramePos += l;
        }
        if (l < CDCECM_BULKEP_SIZE) { // End of frame

            if (_onFrameRx) {
                uint64_t dstMac = macToUint64(&_rxFrame[0]);
                uint64_t srcMac = macToUint64(&_rxFrame[6]);
                uint16_t etherType = (_rxFrame[12] << 8) | _rxFrame[13];
                if ((dstMac == 0xFFFFFFFFFFFF) || (dstMac == _macDevice)) {
                    _onFrameRx(dstMac, srcMac, etherType, &_rxFrame[14], _rxFramePos - 14);
                }
            }
            _rxFramePos = 0;
        } 
        return true;
    }
    return false;
}

void CDCECM::onEnumerated() {
    _rxFramePos = 0;
}

static const char *hexCodes = "0123456789abcdef";

bool CDCECM::getStringDescriptor(uint8_t idx, uint16_t maxlen) {
    if (idx == _stringMac) {
        uint8_t mlen = 12;
        uint8_t o[mlen * 2 + 2];
        o[0] = mlen * 2 + 2;
        o[1] = 0x03;
        for (int i = 0; i < mlen; i++) {
            uint64_t c = _macHost;
            c >>= ((11-i) * 4);
            c &= 0x0f;
            o[2 + (i * 2)] = hexCodes[c];
            o[3 + (i * 2)] = 0;
        }
        _manager->sendBuffer(0, (const uint8_t *)&o, min(maxlen, mlen * 2 + 2));

        return true;
    }
    return false;
}

void CDCECM::sendFrame(uint64_t tha, uint16_t type, const uint8_t *data, uint32_t len) {
    uint8_t buf[len + 14];
    buf[0] = (tha >> 40);
    buf[1] = (tha >> 32);
    buf[2] = (tha >> 24);
    buf[3] = (tha >> 16);
    buf[4] = (tha >>  8);
    buf[5] = (tha >>  0);
    buf[6] = (_macDevice >> 40);
    buf[7] = (_macDevice >> 32);
    buf[8] = (_macDevice >> 24);
    buf[9] = (_macDevice >> 18);
    buf[10] = (_macDevice >>  8);
    buf[11] = (_macDevice >>  0);
    buf[12] = type >> 8;
    buf[13] = type;
    memcpy(&buf[14], data, len);
    _manager->sendBuffer(_epBulk, buf, len + 14);
}

