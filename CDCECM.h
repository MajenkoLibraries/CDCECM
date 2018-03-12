#ifndef _CDCECM_H
#define _CDCECM_H

#include <Arduino.h>

class CDCECM : public USBDevice {

    private:

        USBManager *_manager;
        uint8_t _ifControl;
        uint8_t _ifBulk;
        uint8_t _epControl;
        uint8_t _epBulk;
        uint8_t _stringMac;


#if defined (__PIC32MX__)
#define CDCECM_BULKEP_SIZE 64
        uint8_t _bulkRxA[CDCECM_BULKEP_SIZE];
        uint8_t _bulkRxB[CDCECM_BULKEP_SIZE];
        uint8_t _bulkTxA[CDCECM_BULKEP_SIZE];
        uint8_t _bulkTxB[CDCECM_BULKEP_SIZE];
#elif defined(__PIC32MZ__)
#define CDCECM_BULKEP_SIZE 512
        uint8_t _bulkRxA[CDCECM_BULKEP_SIZE];
        uint8_t _bulkRxB[CDCECM_BULKEP_SIZE];
        uint8_t _bulkTxA[CDCECM_BULKEP_SIZE];
        uint8_t _bulkTxB[CDCECM_BULKEP_SIZE];
#endif

        uint8_t _ctlA[8];
        uint8_t _ctlB[8];

        uint64_t _macHost;
        uint64_t _macDevice;

        uint16_t _packetFilter;

        void (*_onFrameRx)(uint64_t, uint64_t, uint16_t, uint8_t *, uint16_t);
        uint8_t _rxFrame[1540];
        uint16_t _rxFramePos;

    public:
        CDCECM(uint64_t macHost, uint64_t macDevice) : _macHost(macHost), _macDevice(macDevice) {}

        uint16_t getDescriptorLength();
        uint8_t getInterfaceCount();
        bool getStringDescriptor(uint8_t idx, uint16_t maxlen);
        uint32_t populateConfigurationDescriptor(uint8_t *buf);
        void initDevice(USBManager *manager);
        bool getDescriptor(uint8_t ep, uint8_t target, uint8_t id, uint8_t maxlen);
        void configureEndpoints();
        bool onSetupPacket(uint8_t ep, uint8_t target, uint8_t *data, uint32_t l);
        bool onOutPacket(uint8_t ep, uint8_t target, uint8_t *data, uint32_t l);
        bool onInPacket(uint8_t ep, uint8_t target, uint8_t *data, uint32_t l);
        void onEnumerated();

        bool getReportDescriptor(uint8_t __attribute__((unused)) ep, uint8_t __attribute__((unused)) target, uint8_t __attribute__((unused)) id, uint8_t __attribute__((unused)) maxlen) { return false; }

        void onFrameRx(void (*f)(uint64_t, uint64_t, uint16_t, uint8_t *, uint16_t)) { 
            _onFrameRx = f;
        } 

        void sendFrame(uint64_t tha, uint16_t type, const uint8_t *data, uint32_t len);
};

#endif
