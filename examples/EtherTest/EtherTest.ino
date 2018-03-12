#include <CDCECM.h>

extern USBManager USB;
CDCECM eth0(0x00f055012345, 0x00f055012346);
//CDCECM eth1(0x00f0556789ab, 0x00f0556789ac);

static inline uint16_t bswap(uint16_t v) {
	return ((v >> 8) & 0xFF) | (v << 8);
}

static inline uint16_t htons(uint16_t v) {
    return bswap(v);
}

static inline uint16_t ntohs(uint16_t v) {
    return bswap(v);
}

#define NTOHS(A) A = bswap((uint16_t)A)
#define HTONS(A) A = bswap((uint16_t)A)

const uint32_t myIp = 0x0a000002;

struct ethernet {
    uint8_t tha[6];
    uint8_t sha[6];
    uint16_t type;
    uint8_t data[2];
} __attribute__((packed));

struct arp {
	uint16_t htype;
	uint16_t ptype;
	uint8_t hlen;
	uint8_t plen;
	uint16_t oper;
	uint8_t sha[6];
	uint8_t spa[4];
	uint8_t tha[6];
	uint8_t tpa[4];
} __attribute__((packed));

struct ipv4 {
    unsigned ihl:4;
    unsigned version:4;

    unsigned ecn:2;
    unsigned dscp:6;
    
    uint16_t totLen;
    uint16_t id;
    
    unsigned fragment:13;
    unsigned flags:3;
    
    uint8_t ttl;
    uint8_t protocol;
    uint16_t cksum;
    uint8_t sip[4];
    uint8_t dip[4];
} __attribute__((packed));

struct icmp {
    uint8_t type;
    uint8_t code;
    uint16_t cksum;
    uint32_t rest;
} __attribute__((packed));

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

static inline uint32_t ipToUint32(const uint8_t *ip) {
    uint32_t out = 0;

    out |= ((uint32_t)ip[0] << 24);
    out |= ((uint32_t)ip[1] << 16);
    out |= ((uint32_t)ip[2] << 8);
    out |= ((uint32_t)ip[3] << 0);
    return out;
}

uint16_t checksum(uint8_t *data, uint32_t len) {
    uint16_t cs = 0;
    uint32_t tot = 0;
    for (uint32_t i = 0; i < len; i+=2) {

        uint16_t v = 0;
        if (i+1 == len) { // Odd length {
            v = data[i] << 8;
        } else {
            v = (data[i] << 8) | data[i+1];
        }
        tot += v;
    }

    while (tot > 0xFFFF) {
        uint32_t t = tot & 0xFFFF;
        t += (tot >> 16);
        tot = t;
    }

    cs = tot;

    return ~cs;
}

void rx(uint64_t dst, uint64_t src, uint16_t etherType, uint8_t *data, uint16_t len) {

    switch (etherType) {

        case 0x0800: { // IPv4
            struct ipv4 *ip = (struct ipv4 *)data;
            Serial.printf("IP %d Proto %02x %d.%d.%d.%d -> %d.%d.%d.%d\r\n", 
                ip->ihl,
                ip->protocol,
                ip->sip[0], ip->sip[1], ip->sip[2], ip->sip[3],
                ip->dip[0], ip->dip[1], ip->dip[2], ip->dip[3]);

            uint32_t payloadLength = ntohs(ip->totLen) - (ip->ihl*4);

            switch (ip->protocol) {
                case 0x01: { // ICMP                   
                    struct icmp *icmp = (struct icmp *)&data[ip->ihl * 4];
                    Serial.printf("ICMP %d.%d\r\n", icmp->type, icmp->code);
                    switch (icmp->type) {
                        
                        case 0x08: { // Echo Request

                            memcpy(ip->dip, ip->sip, 4); // Copy source to destination.

                            // Load my IP into source
                            ip->sip[0] = (myIp >> 24) & 0xFF;
                            ip->sip[1] = (myIp >> 16) & 0xFF;
                            ip->sip[2] = (myIp >> 8) & 0xFF;
                            ip->sip[3] = (myIp >> 0) & 0xFF;
                            ip->ttl = 255;
                            ip->cksum = 0; // Clear checksum

                            icmp->cksum = 0;
                            icmp->type = 0;
                            
                            uint16_t cs = checksum((uint8_t *)icmp, payloadLength);
                            icmp->cksum = htons(cs);
                            cs = checksum((uint8_t *)ip, ip->ihl * 4);
                            ip->cksum = htons(cs);

                            Serial.printf("Send %d\r\n", htons(ip->totLen));
                            
                            eth0.sendFrame(src, 0x0800, (uint8_t *)ip, htons(ip->totLen));
                        }
                        break;
                    }
                }
                break;
            }

                
        }
        break;
        
        case 0x0806: { // ARP
                struct arp *arp = (struct arp *)data;
                NTOHS(arp->htype);
                NTOHS(arp->ptype);
                NTOHS(arp->oper);

                uint32_t spa = ipToUint32(arp->spa);
                uint32_t tpa = ipToUint32(arp->tpa);
                if (tpa == myIp) {

                    struct arp rarp;

                    rarp.htype = htons(1);
                    rarp.ptype = htons(0x0800);
                    rarp.hlen = 6;
                    rarp.plen = 4;
                    rarp.oper = htons(2);
                    memcpy(rarp.tha, arp->sha, 6);
                    rarp.sha[0] = 0x00;
                    rarp.sha[1] = 0xf0;
                    rarp.sha[2] = 0x55;
                    rarp.sha[3] = 0x01;
                    rarp.sha[4] = 0x23;
                    rarp.sha[5] = 0x46;

                    memcpy(rarp.spa, arp->tpa, 4);
                    memcpy(rarp.tpa, arp->spa, 4);

                    eth0.sendFrame(
                        src,
                        0x0806,
                        (uint8_t *)&rarp,
                        sizeof(struct arp)
                    );                      
                }

            }
            break;
    
		default:
			Serial.printf("Unknown ethernet type %04x\r\n", etherType);
			break;
	}
}

void setup() {
	USB.end();
	USB.addDevice(eth0);
//	USB.addDevice(eth1);
	eth0.onFrameRx(rx);
//	eth1.onFrameRx(rx);
	USB.begin();
}

void loop() {
}
