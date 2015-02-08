#include "hdlc_frame.h"
#include "bit_utils.h"
#include "misc.h"

#include <stdbool.h>
#include <stdio.h>

static const command_mask_t commands[] = {
    {   .cmd = COMMAND_INFORMATION,         .mask = 0x01 },
    {   .cmd = COMMAND_SUPERVISION_RR,      .mask = 0x0f },
    {   .cmd = COMMAND_SUPERVISION_RNR,     .mask = 0x0f },
    {   .cmd = COMMAND_SUPERVISION_REJ,     .mask = 0x0f },
    {   .cmd = COMMAND_DACH,                .mask = 0x0f },
    {   .cmd = COMMAND_UNNUMBERED_UI,       .mask = 0xef },
    {   .cmd = COMMAND_UNNUMBERED__BLANK1,  .mask = 0xef },
    {   .cmd = COMMAND_UNNUMBERED_DISC,     .mask = 0xef },
    {   .cmd = COMMAND_UNNUMBERED_UA,       .mask = 0xef },
    {   .cmd = COMMAND_UNNUMBERED_SNRM,     .mask = 0xef },
    {   .cmd = COMMAND_UNNUMBERED_UI_CD,    .mask = 0xff },
    {   .cmd = COMMAND_UNNUMBERED_UI_VCH,   .mask = 0xff },
    {   .cmd = COMMAND_UNNUMBERED__BLANK2,  .mask = 0xef },
    {   .cmd = COMMAND_UNNUMBERED_UI_P0,    .mask = 0xef },
    {   .cmd = COMMAND_UNNUMBERED_U_RR,     .mask = 0x2f },
    {   .cmd = COMMAND_UNNUMBERED_FRMR,     .mask = 0xef },
    {   .cmd = COMMAND_UNNUMBERED__BLANK3,  .mask = 0x0f },
    {   .cmd = COMMAND_UNNUMBERED_DM,       .mask = 0xef },
    //{   .cmd = COMMAND_UNNUMBERED__BLANK4,  .mask = 0xef },
};

/// PAS 0001-3-3 7.4.1.1
static bool check_fcs(const uint8_t *data, int len)
{
    uint8_t crc[16];

    // invert first 16 bits of data
    for (int i = 0; i < 16; ++i) {
        crc[i] = data[i] ^ 1;
    }

    // CRC with poly: x^16 + x^12 + x^5 + 1
    for (int i = 16; i < len; ++i) {
        int xor = crc[0];

        crc[0] = crc[1];
        crc[1] = crc[2];
        crc[2] = crc[3];
        crc[3] = crc[4] ^ xor;
        crc[4] = crc[5];
        crc[5] = crc[6];
        crc[6] = crc[7];
        crc[7] = crc[8];
        crc[8] = crc[9];
        crc[9] = crc[10];
        crc[10] = crc[11] ^ xor;
        crc[11] = crc[12];
        crc[12] = crc[13];
        crc[13] = crc[14];
        crc[14] = crc[15];
        // CRC at the end of frame is inverted, invert it again
        if (i >= len - 16) {
            crc[15] = data[i] ^ xor ^ 1;
        } else {
            crc[15] = data[i] ^ xor;
        }
    }

    return !(crc[0] | crc[1] | crc[2] | crc[3] | crc[4] | crc[5] | crc[6] | crc[7] |
            crc[8] | crc[9] | crc[10] | crc[11] | crc[12] | crc[13] | crc[14] | crc[15]);
}

static void command_parse(command_t *cmd, uint8_t data)
{
    int cmd_idx = 0;
    for ( ; cmd_idx < ARRAY_LEN(commands); ++cmd_idx) {
        if ((data & commands[cmd_idx].mask) == commands[cmd_idx].cmd) {
            cmd->cmd = commands[cmd_idx].cmd;
            break;
        }
    }
    if (cmd_idx >= ARRAY_LEN(commands)) {
        cmd->cmd = data;
        return;
    }

    switch (cmd->cmd) {
        case COMMAND_INFORMATION:
            GET_BITS(3, 1, &data, cmd->information.send_seq_no);

        case COMMAND_SUPERVISION_RR:
        case COMMAND_SUPERVISION_RNR:
        case COMMAND_SUPERVISION_REJ:
            GET_BITS(1, 4, &data, cmd->information.p_e);
            GET_BITS(3, 5, &data, cmd->information.recv_seq_no);
            break;

        case COMMAND_DACH:
            GET_BITS(1, 4, &data, cmd->dach_access.retry);
            GET_BITS(3, 5, &data, cmd->dach_access.seq_no);
            break;

        case COMMAND_UNNUMBERED_UI:
        case COMMAND_UNNUMBERED_DISC:
        case COMMAND_UNNUMBERED_UA:
        case COMMAND_UNNUMBERED_SNRM:
            GET_BITS(1, 4, &data, cmd->dach_access.retry);
            break;

        case COMMAND_UNNUMBERED_UI_CD:
        case COMMAND_UNNUMBERED_UI_VCH:
            break;

        case COMMAND_UNNUMBERED_UI_P0:
            GET_BITS(1, 4, &data, cmd->unnumbered.ra);
            break;

        case COMMAND_UNNUMBERED_U_RR:
            GET_BITS(1, 4, &data, cmd->unnumbered.p_e);
            GET_BITS(2, 6, &data, cmd->unnumbered.response_format);
            break;

        case COMMAND_UNNUMBERED_FRMR:
        case COMMAND_UNNUMBERED_DM:
            GET_BITS(1, 4, &data, cmd->unnumbered.p_e);
            break;

        case COMMAND_UNNUMBERED__BLANK1:
        case COMMAND_UNNUMBERED__BLANK2:
        case COMMAND_UNNUMBERED__BLANK3:
        default:
            printf("Unknown command %02x\n", data);
            cmd->_reserved= data;
    };
}

// converts array of bits into bytes, uses TETRAPOL bite order
static void pack_bits(uint8_t *bytes, const uint8_t *bits, int nbits)
{
    int nbytes = nbits / 8;
    for (int i = 0; i < nbytes; ++i) {
        bytes[i] =
            (bits[8*i + 0] << 0) |
            (bits[8*i + 1] << 1) |
            (bits[8*i + 2] << 2) |
            (bits[8*i + 3] << 3) |
            (bits[8*i + 4] << 4) |
            (bits[8*i + 5] << 5) |
            (bits[8*i + 6] << 6) |
            (bits[8*i + 7] << 7);
    }
    nbits %= 8;
    if (nbits) {
        bytes[nbytes] = 0;
        for (int i = 0; i < nbits; ++i) {
            bytes[nbytes] |= (bits[8*nbytes + i] << i);
        }
    }
}

bool hdlc_frame_parse(hdlc_frame_t *hdlc_frame, const uint8_t *data, int len)
{
    if (!check_fcs(data, len)) {
        return false;
    }

    uint8_t buf[3];
    pack_bits(buf, data, 3*8);

    addr_parse(&hdlc_frame->addr, buf);
    // TODO: proper command parsing
    command_parse(&hdlc_frame->command, buf[2]);

    pack_bits(hdlc_frame->info, data + 3*8, len - 3*8 - 2*8);
    // len - HDLC_header_len - FCS_len
    hdlc_frame->info_nbits = len - 3*8 - 2*8;

    return true;
}

