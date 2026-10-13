#include "ctap.hpp"
#include <cstring>
#include <expected>
#include <format>
#include <random>
#include <variant>

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

Init Init::response(Unicast &uni, uint8_t nonce[8]) {
    std::vector<uint8_t> data;

    data.insert(data.end(), nonce, nonce+8);

    // TODO: アドホックにブロードキャストアドレスを指定
    for (int i = 0; i < 4; i++) 
        data.push_back(0xff & (uni.cid >> (8 * i)));

    // CTAPHIDプロトコルバージョン識別子
    data.push_back(2);

    // あとはてけとー
    for (int i = 0; i < 4; i++)
        data.push_back(0);

    Init init = Init {
        .bcnt=17,
        .data = data,
    };

    memcpy(&init.nonce, nonce, 8);

    return init;
}

CTAP::CTAP(CTAPTransport &_trans) : trans(_trans) {
}

Unicast Unicast::random() {
    // 乱数生成器の初期化
    std::random_device rd;
    std::mt19937 gen(rd());

    // ブロードキャストを除外した上で乱数生成
    std::uniform_int_distribution<uint32_t> distrib(0, std::numeric_limits<uint32_t>::max() - 1);

    return Unicast {
        .cid = distrib(gen)
    };
}

std::expected<void, std::string> CTAP::handle(CtapHidRequest &req) {
    return std::visit(overloaded {
        [&](Init init) {
            return handleInit(req.cid, init);
        }
    }, req.cmd);
}


std::expected<void, std::string> CTAP::handleInit(ChannelIdentifier &cid, Init &init) {
    if (std::holds_alternative<Broadcast>(cid)) {
        ChannelIdentifier client_random_cid = Unicast::random();


    } else {
        return std::unexpected("unsupported pattern");
    }
}

void CTAP::send(ChannelIdentifier &cid, CtapHidCommand &cmd) {
    
}

std::expected<CtapHidRequest, std::string> parseCtapHidRequest(uint8_t *payload, size_t payload_size) {
    std::expected<ChannelIdentifier, std::string> cid = parseCtapHidChannelIdentifier(payload);
    if (!cid) return std::unexpected(cid.error());

    std::expected<CtapHidCommand, std::string> cmd = parseCtapHidCommand(&payload[4], payload_size - 4);
    if (!cmd) return std::unexpected(cmd.error());

    return CtapHidRequest {
        .cid=*cid, .cmd=*cmd
    };
}

std::expected<ChannelIdentifier, std::string> parseCtapHidChannelIdentifier(uint8_t *cid_ptr) {
    uint32_t cid = 0;
    for (int i = 0; i < 4; i++) {
        cid |= static_cast<uint32_t>(
            static_cast<uint32_t>(cid_ptr[i]) << (8 * i)
        );
    }

    if (cid == 0xffffffff) {
        return Broadcast {};
    } else {
        return Unicast {.cid = cid};
    }
}

std::expected<CtapHidCommand, std::string> parseCtapHidCommand(uint8_t *cmd_ident, size_t cmd_ident_size) {
    // CMDは最上位ビットが常に1なので0にしておく
    uint8_t command_identifier = cmd_ident[0] & (~0x80);
    switch (command_identifier) {
        case CTAP_HID_INIT: {
            Init ret = Init{};
            memcpy(&ret.nonce, &cmd_ident[3], 8);

            return ret;
        }
        default:
            return std::unexpected(std::format("unsupported CTAPHID Commands: {}", command_identifier));
            break;
    }
}