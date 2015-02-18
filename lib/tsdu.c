#define LOG_PREFIX "tsdu"
#include <tetrapol/log.h>
#include <tetrapol/tsdu.h>
#include <tetrapol/misc.h>
#include <tetrapol/bit_utils.h>
#include <tetrapol/phys_ch.h>
#include <tetrapol/misc.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define CHECK_LEN(len, min_len, tsdu) \
    if ((len) < (min_len)) { \
        LOG(ERR, "%d data too short %d < %d", __LINE__, (len), (min_len)); \
        tsdu_destroy((tsdu_base_t *)tsdu); \
        return NULL; \
    }

static const char *codop_str[256] = {
    "N/A",                          // 0x00,
    "N/A",                          // 0x01,
    "N/A",                          // 0x02,
    "N/A",                          // 0x03,
    "N/A",                          // 0x04,
    "N/A",                          // 0x05,
    "N/A",                          // 0x06,
    "N/A",                          // 0x07,
    "D_REJECT",                     // 0x08,
    "D_REFUSAL",                    // 0x09,
    "U_END",                        // 0x0a,
    "D_BACK_CCH",                   // 0x0b,
    "D_RELEASE",                    // 0x0c,
    "U_ABORT",                      // 0x0d,
    "U_TERMINATE",                  // 0x0e,
    "D_HOOK_ON_INVITATION",         // 0x0f,
    "D_RETURN",                     // 0x10,
    "U_EVENT_REPORT",               // 0x11,
    "D_CALL_WAITING",               // 0x12,
    "D_AUTHENTICATION",             // 0x13,
    "U_AUTHENTICATION",             // 0x14,
    "N/A",                          // 0x15,
    "D_AUTHORISATION",              // 0x16,
    "U_ERROR_REPORT",               // 0x17,
    "D_CHANNEL_INIT",               // 0x18,
    "N/A",                          // 0x19,
    "N/A",                          // 0x1a,
    "N/A",                          // 0x1b,
    "N/A",                          // 0x1c,
    "N/A",                          // 0x1d,
    "N/A",                          // 0x1e,
    "N/A",                          // 0x1f,
    "U_REGISTRATION_REQ",           // 0x20,
    "D_REGISTRATION_NAK",           // 0x21,
    "D_REGISTRATION_ACK",           // 0x22,
    "D_FORCED_REGISTRATION",        // 0x23,
    "U_LOCATION_ACTIVITY",          // 0x24,
    "D_LOCATION_ACTIVITY_ACK",      // 0x25,
    "N/A",                          // 0x26,
    "N/A",                          // 0x27,
    "N/A",                          // 0x28,
    "N/A",                          // 0x29,
    "N/A",                          // 0x2a,
    "N/A",                          // 0x2b,
    "N/A",                          // 0x2c,
    "N/A",                          // 0x2d,
    "N/A",                          // 0x2e,
    "N/A",                          // 0x2f,
    "U_CALL_SETUP",                 // 0x30,
    "D_CALL_ALERT",                 // 0x31,
    "D_CALL_SETUP",                 // 0x32,
    "U_CALL_ANSWER",                // 0x33,
    "D_CALL_CONNECT",               // 0x34,
    "D_CALL_SWITCH",                // 0x35,
    "U_CALL_INTRUSION_PC",          // 0x36,
    "U_CALL_INTRUSION_OCH",         // 0x37,
    "N/A",                          // 0x38,
    "D_TRANSFER_NAK",               // 0x39,
    "U_TRANSFER_REQ",               // 0x3a,
    "U_CALL_INTRUSION_ECH",         // 0x3b,
    "U_CALL_RELEASE",               // 0x3c,
    "U_CALL_CONNECT",               // 0x3d,
    //"U_CALL_SWITCH",                // 0x3d,
    "D_CALL_START",                 // 0x3e,
    "N/A",                          // 0x3f,
    "N/A",                          // 0x40,
    "N/A",                          // 0x41,
    "D_FUNCTIONAL_SHORT_DATA",      // 0x42,
    "U_DATA_DOWN_ACCEPT",           // 0x43,
    "U_DATA_MSG_UP",                // 0x44,
    "D_DATA_MSG_DOWN",              // 0x45,
    "D_EXPLICIT_SHORT_DATA",        // 0x46,
    "D_SEECRET_0x47",               // 0x47,
    "D_DATA_END",                   // 0x48,
    "D_DATAGRAM_NOTIFY",            // 0x49,
    "D_DATAGRAM",                   // 0x4a,
    "D_BROADCAST",                  // 0x4b,
    "D_DATA_SERV",                  // 0x4c,
    "N/A",                          // 0x4d,
    "D_DATA_DOWN_STATUS",           // 0x4e,
    "N/A",                          // 0x4f,
    "U_OCH_RELEASE",                // 0x50,
    "U_OCH_SETUP",                  // 0x51,
    "U_ECH_CLOSE",                  // 0x52,
    "D_EMERGENCY_NOTIFICATION",     // 0x53,
    "U_ECH_SETUP",                  // 0x54,
    "D_GROUP_ACTIVATION",           // 0x55,
    "D_ECH_ACTIVATION",             // 0x56,
    "D_GROUP_END",                  // 0x57,
    "D_GROUP_IDLE",                 // 0x58,
    "D_GROUP_REJECT",               // 0x59,
    "D_ECH_REJECT",                 // 0x5a,
    "D_GROUP_PAGING",               // 0x5b,
    "D_BROADCAST_NOTIFICATION",     // 0x5c,
    "D_CRISIS_NOTIFICATION",        // 0x5d,
    "N/A",                          // 0x5e,
    "D_EMERGENCY_ACK",              // 0x5f,
    "D_CONNECT_DCH",                // 0x60,
    "N/A",                          // 0x61,
    "D_CONNECT_CCH",                // 0x62,
    "D_DATA_AUTHENTICATION",        // 0x63,
    "D_DATA_REQUEST",               // 0x64,
    "D_DCH_OPEN",                   // 0x65,
    "U_DATA_REQUEST",               // 0x66,
    "D_EXTENDED_STATUS",            // 0x67,
    "D_CCH_OPEN",                   // 0x68,
    "D_BROADCAST_WAITING",          // 0x69,
    "N/A",                          // 0x6a,
    "N/A",                          // 0x6b,
    "N/A",                          // 0x6c,
    "N/A",                          // 0x6d,
    "N/A",                          // 0x6e,
    "N/A",                          // 0x6f,
    "D_ACCESS_DISABLED",            // 0x70,
    "D_TRAFFIC_ENABLED",            // 0x71,
    "D_TRAFFIC_DISABLED",           // 0x72,
    "U_DEVIATION_CLEAR",            // 0x73,
    "U_DEVIATION_SET",              // 0x74,
    "N/A",                          // 0x75,
    "D_DEVIATION_ON",               // 0x76,
    "D_ABILITY_MNGT",               // 0x77,
    "D_SERVICE_DISABLED",           // 0x78,
    "N/A",                          // 0x79,
    "N/A",                          // 0x7a,
    "N/A",                          // 0x7b,
    "N/A",                          // 0x7c,
    "N/A",                          // 0x7d,
    "N/A",                          // 0x7e,
    "N/A",                          // 0x7f,
    "D_EMERGENCY_NAK",              // 0x80,
    "U_EMERGENCY_REQ",              // 0x81,
    "D_GROUP_OVERLOAD_ID",          // 0x82,
    "D_ECH_OVERLOAD_ID",            // 0x83,
    "D_PRIORITY_GRP_WAITING",       // 0x84,
    "D_PRIORITY_GRP_ACTIVATION",    // 0x85,
    "D_OC_ACTIVATION",              // 0x86,
    "D_OC_REJECT",                  // 0x87,
    "D_OC_PAGING",                  // 0x88,
    "N/A",                          // 0x89,
    "N/A",                          // 0x8a,
    "N/A",                          // 0x8b,
    "N/A",                          // 0x8c,
    "N/A",                          // 0x8d,
    "N/A",                          // 0x8e,
    "N/A",                          // 0x8f,
    "D_SYSTEM_INFO",                // 0x90,
    "N/A",                          // 0x91,
    "D_GROUP_LIST",                 // 0x92,
    "D_GROUP_COMPOSITION",          // 0x93,
    "D_NEIGHBOURING_CELL",          // 0x94,
    "D_ECCH_DESCRIPTION",           // 0x95,
    "D_ADDITIONAL_PARTICIPANTS",    // 0x96,
    "D_RESERVED_0x97",              // 0x97,
    "N/A",                          // 0x98,
    "N/A",                          // 0x99,
    "N/A",                          // 0x9a,
    "N/A",                          // 0x9b,
    "N/A",                          // 0x9c,
    "N/A",                          // 0x9d,
    "N/A",                          // 0x9e,
    "N/A",                          // 0x9f,
    "N/A",                          // 0xa0,
    "N/A",                          // 0xa1,
    "N/A",                          // 0xa2,
    "N/A",                          // 0xa3,
    "N/A",                          // 0xa4,
    "N/A",                          // 0xa5,
    "N/A",                          // 0xa6,
    "N/A",                          // 0xa7,
    "N/A",                          // 0xa8,
    "N/A",                          // 0xa9,
    "N/A",                          // 0xaa,
    "N/A",                          // 0xab,
    "N/A",                          // 0xac,
    "N/A",                          // 0xad,
    "N/A",                          // 0xae,
    "N/A",                          // 0xaf,
    "N/A",                          // 0xb0,
    "N/A",                          // 0xb1,
    "N/A",                          // 0xb2,
    "N/A",                          // 0xb3,
    "N/A",                          // 0xb4,
    "N/A",                          // 0xb5,
    "N/A",                          // 0xb6,
    "N/A",                          // 0xb7,
    "N/A",                          // 0xb8,
    "N/A",                          // 0xb9,
    "N/A",                          // 0xba,
    "N/A",                          // 0xbb,
    "N/A",                          // 0xbc,
    "N/A",                          // 0xbd,
    "N/A",                          // 0xbe,
    "N/A",                          // 0xbf,
    "N/A",                          // 0xc0,
    "N/A",                          // 0xc1,
    "N/A",                          // 0xc2,
    "N/A",                          // 0xc3,
    "N/A",                          // 0xc4,
    "D_INFORMATION_DELIVERY",       // 0xc5,
    "N/A",                          // 0xc6,
    "N/A",                          // 0xc7,
    "N/A",                          // 0xc8,
    "N/A",                          // 0xc9,
    "N/A",                          // 0xca,
    "N/A",                          // 0xcb,
    "N/A",                          // 0xcc,
    "N/A",                          // 0xcd,
    "N/A",                          // 0xce,
    "N/A",                          // 0xcf,
    "N/A",                          // 0xd0,
    "N/A",                          // 0xd1,
    "N/A",                          // 0xd2,
    "N/A",                          // 0xd3,
    "N/A",                          // 0xd4,
    "N/A",                          // 0xd5,
    "N/A",                          // 0xd6,
    "N/A",                          // 0xd7,
    "N/A",                          // 0xd8,
    "N/A",                          // 0xd9,
    "N/A",                          // 0xda,
    "N/A",                          // 0xdb,
    "N/A",                          // 0xdc,
    "N/A",                          // 0xdd,
    "N/A",                          // 0xde,
    "N/A",                          // 0xdf,
    "D_CALL_ACTIVATION",            // 0xe0,
    "D_CALL_COMPOSITION",           // 0xe1,
    "D_CALL_END",                   // 0xe2,
    "D_CALL_OVERLOAD_ID",           // 0xe3,
    "N/A",                          // 0xe4,
    "N/A",                          // 0xe5,
    "N/A",                          // 0xe6,
    "N/A",                          // 0xe7,
    "N/A",                          // 0xe8,
    "N/A",                          // 0xe9,
    "N/A",                          // 0xea,
    "N/A",                          // 0xeb,
    "N/A",                          // 0xec,
    "N/A",                          // 0xed,
    "N/A",                          // 0xee,
    "N/A",                          // 0xef,
    "N/A",                          // 0xf0,
    "N/A",                          // 0xf1,
    "N/A",                          // 0xf2,
    "N/A",                          // 0xf3,
    "N/A",                          // 0xf4,
    "N/A",                          // 0xf5,
    "N/A",                          // 0xf6,
    "N/A",                          // 0xf7,
    "N/A",                          // 0xf8,
    "N/A",                          // 0xf9,
    "N/A",                          // 0xfa,
    "N/A",                          // 0xfb,
    "N/A",                          // 0xfc,
    "N/A",                          // 0xfd,
    "N/A",                          // 0xfe,
    "N/A",                          // 0xff,
};

const int CELL_RADIO_PARAM_PWR_TX_ADJUST_TO_DBM[16] = {
    -76, -72, -68, -64, -60, -56, -52, -48,
    -44, -40, -36, -32, -28, -24, -20, -16,
};

const int CELL_RADIO_PARAM_RX_LEV_ACCESS_TO_DBM[16] = {
    -92, -88, -84, -80, -76, -72, -68, -64,
    -60, -56, -52, -48, -44, -40, -36, -32,
};

void tsdu_destroy(tsdu_base_t *tsdu)
{
    if (!tsdu) {
        return;
    }
    for (int i = 0; i < tsdu->noptionals; ++i) {
        free(tsdu->optionals[i]);
    }
    free(tsdu);
}

static void tsdu_base_set_nopts(tsdu_base_t *tsdu, int noptionals)
{
    tsdu->noptionals = noptionals;
    memset(tsdu->optionals, 0, noptionals * sizeof(void *));
}

static void activation_mode_decode(activation_mode_t *am, uint8_t data)
{
    am->hook = get_bits(2, &data, 0);
    am->type = get_bits(2, &data, 2);
}

static void cell_id_decode1(cell_id_t *cell_id, const uint8_t *data)
{
    int type = get_bits(2, data, 0);
    if (type == CELL_ID_FORMAT_0) {
        cell_id->bs_id = get_bits(6, data, 2);
        cell_id->rws_id = get_bits(4, data, 8);
    } else if (type == CELL_ID_FORMAT_1) {
        cell_id->bs_id = get_bits(4, data, 8);
        cell_id->rws_id = get_bits(6, data, 2);
    } else {
        LOG(WTF, "unknown cell_id_type (%d)", type);
    }
}

// specific for d_system_info - cell in offline mode
static void cell_id_decode2(cell_id_t *cell_id, const uint8_t *data)
{
    int type = get_bits(2, data, 8);
    if (type == CELL_ID_FORMAT_0) {
        cell_id->bs_id = get_bits(6, data, 10);
        cell_id->rws_id = get_bits(4, data, 4);;
    } else if (type == CELL_ID_FORMAT_1) {
        cell_id->bs_id = get_bits(4, data, 4);;
        cell_id->rws_id = get_bits(6, data, 10);;
    } else {
        LOG(WTF, "unknown cell_id_type (%d)", type);
    }
}

static tsdu_d_group_activation_t *
d_group_activation_decode(const uint8_t *data, int nbits)
{
    tsdu_d_group_activation_t *tsdu = malloc(sizeof(tsdu_d_group_activation_t));
    if (!tsdu) {
        return NULL;
    }

    tsdu_base_set_nopts(&tsdu->base, 0);
    CHECK_LEN(nbits, 9 * 8, tsdu);

    int _zero0;
    activation_mode_decode(&tsdu->activation_mode, data[1]);
    tsdu->group_id              = get_bits(12, data + 1, 4);
    tsdu->coverage_id           = get_bits(8,  data + 3, 0);
    _zero0                      = get_bits(4,  data + 4, 0);
    tsdu->channel_id            = get_bits(12, data + 4, 4);
    tsdu->u_ch_scrambling       = get_bits(8,  data + 6, 0);
    tsdu->d_ch_scrambling       = get_bits(8,  data + 7, 0);
    tsdu->key_reference._data   = get_bits(8,  data + 8, 0);

    if (_zero0 != 0) {
        LOG(WTF, "nonzero padding: 0x%02x", _zero0);
    }

    tsdu->has_addr_tti = false;
    if (nbits >= 12 * 8) {
        // FIXME: proper IEI handling
        uint8_t iei = get_bits(8, data + 9, 0);
        if (iei != IEI_TTI) {
            LOG(WTF, "expected IEI_TTI got %d", iei);
        } else {
            tsdu->has_addr_tti = true;
            addr_parse(&tsdu->addr_tti, &data[10], 0);
        }
    }

    if (nbits > 12*8) {
        LOG(WTF, "unused bits (%d)", nbits);
    }

    return tsdu;
}

static void d_group_activation_print(tsdu_d_group_activation_t *tsdu)
{
    printf("\tCODOP=0x%02x (D_GROUP_ACTIVATION)\n", D_GROUP_ACTIVATION);
    printf("\t\tACTIVATION_MODE: HOOK=%d TYPE=%d\n",
           tsdu->activation_mode.hook, tsdu->activation_mode.type);
    printf("\t\tGROUP_ID=%d\n", tsdu->group_id);
    printf("\t\tCOVERAGE_ID=%d\n", tsdu->coverage_id);
    printf("\t\tCHANNEL_ID=%d\n", tsdu->channel_id);
    printf("\t\tU_CH_SCRAMBLING=%d\n", tsdu->u_ch_scrambling);
    printf("\t\tD_CH_SCRAMBLING=%d\n", tsdu->d_ch_scrambling);
    printf("\t\tKEY_REFERENCE: KEY_TYPE=%i KEY_INDEX=%i\n",
           tsdu->key_reference.key_type, tsdu->key_reference.key_index);
    if (tsdu->has_addr_tti) {
        printf("\t\tADDR_TTI=");
        addr_print(&tsdu->addr_tti);
        printf("\n");
    }
}

static tsdu_d_group_list_t *d_group_list_decode(const uint8_t *data, int nbits)
{
    tsdu_d_group_list_t *tsdu = malloc(sizeof(tsdu_d_group_list_t));
    if (!tsdu) {
        return NULL;
    }

    tsdu_base_set_nopts(&tsdu->base, 3);
    tsdu->nemergency = 0;
    tsdu->ngroup = 0;
    tsdu->nopen = 0;

    int rlen = 2*8; ///< required data length
    CHECK_LEN(nbits, rlen, tsdu);
    tsdu->reference_list._data = get_bits(8, data + 1, 0);
    if (tsdu->reference_list.revision == 0) {
        return tsdu;
    }

    rlen += 1*8;
    CHECK_LEN(nbits, rlen, tsdu);
    tsdu->index_list._data = get_bits(8, data + 2, 0);
    data += 3;
    do {
        rlen += 1*8;
        CHECK_LEN(nbits, rlen, tsdu);
        const type_nb_t type_nb = {
            ._data = get_bits(8, data, 0),
        };
        if (type_nb.type == TYPE_NB_TYPE_END) {
            break;
        }
        data += 1;

        if (type_nb.type == TYPE_NB_TYPE_EMERGENCY) {
            const int n = tsdu->nemergency + type_nb.number;
            const int l = sizeof(tsdu_d_group_list_emergency_t) * n;
            tsdu_d_group_list_emergency_t *p = realloc(tsdu->emergency, l);
            if (!p) {
                tsdu_destroy(&tsdu->base);
                return NULL;
            }
            tsdu->emergency = p;
            for ( ; tsdu->nemergency < n; ++tsdu->nemergency) {
                const int i = tsdu->nemergency;
                cell_id_decode1(&tsdu->emergency[i].cell_id, data);
                int zero = get_bits(4, data + 1, 4);
                if (zero != 0) {
                    LOG(WTF, "nonzero padding (%d)", zero);
                }
                data += 2;
                rlen += 2*8;
            }
        }

        if (type_nb.type == TYPE_NB_TYPE_OPEN) {
            const int n = tsdu->nopen + type_nb.number;
            const int l = sizeof(tsdu_d_group_list_open_t) * n;
            tsdu_d_group_list_open_t *p = realloc(tsdu->open,l);
            if (!p) {
                tsdu_destroy(&tsdu->base);
                return NULL;
            }
            tsdu->open = p;
            for ( ; tsdu->nopen < n; ++tsdu->nopen) {
                const int i = tsdu->nopen;
                tsdu->open[i].coverage_id           = get_bits(8, data, 0);
                tsdu->open[i].call_priority         = get_bits(4, data + 1, 0);
                tsdu->open[i].group_id              = get_bits(12, data + 1, 4);
                uint8_t padding                     = get_bits(2, data + 3, 0);
                if (padding != 0) {
                    LOG(WTF, "nonzero padding (%d)", padding);
                }
                tsdu->open[i].och_parameters.add    = get_bits(1, data + 3, 2);
                tsdu->open[i].och_parameters.mbn    = get_bits(1, data + 3, 3);
                tsdu->open[i].neighbouring_cell     = get_bits(12, data + 3, 4);
                data += 5;
                rlen +=  5*8;
            }
        }
        if (type_nb.type == TYPE_NB_TYPE_TALK_GROUP) {
            const int n = tsdu->ngroup + type_nb.number;
            const int l = sizeof(tsdu_d_group_list_talk_group_t) * n;
            tsdu_d_group_list_talk_group_t *p = realloc(tsdu->group, l);
            if (!p) {
                tsdu_destroy(&tsdu->base);
                return NULL;
            }
            tsdu->group = p;
            for ( ; tsdu->ngroup < n; ++tsdu->ngroup) {
                const int i = tsdu->ngroup;
                tsdu->group[i].coverage_id          = get_bits(8, data, 0);
                uint8_t zero                        = get_bits(8, data + 1, 0);
                if (zero != 0) {
                    LOG(WTF, "nonzero padding in talk group-1 (%d)", zero);
                }
                uint8_t padding                     = get_bits(4, data + 2, 0);
                if (padding != 0) {
                    LOG(WTF, "nonzero padding in talk group-2 (%d)", padding);
                }
                tsdu->group[i].neighbouring_cell    = get_bits(12, data + 2, 4);
                data += 4;
                rlen += 4*8;
            }
        }
    } while(true);

    return tsdu;
}

static void d_group_list_print(tsdu_d_group_list_t *tsdu)
{
    printf("\tCODOP=0x%02x (D_GROUP_LIST)\n", D_GROUP_LIST);
    printf("\t\tREFERENCE_LIST REVISION=%d CSG=%d CSO=%d DC=%d\n",
           tsdu->reference_list.revision, tsdu->reference_list.csg,
           tsdu->reference_list.cso, tsdu->reference_list.dc);
    if (tsdu->reference_list.revision == 0) {
        return;
    }
    printf("\t\tINDEX_LIST MODE=%d INDEX=%d\n",
           tsdu->index_list.mode, tsdu->index_list.index);

    if (tsdu->nopen) {
        printf("\t\tOCH\n");
        for (int i = 0; i < tsdu->nopen; ++i) {
            printf("\t\t\tCOVERAGE_ID=%d CALL_PRIORITY=%d GROUP_ID=%d "
                   "OCH_PARAMETERS.ADD=%d OCH_PARAMETERS.MBN=%d "
                   "NEIGBOURING_CELL=%d\n",
                   tsdu->open[i].coverage_id,
                   tsdu->open[i].call_priority,
                   tsdu->open[i].group_id,
                   tsdu->open[i].och_parameters.add,
                   tsdu->open[i].och_parameters.mbn,
                   tsdu->open[i].neighbouring_cell);
        }
    }

    if (tsdu->ngroup) {
        printf("\t\tGROUP\n");
        for (int i = 0; i < tsdu->ngroup; ++i) {
            printf("\t\t\tCOVERAGE_ID=%d NEIGHBOURING_CALL=%d\n",
                   tsdu->group[i].coverage_id, tsdu->group[i].neighbouring_cell);
        }
    }

    if (tsdu->nemergency) {
        printf("\t\t\tEMERGENCY\n");
        for (int i = 0; i < tsdu->nemergency; ++i) {
            printf("\t\t\tCELL_ID.BS_ID=%d CELL_ID.RWS_ID=%d\n",
                   tsdu->emergency[i].cell_id.bs_id, tsdu->emergency[i].cell_id.rws_id);
        }
    }
}

static tsdu_d_group_composition_t *d_group_composition_decode(const uint8_t *data, int nbits)
{
    tsdu_d_group_composition_t *tsdu = malloc(sizeof(tsdu_d_group_composition_t));
    if (!tsdu) {
        return NULL;
    }

    tsdu_base_set_nopts(&tsdu->base, 0);
    CHECK_LEN(nbits, 3*8, tsdu);

    tsdu->group_id = get_bits(12, data + 1, 0);
    tsdu->og_nb = get_bits(4, data + 2, 4);

    CHECK_LEN(nbits, 3*8 + 12*tsdu->og_nb, tsdu);

    int skip = 0;
    for (int i = 0; i < tsdu->og_nb; ++i) {
        tsdu->group_ids[i] = get_bits(12, data + 3, skip);
        skip += 12;
    }

    return tsdu;
}

static void d_group_composition_print(tsdu_d_group_composition_t *tsdu)
{
    printf("\tCODOP=0x%02x (D_GROUP_COMPOSITION)\n", D_GROUP_COMPOSITION);
    printf("\t\tGROUP_ID=%d\n", tsdu->group_id);
    for (int i = 0; i < tsdu->og_nb; ++i) {
        printf("\t\tGROUP_ID=%d\n", tsdu->group_ids[i]);
    }
}

static cell_id_list_t *iei_cell_id_list_decode(
        cell_id_list_t *cell_ids, const uint8_t *data, int len)
{
    int n = 0;
    if (cell_ids) {
        n = cell_ids->len;
    }
    n += len / 2;
    const int l = sizeof(cell_id_list_t) + n * sizeof(cell_id_t);
    cell_id_list_t *p = realloc(cell_ids, l);
    if (!p) {
        LOG(ERR, "ERR OOM");
        return NULL;
    }
    if (!cell_ids) {
        p->len = 0;
    }
    cell_ids = p;

    for ( ; cell_ids->len < n; ++cell_ids->len) {
        cell_id_decode1(&cell_ids->cell_ids[cell_ids->len], data);
        data += 2;
    }

    return cell_ids;
}

addr_list_t *iei_adjacent_bn_list_decode(
        addr_list_t *adj_cells, const uint8_t *data, int len)
{
    int n = 0;
    if (adj_cells) {
        n = adj_cells->len;
    }
    n +=  len * 2 / 3;
    const int l = sizeof(addr_list_t) + n * sizeof(addr_t);
    addr_list_t *p = realloc(adj_cells, l);
    if (!p) {
        LOG(ERR, "ERR OOM");
        return NULL;
    }
    if (!adj_cells) {
        p->len = 0;
    }
    adj_cells = p;

    for (int skip = 0; adj_cells->len < n; ++adj_cells->len) {
        addr_parse(&adj_cells->addrs[adj_cells->len], data, skip);
        skip += 12;
    }

    return adj_cells;
}

static tsdu_d_neighbouring_cell_t *d_neighbouring_cell_decode(const uint8_t *data, int nbits)
{
    tsdu_d_neighbouring_cell_t *tsdu = malloc(sizeof(tsdu_d_neighbouring_cell_t));
    if (!tsdu) {
        return NULL;
    }
    tsdu_base_set_nopts(&tsdu->base, 2);
    CHECK_LEN(nbits, 2*8, tsdu);

    uint8_t _zero                               = get_bits(4, data + 1, 0);
    tsdu->ccr_config.number                     = get_bits(4, data + 1, 4);
    if (_zero != 0) {
        LOG(WTF, "d_neighbouring_cell padding != 0 (%d)", _zero);
    }

    if (!tsdu->ccr_config.number) {
        return tsdu;
    }

    tsdu->ccr_param = data[2];
    if (tsdu->ccr_param) {
        LOG(WTF, "d_neighbouring_cell ccr_param != 0 (%d)", tsdu->ccr_param);
    }

    data += 3;
    nbits -= 3 * 8;
    CHECK_LEN(nbits, 3 * 8 * tsdu->ccr_config.number, tsdu);
    for (int i = 0; i < tsdu->ccr_config.number; ++i) {
        tsdu->adj_cells[i].bn_nb                = get_bits(4,  data, 0);
        tsdu->adj_cells[i].channel_id           = get_bits(12, data, 4);
        tsdu->adj_cells[i].adjacent_param._data = get_bits(8,  data + 2, 0);
        if (tsdu->adj_cells[i].adjacent_param._reserved) {
            LOG(WTF, "adjacent_param._reserved != 0");
        }
        data += 3;
        nbits -= 3 * 8;
    }

    while (nbits > 7) {
        CHECK_LEN(nbits, 2 * 8, tsdu);
        const uint8_t iei                       = get_bits(8, data, 0);
        const uint8_t len                       = get_bits(8, data + 1, 0);
        data += 2;
        nbits -= 2 * 8;
        CHECK_LEN(nbits, len * 8, tsdu);
        if (iei == IEI_CELL_ID_LIST && len) {
            cell_id_list_t *p = iei_cell_id_list_decode(
                        tsdu->cell_ids, data, len);
            if (!p) {
                break;
            }
            tsdu->cell_ids = p;
        } else if (iei == IEI_ADJACENT_BN_LIST && len) {
            addr_list_t *p = iei_adjacent_bn_list_decode(
                        tsdu->cell_bns, data, len);
            if (!p) {
                break;
            }
            tsdu->cell_bns = p;
        } else {
            if (len) {
                LOG(WTF, "d_neighbouring_cell unknown iei (0x%x)", iei);
            }
        }
        data += len;
        nbits -= len * 8;
    }

    return tsdu;
}

static void d_neighbouring_cell_print(tsdu_d_neighbouring_cell_t *tsdu)
{
    printf("\tCODOP=0x%x (D_NEIGHBOURING_CELL)\n", D_NEIGHBOURING_CELL);
    printf("\t\tCCR_CONFIG=%d\n", tsdu->ccr_config.number);
    if (!tsdu->ccr_config.number) {
        return;
    }
    printf("\t\tCCR_PARAM=%d\n", tsdu->ccr_param);
    for (int i = 0; i < tsdu->ccr_config.number; ++i) {
        printf("\t\t\tBN_NB=%d CHANNEL_ID=%d ADJACENT_PARAM=%d BN=%d LOC=%d EXP=%d RXLEV_ACCESS=%d\n",
               tsdu->adj_cells[i].bn_nb,
               tsdu->adj_cells[i].channel_id,
               tsdu->adj_cells[i].adjacent_param._data,
               tsdu->adj_cells[i].adjacent_param.bn,
               tsdu->adj_cells[i].adjacent_param.loc,
               tsdu->adj_cells[i].adjacent_param.exp,
               tsdu->adj_cells[i].adjacent_param.rxlev_access);
    }
    if (tsdu->cell_ids) {
        printf("\t\tCELL_IDs\n");
        for (int i = 0; i < tsdu->cell_ids->len; ++i) {
            printf("\t\t\tCELL_ID BS_ID=%d RSW_ID=%d\n",
                   tsdu->cell_ids->cell_ids[i].bs_id,
                   tsdu->cell_ids->cell_ids[i].rws_id);
        }
    }
    if (tsdu->cell_bns) {
        printf("\t\tCELL_BNs\n");
        for (int i = 0; i < tsdu->cell_bns->len; ++i) {
            printf("\t\t\tCELL_BN=");
            addr_print(&tsdu->cell_bns->addrs[i]);
            printf("\n");
        }
    }
}

static tsdu_d_system_info_t *d_system_info_decode(const uint8_t *data, int nbits)
{
    tsdu_d_system_info_t *tsdu = malloc(sizeof(tsdu_d_system_info_t));
    if (!tsdu) {
        return NULL;
    }

    tsdu_base_set_nopts(&tsdu->base, 0);

    // minimal size of disconnected mode
    CHECK_LEN(nbits, 9*8, tsdu);

    tsdu->cell_state._data = get_bits(8, data + 1, 0);
    switch (tsdu->cell_state.mode) {
        case CELL_STATE_MODE_NORMAL:
            CHECK_LEN(nbits, (17 * 8), tsdu);
            tsdu->cell_config._data                     = get_bits( 8, data + 2, 0);
            tsdu->country_code                          = get_bits( 8, data + 3, 0);
            tsdu->system_id._data                       = get_bits( 8, data + 4, 0);
            tsdu->loc_area_id._data                     = get_bits( 8, data + 5, 0);
            tsdu->bn_id                                 = get_bits( 8, data + 6, 0);
            cell_id_decode1(&tsdu->cell_id, data + 7);
            tsdu->cell_bn                               = get_bits(12, data + 7, 12);
            tsdu->u_ch_scrambling                       = get_bits( 8, data + 10, 0);
            tsdu->cell_radio_param.tx_max               = get_bits( 3, data + 11, 0);
            tsdu->cell_radio_param.radio_link_timeout   = get_bits( 5, data + 11, 3);
            tsdu->cell_radio_param.pwr_tx_adjust        = get_bits( 4, data + 12, 0);
            tsdu->cell_radio_param.rx_lev_access        = get_bits( 4, data + 12, 4);
            tsdu->system_time                           = get_bits( 8, data + 13, 0);
            tsdu->cell_access._data                     = get_bits( 8, data + 14, 0);
            tsdu->_unused_1                             = get_bits( 4, data + 15, 0);
            tsdu->superframe_cpt                        = get_bits(12, data + 15, 4);
            break;

        default:
            LOG(WTF, "unknown cell_state.mode=%d", tsdu->cell_state.mode);

        case CELL_STATE_MODE_DISC_INTERN_BN:
        case CELL_STATE_MODE_DISC_MAIN_SWITCH:
        case CELL_STATE_MODE_DISC_RADIOSWITCH:
        case CELL_STATE_MODE_DISC_BSC:
            tsdu->cell_state._data &= 0xf0;
            cell_id_decode2(&tsdu->cell_id, data + 1);
            tsdu->bn_id                                 = get_bits( 8, data + 3, 0);
            tsdu->u_ch_scrambling                       = get_bits( 8, data + 4, 0);
            tsdu->cell_radio_param.tx_max               = get_bits( 3, data + 5, 0);
            tsdu->cell_radio_param.radio_link_timeout   = get_bits( 5, data + 5, 3);
            tsdu->cell_radio_param.pwr_tx_adjust        = get_bits( 4, data + 6, 0);
            tsdu->cell_radio_param.rx_lev_access        = get_bits( 4, data + 6, 4);
            tsdu->band                                  = get_bits( 4, data + 7, 0);
            tsdu->channel_id                            = get_bits(12, data + 7, 4);
            break;
    }

    return tsdu;
}

static void d_system_info_print(tsdu_d_system_info_t *tsdu)
{
    printf("\tCODOP=0x%0x (D_SYSTEM_INFO)\n", tsdu->base.codop);
    printf("\t\tCELL_STATE\n");
    printf("\t\t\tMODE=%03x\n", tsdu->cell_state.mode);
    if (tsdu->cell_state.mode == CELL_STATE_MODE_NORMAL) {
        printf("\t\t\tBCH=%d\n", tsdu->cell_state.bch);
        printf("\t\t\tROAM=%d\n", tsdu->cell_state.roam);
        printf("\t\t\tEXP=%d\n", tsdu->cell_state.exp);
        printf("\t\t\tRSERVED=%d\n", tsdu->cell_state._reserved_00);

        printf("\t\tCELL_CONFIG\n");
        printf("\t\t\tECCH=%d\n", tsdu->cell_config.eccch);
        printf("\t\t\tATTA=%d\n", tsdu->cell_config.atta);
        printf("\t\t\tRESERVED=%d\n", tsdu->cell_config._reserved_0);
        printf("\t\t\tMUX_TYPE=%d\n", tsdu->cell_config.mux_type);
        printf("\t\t\tSIM=%d\n", tsdu->cell_config.sim);
        printf("\t\t\tDC=%d\n", tsdu->cell_config.dc);
        printf("\t\tCOUNTRY_CODE=%d\n", tsdu->country_code);
        printf("\t\tSYSTEM_ID\n");
        printf("\t\t\tVERSION=%d\n", tsdu->system_id.version);
        printf("\t\t\tNETWORK=%d\n", tsdu->system_id.network);
        printf("\t\tLOC_AREA_ID\n");
        printf("\t\t\tLOC_ID=%d\n", tsdu->loc_area_id.loc_id);
        printf("\t\t\tMODE=%d\n", tsdu->loc_area_id.mode);
        printf("\t\tBN_ID=%d\n", tsdu->bn_id);
        printf("\t\tCELL_ID: BS_ID=%d RWS_ID=%d\n",
               tsdu->cell_id.bs_id, tsdu->cell_id.rws_id);
        printf("\t\tCELL_BN=%d\n", tsdu->cell_bn);
        printf("\t\tU_CH_SCRAMBLING=%d\n", tsdu->u_ch_scrambling);
        printf("\t\tCELL_RADIO_PARAM\n");
        printf("\t\t\tTX_MAX=%d\n", tsdu->cell_radio_param.tx_max);
        printf("\t\t\tRADIO_LINK_TIMEOUT=%d\n",
                tsdu->cell_radio_param.radio_link_timeout);
        printf("\t\t\tPWR_TX_ADJUST=%d dBm\n",
                CELL_RADIO_PARAM_PWR_TX_ADJUST_TO_DBM[
                    tsdu->cell_radio_param.pwr_tx_adjust]);
        printf("\t\t\tRX_LEV_ACCESS=%d dBm\n",
                CELL_RADIO_PARAM_RX_LEV_ACCESS_TO_DBM[
                    tsdu->cell_radio_param.rx_lev_access]);
        printf("\t\tSYSTEM_TIME=%d\n", tsdu->system_time);
        printf("\t\tCELL_ACCESS\n");
        printf("\t\t\tMIN_SERVICE_CLASS=%d\n",
                tsdu->cell_access.min_service_class);
        printf("\t\t\tMIN_REG_CLASS=%d\n",
                tsdu->cell_access.min_reg_class);
        printf("\t\tSUPERFRAME_CPT=%d\n", tsdu->superframe_cpt);
    } else {
        printf("\t\tCELL_ID BS_ID=%d RWS_ID=%d\n",
               tsdu->cell_id.bs_id, tsdu->cell_id.rws_id);
        printf("\t\tCELL_BN=%d\n", tsdu->cell_bn);
        printf("\t\tU_CH_SCRAMBLING=%d\n", tsdu->u_ch_scrambling);
        printf("\t\tCELL_RADIO_PARAM\n");
        printf("\t\t\tTX_MAX=%d\n", tsdu->cell_radio_param.tx_max);
        printf("\t\t\tRADIO_LINK_TIMEOUT=%d\n",
                tsdu->cell_radio_param.radio_link_timeout);
        printf("\t\t\tPWR_TX_ADJUST=%d dBm\n",
                CELL_RADIO_PARAM_PWR_TX_ADJUST_TO_DBM[
                    tsdu->cell_radio_param.pwr_tx_adjust]);
        printf("\t\t\tRX_LEV_ACCESS=%d dBm\n",
                CELL_RADIO_PARAM_RX_LEV_ACCESS_TO_DBM[
                    tsdu->cell_radio_param.rx_lev_access]);
        printf("\t\tBAND=%d\n", tsdu->band);
        printf("\t\tCHANNEL_ID=%d\n", tsdu->channel_id);
    }
}

static tsdu_d_ech_overload_id_t *d_ech_overload_id_decode(const uint8_t *data, int nbits)
{
    tsdu_d_ech_overload_id_t *tsdu = malloc(sizeof(tsdu_d_ech_overload_id_t));
    if (!tsdu) {
        return NULL;
    }
    tsdu_base_set_nopts(&tsdu->base, 0);

    CHECK_LEN(nbits, 6*8, tsdu);

    tsdu->activation_mode.hook = get_bits(2, data + 1, 0);
    tsdu->activation_mode.type = get_bits(2, data + 1, 2);
    tsdu->group_id = get_bits(12, data + 1, 4);
    cell_id_decode1(&tsdu->cell_id, data + 3);
    tsdu->organisation = get_bits(8, data + 5, 0);

    return tsdu;
}

static void d_ech_overload_id_print(const tsdu_d_ech_overload_id_t *tsdu)
{
    printf("\tCODOP=0x%0x (D_ECH_OVERLOAD_ID)\n", tsdu->base.codop);
    printf("\t\tACTIVATION_MODE: hook=%d type=%d\n",
           tsdu->activation_mode.hook, tsdu->activation_mode.type);
    printf("\t\tGROUP_ID=%d", tsdu->group_id);
    printf("\t\tCELL_ID: BS_ID=%d RWS_ID=%d\n",
           tsdu->cell_id.bs_id, tsdu->cell_id.rws_id);
    printf("\t\tORGANISATION=%d\n", tsdu->organisation);
}

static tsdu_seecret_codop_t *d_seecret_parse(const uint8_t *data, int nbits)
{
    tsdu_seecret_codop_t *tsdu = malloc(sizeof(tsdu_seecret_codop_t));
    if (!tsdu) {
        return NULL;
    }

    tsdu->nbits = nbits;
    if (!nbits) {
        tsdu_base_set_nopts(&tsdu->base, 0);
        return tsdu;
    }

    tsdu_base_set_nopts(&tsdu->base, 1);

    tsdu->data = malloc((nbits + 7) / 8);
    if (!tsdu->data) {
        tsdu_destroy(&tsdu->base);
        return NULL;
    }

    memcpy(tsdu->data, data, (nbits + 7) / 8);

    return tsdu;
}

static void d_seecret_print(const tsdu_seecret_codop_t *tsdu)
{
    printf("\tCODOP=0x%0x (seecret)\n", tsdu->base.codop);
    printf("\t\tnbits=%d data=", tsdu->nbits);
    print_hex(tsdu->data, (tsdu->nbits + 7) / 8);
}

static tsdu_d_data_end_t *d_data_end_decode(const uint8_t *data, int nbits)
{
    tsdu_d_data_end_t *tsdu = malloc(sizeof(tsdu_d_data_end_t));
    if (!tsdu) {
        return NULL;
    }

    tsdu_base_set_nopts(&tsdu->base, 0);
    CHECK_LEN(nbits, 2*8, tsdu);
    tsdu->cause = data[1];

    return tsdu;
}

static void d_data_end_print(const tsdu_d_data_end_t *tsdu)
{
    printf("\tCODOP=0x%0x (D_DATA_END)\n", tsdu->base.codop);
    printf("\t\tCAUSE=0x%02x\n", tsdu->cause);
}

static tsdu_d_datagram_notify_t *d_datagram_notify_decode(const uint8_t *data, int nbits)
{
    tsdu_d_datagram_notify_t *tsdu = malloc(sizeof(tsdu_d_datagram_notify_t));
    if (!tsdu) {
        return NULL;
    }

    tsdu_base_set_nopts(&tsdu->base, 0);
    CHECK_LEN(nbits, 5*8, tsdu);

    tsdu->call_priority         = get_bits(4, data + 1, 4);
    tsdu->message_reference     = data[2] | (data[3] << 8);
    tsdu->key_reference._data   = data[4];

    if (nbits >= 7 * 8) {
        tsdu->destination_port  = data[5] | (data[6] << 8);
    } else {
        tsdu->destination_port = -1;
    }

    return tsdu;
}

static void d_datagram_notify_print(const tsdu_d_datagram_notify_t *tsdu)
{
    printf("\tCODOP=0x%0x (D_DATAGRAM_NOTIFY)\n", tsdu->base.codop);
    printf("\t\tCALL_PRIORITY=%d\n", tsdu->call_priority);
    printf("\t\tMESSAGE_REFERENCE=%d\n", tsdu->message_reference);
    printf("\t\tKEY_REFERENCE: key_index=%d key_type=%d\n",
           tsdu->key_reference.key_index, tsdu->key_reference.key_type);
    if (tsdu->destination_port != -1) {
        printf("\t\tDESTINATION_PORT=%d\n", tsdu->destination_port);
    }
}

static tsdu_d_datagram_t *d_datagram_decode(const uint8_t *data, int nbits)
{
    const int len = nbits / 8 - 5;
    if (len < 0) {
        LOG(WTF, "too short");
        return NULL;
    }

    tsdu_d_datagram_t *tsdu = malloc(sizeof(tsdu_d_datagram_t) + len);
    if (!tsdu) {
        return NULL;
    }

    tsdu_base_set_nopts(&tsdu->base, 0);

    tsdu->call_priority = get_bits(4, data + 1, 4);
    tsdu->message_reference = data[2] | (data[3] << 8);
    tsdu->key_reference._data = data[4];
    tsdu->len = len;
    memcpy(tsdu->data, data + 5, len);

    return tsdu;
}

static void d_datagram_print(const tsdu_d_datagram_t *tsdu)
{
    printf("\tCODOP=0x%0x (D_DATAGRAM)\n", tsdu->base.codop);
    printf("\t\tCALL_PRIORITY=%d\n", tsdu->call_priority);
    printf("\t\tMESSAGE_REFERENCE=%d\n", tsdu->message_reference);
    printf("\t\tKEY_REFERENCE: key_type=%d key_index=%d\n",
           tsdu->key_reference.key_type, tsdu->key_reference.key_index);
    printf("\t\tDATA: len=%d data=", tsdu->len);
    print_hex(tsdu->data, tsdu->len);
}

static tsdu_d_explicit_short_data_t *d_explicit_short_data_decode(
        const uint8_t *data, int nbits)
{
    const int len = nbits / 8 - 1;
    if (len < 0) {
        LOG(WTF, "too short");
        return NULL;
    }

    tsdu_d_explicit_short_data_t *tsdu = malloc(
                sizeof(tsdu_d_explicit_short_data_t) + len);
    if (!tsdu) {
        LOG(ERR, "ERR OOM");
        return NULL;
    }
    tsdu_base_set_nopts(&tsdu->base, 0);

    tsdu->len = len;
    memcpy(tsdu->data, data + 1, len);

    return tsdu;
}

static void d_explicit_short_data_print(const tsdu_d_explicit_short_data_t *tsdu)
{
    printf("\tCODOP=0x%0x (D_EXPLICIT_SHORT_DATA)\n", tsdu->base.codop);
    printf("\t\tDATA: len=%d data=", tsdu->len);
    print_hex(tsdu->data, tsdu->len);
}

tsdu_t *tsdu_d_decode(const uint8_t *data, int nbits, int prio, int id_tsap)
{
    CHECK_LEN(nbits, 8, NULL);

    const codop_t codop = get_bits(8, data, 0);

    tsdu_t *tsdu = NULL;
    switch (codop) {
        case D_DATA_END:
            tsdu = (tsdu_t *)d_data_end_decode(data, nbits);
            break;

        case D_DATAGRAM:
            tsdu = (tsdu_t *)d_datagram_decode(data, nbits);
            break;

        case D_DATAGRAM_NOTIFY:
            tsdu = (tsdu_t *)d_datagram_notify_decode(data, nbits);
            break;

        case D_ECH_OVERLOAD_ID:
            tsdu = (tsdu_t *)d_ech_overload_id_decode(data, nbits);
            break;

        case D_EXPLICIT_SHORT_DATA:
            tsdu = (tsdu_t *)d_explicit_short_data_decode(data, nbits);
            break;

        case D_GROUP_ACTIVATION:
            tsdu = (tsdu_t *)d_group_activation_decode(data, nbits);
            break;

        case D_GROUP_COMPOSITION:
            tsdu = (tsdu_t *)d_group_composition_decode(data, nbits);
            break;

        case D_GROUP_LIST:
            tsdu = (tsdu_t *)d_group_list_decode(data, nbits);
            break;

        case D_NEIGHBOURING_CELL:
            tsdu = (tsdu_t *)d_neighbouring_cell_decode(data, nbits);
            break;

        case D_SYSTEM_INFO:
            tsdu = (tsdu_t *)d_system_info_decode(data, nbits);
            break;

        case D_SEECRET_0x47:
        case D_RESERVED_0x97:
            tsdu = (tsdu_t *)d_seecret_parse(data, nbits);
            break;

        default:
            LOG(WTF, "unsupported codop 0x%02x", codop);
    }

    if (tsdu) {
        tsdu->codop = codop;
        tsdu->downlink = true;
        tsdu->prio = prio;
        tsdu->id_tsap = id_tsap;
    }

    return tsdu;
}

static void tsdu_d_print(const tsdu_t *tsdu)
{
    switch (tsdu->codop) {
        case D_DATA_END:
            d_data_end_print((const tsdu_d_data_end_t *)tsdu);
            break;

        case D_DATAGRAM:
            d_datagram_print((const tsdu_d_datagram_t *)tsdu);
            break;

        case D_DATAGRAM_NOTIFY:
            d_datagram_notify_print((const tsdu_d_datagram_notify_t *)tsdu);
            break;

        case D_ECH_OVERLOAD_ID:
            d_ech_overload_id_print((const tsdu_d_ech_overload_id_t *)tsdu);
            break;

        case D_EXPLICIT_SHORT_DATA:
            d_explicit_short_data_print((const tsdu_d_explicit_short_data_t *)tsdu);
            break;

        case D_GROUP_ACTIVATION:
            d_group_activation_print((tsdu_d_group_activation_t *)tsdu);
            break;

        case D_GROUP_COMPOSITION:
            d_group_composition_print((tsdu_d_group_composition_t *)tsdu);
            break;

        case D_GROUP_LIST:
            d_group_list_print((tsdu_d_group_list_t *)tsdu);
            break;

        case D_NEIGHBOURING_CELL:
            d_neighbouring_cell_print((tsdu_d_neighbouring_cell_t *)tsdu);
            break;

        case D_SYSTEM_INFO:
            d_system_info_print((tsdu_d_system_info_t *)tsdu);
            break;

        case D_SEECRET_0x47:
        case D_RESERVED_0x97:
            d_seecret_print((tsdu_seecret_codop_t *)tsdu);
            break;

        default:
            LOG(WTF, "print not implemented: downlink codop=0x%02x",
                tsdu->codop);
    }
}

static void tsdu_u_print(const tsdu_t *tsdu)
{
    switch (tsdu->codop) {
        default:
            LOG(WTF, "print not implemented: uplink codop=0x%02x",
                tsdu->codop);
    }
}

void tsdu_print(tsdu_t *tsdu)
{
    if (tsdu->downlink) {
        tsdu_d_print(tsdu);
    } else {
        tsdu_u_print(tsdu);
    }
}
