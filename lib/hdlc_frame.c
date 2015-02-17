#define LOG_EFIX "hdlc"
#include <tetrapol/log.h>
#include <tetrapol/hdlc_frame.h>
#include <tetrapol/bit_utils.h>
#include <tetrapol/misc.h>

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

static void command_parse(command_t *cmd, uint8_t data)
{
    cmd->cmd = data;

    int cmd_idx = 0;
    for ( ; cmd_idx < ARRAY_LEN(commands); ++cmd_idx) {
        if ((data & commands[cmd_idx].mask) == commands[cmd_idx].cmd) {
            cmd->cmd = commands[cmd_idx].cmd;
            break;
        }
    }

    switch (cmd->cmd) {
        case COMMAND_INFORMATION:
            cmd->information.recv_seq_no    = get_bits(3, &data, 0);
            cmd->information.p_e            = get_bits(1, &data, 3);
            cmd->information.send_seq_no    = get_bits(3, &data, 4);
            break;

        case COMMAND_SUPERVISION_RR:
        case COMMAND_SUPERVISION_RNR:
        case COMMAND_SUPERVISION_REJ:
            cmd->supervision.recv_seq_no    = get_bits(3, &data, 0);
            cmd->supervision.p_e            = get_bits(1, &data, 3);
            break;

        case COMMAND_DACH:
            cmd->dach_access.seq_no         = get_bits(3, &data, 0);
            cmd->dach_access.retry          = get_bits(1, &data, 3);
            break;

        case COMMAND_UNNUMBERED_UI:
        case COMMAND_UNNUMBERED_DISC:
        case COMMAND_UNNUMBERED_UA:
        case COMMAND_UNNUMBERED_SNRM:
            cmd->unnumbered.p_e             = get_bits(1, &data, 3);
            break;

        case COMMAND_UNNUMBERED_UI_CD:
        case COMMAND_UNNUMBERED_UI_VCH:
            break;

        case COMMAND_UNNUMBERED_UI_P0:
            cmd->unnumbered.ra              = get_bits(1, &data, 3);
            break;

        case COMMAND_UNNUMBERED_U_RR:
            cmd->unnumbered.response_format = get_bits(2, &data, 0);
            cmd->unnumbered.p_e             = get_bits(1, &data, 3);
            break;

        case COMMAND_UNNUMBERED_FRMR:
        case COMMAND_UNNUMBERED_DM:
            cmd->unnumbered.p_e             = get_bits(1, &data, 3);
            break;

        case COMMAND_UNNUMBERED__BLANK1:
        case COMMAND_UNNUMBERED__BLANK2:
        case COMMAND_UNNUMBERED__BLANK3:
        default:
            LOG(WTF, "Unknown command 0x%02x", data);
    };
}

bool hdlc_frame_parse(hdlc_frame_t *hdlc_frame, const uint8_t *data, int nbits)
{
    if (!check_fcs(data, nbits)) {
        // TODO: check for stuffing frames, detect other broken frames
        return false;
    }

    addr_parse(&hdlc_frame->addr, data, 0);
    command_parse(&hdlc_frame->command, data[2]);
    // nbits - HDLC_header_nbits - FCS_len
    hdlc_frame->nbits = nbits - 3*8 - 2*8;
    memcpy(hdlc_frame->data, data + 3, (hdlc_frame->nbits + 7) / 8);

    return true;
}

