#pragma once

#include <stdint.h>

// PAS 0001-3-2 4.4
enum {
    D_REJECT                    = 0x08,
    D_REFUSAL                   = 0x09,
    U_END                       = 0x0a,
    D_BACK_CCH                  = 0x0b,
    D_RELEASE                   = 0x0c,
    U_ABORT                     = 0x0d,
    U_TERMINATE                 = 0x0e,
    D_HOOK_ON_INVITATION        = 0x0f,
    D_RETURN                    = 0x10,
    U_EVENT_REPORT              = 0x11,
    D_CALL_WAITING              = 0x12,
    D_AUTHENTICATION            = 0x13,
    U_AUTHENTICATION            = 0x14,
    D_AUTHORISATION             = 0x16,
    U_ERROR_REPORT              = 0x17,
    D_CHANNEL_INIT              = 0x18,
    U_REGISTRATION_REQ          = 0x20,
    D_REGISTRATION_NAK          = 0x21,
    D_REGISTRATION_ACK          = 0x22,
    D_FORCED_REGISTRATION       = 0x23,
    U_LOCATION_ACTIVITY         = 0x24,
    D_LOCATION_ACTIVITY_ACK     = 0x25,
    U_CALL_SETUP                = 0x30,
    D_CALL_ALERT                = 0x31,
    D_CALL_SETUP                = 0x32,
    U_CALL_ANSWER               = 0x33,
    D_CALL_CONNECT              = 0x34,
    D_CALL_SWITCH               = 0x35,
    U_CALL_INTRUSION_PC         = 0x36,
    U_CALL_INTRUSION_OCH        = 0x37,
    D_TRANSFER_NAK              = 0x39,
    U_TRANSFER_REQ              = 0x3a,
    U_CALL_INTRUSION_ECH        = 0x3b,
    U_CALL_RELEASE              = 0x3c,
    U_CALL_CONNECT              = 0x3d,
    U_CALL_SWITCH               = 0x3d,
    D_CALL_START                = 0x3e,
    D_CALL_ACTIVATION           = 0xe0,
    D_CALL_COMPOSITION          = 0xe1,
    D_CALL_END                  = 0xe2,
    D_CALL_OVERLOAD_ID          = 0xe3,
    D_FUNCTIONAL_SHORT_DATA     = 0x42,
    U_DATA_DOWN_ACCEPT          = 0x43,
    U_DATA_MSG_UP               = 0x44,
    D_DATA_MSG_DOWN             = 0x45,
    D_EXPLICIT_SHORT_DATA       = 0x46,
    D_DATA_END                  = 0x48,
    D_DATAGRAM_NOTIFY           = 0x49,
    D_DATAGRAM                  = 0x4a,
    D_BROADCAST                 = 0x4b,
    D_DATA_SERV                 = 0x4c,
    D_DATA_DOWN_STATUS          = 0x4e,
    D_CONNECT_DCH               = 0x60,
    D_CONNECT_CCH               = 0x62,
    D_DATA_AUTHENTICATION       = 0x63,
    D_DATA_REQUEST              = 0x64,
    D_DCH_OPEN                  = 0x65,
    U_DATA_REQUEST              = 0x66,
    D_EXTENDED_STATUS           = 0x67,
    D_CCH_OPEN                  = 0x68,
    D_BROADCAST_WAITING         = 0x69,
    U_OCH_RELEASE               = 0x50,
    U_OCH_SETUP                 = 0x51,
    U_ECH_CLOSE                 = 0x52,
    D_EMERGENCY_NOTIFICATION    = 0x53,
    U_ECH_SETUP                 = 0x54,
    D_GROUP_ACTIVATION          = 0x55,
    D_ECH_ACTIVATION            = 0x56,
    D_GROUP_END                 = 0x57,
    D_GROUP_IDLE                = 0x58,
    D_GROUP_REJECT              = 0x59,
    D_ECH_REJECT                = 0x5a,
    D_GROUP_PAGING              = 0x5b,
    D_BROADCAST_NOTIFICATION    = 0x5c,
    D_CRISIS_NOTIFICATION       = 0x5d,
    D_EMERGENCY_ACK             = 0x5f,
    D_EMERGENCY_NAK             = 0x80,
    U_EMERGENCY_REQ             = 0x81,
    D_GROUP_OVERLOAD_ID         = 0x82,
    D_ECH_OVERLOAD_ID           = 0x83,
    D_PRIORITY_GRP_WAITING      = 0x84,
    D_PRIORITY_GRP_ACTIVATION   = 0x85,
    D_OC_ACTIVATION             = 0x86,
    D_OC_REJECT                 = 0x87,
    D_OC_PAGING                 = 0x88,
    D_ACCESS_DISABLED           = 0x70,
    D_TRAFFIC_ENABLED           = 0x71,
    D_TRAFFIC_DISABLED          = 0x72,
    U_DEVIATION_CLEAR           = 0x73,
    U_DEVIATION_SET             = 0x74,
    D_DEVIATION_ON              = 0x76,
    D_SERVICE_DISABLED          = 0x78,
    D_ABILITY_MNGT              = 0x77,
    D_SYSTEM_INFO               = 0x90,
    D_GROUP_LIST                = 0x92,
    D_GROUP_COMPOSITION         = 0x93,
    D_NEIGHBOURING_CELL         = 0x94,
    D_ECCH_DESCRIPTION          = 0x95,
    D_ADDITIONAL_PARTICIPANTS   = 0x96,
    D_INFORMATION_DELIVERY      = 0xc5,
};
typedef uint8_t codop_t;

enum {
    IEI_GROUP_ID            = 0x01,
    IEI_CELL_ID_LIST        = 0x02,
    IEI_KEY_OF_CALL         = 0x03,
    IEI_ADJACENT_BN_LIST    = 0x04,
    IEI_TTI                 = 0x05,
    IEI_ADR                 = 0x06,
    IEI_USER_PRIORITY       = 0x81,
    IEI_COVERAGE_ID         = 0x82,
    IEI_KEY_REFERENCE       = 0x83,
    IEI_ADD_SETUP_PARAM     = 0x84,
    IEI_PROFILE_ID          = 0x85,
};
typedef uint8_t iei_t;

// do not use directly, this struct must be first member of each TSDU structure
typedef struct {
    codop_t codop;
    int noptionals;     ///< number of optionals
    /**
      In subclassed TSDU structure, noptionals pointers should be present.
      Those are initialized to NULL by tsdu_base_init and freed by tsdu_destroy.
      */
    void *optionals[];
} tsdu_base_t;

/// PAS 0001-3-2 5.3.23
enum {
    CELL_STATE_EXP_NORMAL = 0,
    CELL_STATE_EXP_EXPERIMENTAL = 1,
};

enum {
    CELL_STATE_ROAM_ACCEPTED    = 0,
    CELL_STATE_ROAM_HOME_ONLY   = 1,
};

enum {
    CELL_STATE_MODE_NORMAL              = 0,
    CELL_STATE_MODE_DISC_INTERN_BN      = 1,
    CELL_STATE_MODE_DISC_MAIN_SWITCH    = 2,
    CELL_STATE_MODE_DISC_RADIOSWITCH    = 3,
    CELL_STATE_MODE_DISC_BSC            = 4,
    // 3 values reserved
};

typedef union {
    uint8_t _data;
    struct {
        unsigned int exp : 1;
        unsigned int roam : 1;
        unsigned int _reserved_00 : 2;
        unsigned int bch : 1;
        unsigned int mode : 3;
    };
} cell_state_t;

/// PAS 0001-3-2 5.3.20
enum {
    CELL_CONFIG_DC_SINGLE           = 0,
    CELL_CONFIG_DC_DOUBLE_COVERAGE  = 1,
};

enum {
    CELL_CONFIG_SIM_SINGLE      = 0,
    CELL_CONFIG_SIM_SIMULCAST   = 1,
};

enum {
    CELL_CONFIG_MUX_TYPE_DEFAULT    = 0,
    CELL_CONFIG_MUX_TYPE_TYPE_2     = 1,
    // 6 values reserved
};

enum {
    CELL_CONFIG_ATTA_NOT_SUPPORTED  = 0,
    CELL_CONFIG_ATTA_SUPPORTED      = 1,
};

enum {
    CELL_CONFIG_ECCH_NOT_PRESENT    = 0,
    CELL_CONFIG_ECCH_PRESENT        = 1,
};

typedef union {
    uint8_t _data;
    struct {
        unsigned int dc : 1;
        unsigned int sim : 1;
        unsigned int mux_type : 3;
        unsigned int _reserved_0 : 1;
        unsigned int atta : 1;
        unsigned int eccch : 1;
    };
} cell_config_t;

/// PAS 0001-3-2 5.3.65
typedef union {
    uint8_t _data;
    struct {
        unsigned int version : 4;
        unsigned int network : 4;
    };
} system_id_t;

/// PAS 0001-3-2 5.3.40
enum {
    LOCAL_AREA_ID_MODE_RSW_BS   = 0,
    LOCAL_AREA_ID_MODE_BS       = 1,
    LOCAL_AREA_ID_MODE_LOC      = 2,
    // one value reserved
};

typedef union {
    uint8_t _data;
    struct {
        unsigned int loc_id : 6;
        unsigned int mode : 2;
    };
} loc_area_id_t;

/// PAS 0001-3-2 5.3.9
enum {
    BN_ID_UNDEFINED = 0,
};

/// PAS 0001-3-2 5.3.21
enum {
    CELL_ID_FORMAT_0    = 0,
    CELL_ID_FORMAT_1    = 1,
    // 2 values reserved
};

typedef struct {
    uint8_t bs_id;
    uint8_t rws_id;
} cell_id_t;

/// PAS 0001-3-2 5.3.22
enum {
    CELL_RADIO_PARAM_TX_MAX_UNLIMITED   = 0,
    // 7 values reserved
};

enum {
    CELL_RADIO_PARAM_RADIO_LINK_TIMEOUT_INTERNAL,
    // 31 values reserved
};

/// convert pwr_tx_adjust to TX power adjustment in dBm
extern const int CELL_RADIO_PARAM_PWR_TX_ADJUST_TO_DBM[16];

/// shall by used by RT to estimate radio conditions for one cell
extern const int CELL_RADIO_PARAM_RX_LEV_ACCESS_TO_DBM[16];

typedef struct {
    uint8_t tx_max;
    uint8_t radio_link_timeout;
    uint8_t pwr_tx_adjust;
    uint8_t rx_lev_access;
} cell_radio_param_t;

/// PAS 0001-3-2 5.3.18
typedef union {
    uint8_t _data;
    struct {
        unsigned int min_service_class : 4;
        unsigned int min_reg_class : 4;
    };
} cell_access_t;

/// PAS 0001-3-2 4.4.71
typedef struct {
    tsdu_base_t base;
    cell_state_t cell_state;
    cell_config_t cell_config;
    uint8_t country_code;
    system_id_t system_id;
    loc_area_id_t loc_area_id;
    uint8_t bn_id;
    uint16_t cell_id;
    uint16_t cell_bn;
    uint8_t u_ch_scrambling;
    cell_radio_param_t cell_radio_param;
    uint8_t system_time;
    cell_access_t cell_access;
    uint8_t _unused_1;  // 4b are marked as unused
    uint16_t superframe_cpt;
// used only when cell_state == disconnected
    uint8_t band;
    uint16_t channel_id;
} tsdu_system_info_t;

// this might change in future
typedef tsdu_base_t tsdu_t;

void tsdu_destroy(tsdu_base_t *tsdu);
tsdu_t *tsdu_decode(const uint8_t *data, int nbits);
void tsdu_print(tsdu_t *tsdu);

void tsdu_process(const uint8_t* t, int data_length, int mod);

void decode_pch(const uint8_t *t);
void decode_rch(const uint8_t *t);
// TODO
void decode_rch_addressconst (uint8_t *t);

