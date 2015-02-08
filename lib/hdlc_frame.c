#include "hdlc_frame.h"
#include "bit_utils.h"
#include "misc.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

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
static bool check_fcs(const uint8_t *data, int nbits)
{
    // roll in firts 16 bites of data
    uint32_t crc = 0;
    uint8_t b = data[0];
    for (int i = 0; i < 8; ++i) {
        crc = (crc << 1) | (b & 1);
        b = b >> 1;
    }
    b = data[1];
    for (int i = 0; i < 8; ++i) {
        crc = (crc << 1) | (b & 1);
        b = b >> 1;
    }

    // invert first 16 bits of data
    crc ^= 0xffff;

    nbits -= 16;
    data += 2;
    for ( ; nbits > 0; ++data) {
        b = *data;
        for (int offs = 0; offs < 8 && nbits; ++offs, --nbits) {
            // shift data bits into CRC
            crc = (crc << 1) | (b & 1);
            b = b >> 1;
            if (crc & 0x10000) {
                // CRC with poly: x^16 + x^12 + x^5 + 1
                crc ^= 0x11021;
            }
        }
    }

    return !(crc ^ 0xffff);
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

bool hdlc_frame_parse(hdlc_frame_t *hdlc_frame, const uint8_t *data, int nbits)
{
    if (!check_fcs(data, nbits)) {
        return false;
    }

    addr_parse(&hdlc_frame->addr, data);
    command_parse(&hdlc_frame->command, data[2]);
    memcpy(hdlc_frame->info, data + 3, (nbits - 3*8 - 2*8 + 7) / 8);
    // nbits - HDLC_header_nbits - FCS_len
    hdlc_frame->info_nbits = nbits - 3*8 - 2*8;

    return true;
}

