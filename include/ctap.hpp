#pragma once

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <variant>
#include <expected>
#include <vector>

#define CTAP_HID_INIT 0x06

typedef struct Unicast Unicast;

struct Init {
    uint8_t nonce[8];
    uint16_t bcnt;
    std::vector<uint8_t> data;

    void print() const {
        // printf("Nonce: 0x%lx\n", nonce);
    }

    static Init response(Unicast &uni, uint8_t nonce[8]);
};

using CtapHidCommand = std::variant<Init>;

struct Unicast {
    uint32_t cid;
    static Unicast random();
};

struct Broadcast {
};

using ChannelIdentifier = std::variant<Unicast, Broadcast>;

struct CtapHidRequest {
    ChannelIdentifier cid;
    CtapHidCommand cmd;
};

class CTAPTransport {
  public:
    virtual ~CTAPTransport() = default;
    virtual void send() const = 0;
};
    
class CTAP {
    public:
        CTAP(CTAPTransport &_trans);

        CTAPTransport &trans;

        std::expected<void, std::string> handle(CtapHidRequest &req);
        std::expected<void, std::string> handleInit(ChannelIdentifier &cid, Init &init);
        void send(ChannelIdentifier &cid, CtapHidCommand &cmd);
};

// payloadの先頭はchannel identifierを指すことを仮定
std::expected<ChannelIdentifier, std::string> parseCtapHidChannelIdentifier(uint8_t *cid_ptr);

// payloadの先頭はchannel identifierを指すことを仮定
std::expected<CtapHidRequest, std::string> parseCtapHidRequest(uint8_t *payload, size_t payload_size);

// cmd_identの先頭はcommand identifierを指すことを前提
std::expected<CtapHidCommand, std::string> parseCtapHidCommand(uint8_t *cmd_ident, size_t cmd_ident_size);
