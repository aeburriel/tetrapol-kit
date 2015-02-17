#pragma once

#include <tetrapol/addr.h>
#include <tetrapol/system_config.h>

#include <stdbool.h>
#include <stdint.h>

/// PAS 0001-3-3 7.5.4.2
typedef struct {
    uint8_t cmd;
    uint8_t mask;
} command_mask_t;

enum {
    COMMAND_INFORMATION = 0x00,
    COMMAND_SUPERVISION_RR = 0x01,
    COMMAND_SUPERVISION_RNR = 0x05,
    COMMAND_SUPERVISION_REJ = 0x09,
    COMMAND_DACH = 0x0d,
    COMMAND_UNNUMBERED_UI = 0x03,
    COMMAND_UNNUMBERED__BLANK1 = 0x23,  ///< suspicious blank line in table
    COMMAND_UNNUMBERED_DISC = 0x43,
    COMMAND_UNNUMBERED_UA =  0x53,
    COMMAND_UNNUMBERED_SNRM = 0x83,
    COMMAND_UNNUMBERED_UI_CD = 0xa3,
    COMMAND_UNNUMBERED_UI_VCH = 0xc3,
    COMMAND_UNNUMBERED__BLANK2 = 0xe3, ///< suspicious blank line in table
    COMMAND_UNNUMBERED_UI_P0 = 0x07,
    COMMAND_UNNUMBERED_U_RR = 0x27,
    COMMAND_UNNUMBERED_FRMR = 0x87,
    COMMAND_UNNUMBERED__BLANK3 = 0x0b, ///< suspicious blank line in table
    COMMAND_UNNUMBERED_DM = 0x0f,
    //COMMAND_UNNUMBERED__BLANK4 = 0x0f, ///< suspicious blank line in table
};

typedef struct {
    uint8_t cmd;
    union {
        struct {
            uint8_t recv_seq_no;    ///< N(R)
            uint8_t send_seq_no;    ///< N(S)
            uint8_t p_e;            ///< P/E
        } information;

        struct {
            uint8_t recv_seq_no;    ///< N(R)
            uint8_t p_e;            ///< P/E
        } supervision;

        struct {
            uint8_t seq_no;         ///< N'(R) or N'(S)
            uint8_t retry;          ///< R
        } dach_access;

        struct {
            union {
                bool p_e;           ///< P, P/E, E
                bool ra;            ///< RA
            };
            uint8_t response_format;    ///< RSP
        } unnumbered;
    };
} command_t;

typedef struct {
    addr_t addr;
    command_t command;
    int nbits;          ///< lenght is in bits
    /// data packed into bytes
    /// [max_block_size * max_blocks_per_frame - addr - command - FCS]
    uint8_t data[SYS_PAR_N200_BYTES_MAX - 2 - 1 - 2];
} hdlc_frame_t;

extern const addr_t addr_all;

bool hdlc_frame_parse(hdlc_frame_t *hdlc_frame, const uint8_t *data, int len);
