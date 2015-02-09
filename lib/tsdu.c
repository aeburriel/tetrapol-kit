#include "tsdu.h"
#include "misc.h"
#include "bit_utils.h"
#include "phys_ch.h"
#include "misc.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define CHECK_LEN(len, min_exp_len, tsdu) \
    if ((len) < (min_exp_len)) { \
        printf("%s:%d TSDU data too short %d < %d\n", \
                __func__, __LINE__, (len), (min_exp_len)); \
        tsdu_destroy((tsdu_base_t *)tsdu); \
        return NULL; \
    }

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

static void tsdu_base_init(tsdu_base_t *tsdu, codop_t codop, int noptionals)
{
    tsdu->noptionals = noptionals;
    tsdu->codop = codop;
    memset(tsdu->optionals, 0, noptionals * sizeof(void *));
}

static tsdu_system_info_t *decode_system_info(const uint8_t *data, int nbits)
{
    tsdu_system_info_t *tsdu = malloc(sizeof(tsdu_system_info_t));
    if (!tsdu) {
        return NULL;
    }

    tsdu_base_init(&tsdu->base, D_SYSTEM_INFO, 0);

    // minimal size of disconnected mode
    CHECK_LEN(nbits, 9*8, tsdu);

    GET_BITS(8, 1*8, data, tsdu->cell_state._data);
    switch (tsdu->cell_state.mode) {
        case CELL_STATE_MODE_NORMAL:
            CHECK_LEN(nbits, (17 * 8), tsdu);
            GET_BITS( 8,  2*8,    data, tsdu->cell_config._data);
            GET_BITS( 8,  3*8,    data, tsdu->country_code);
            GET_BITS( 8,  4*8,    data, tsdu->system_id._data);
            GET_BITS( 8,  5*8,    data, tsdu->loc_area_id._data);
            GET_BITS( 8,  6*8,    data, tsdu->bn_id);
            GET_BITS(12,  7*8,    data, tsdu->cell_id);
            GET_BITS(12,  7*8+12, data, tsdu->cell_bn);
            GET_BITS( 8, 10*8,    data, tsdu->u_ch_scrambling);
            GET_BITS( 3, 11*8,    data, tsdu->cell_radio_param.tx_max);
            GET_BITS( 5, 11*8+3,  data, tsdu->cell_radio_param.radio_link_timeout);
            GET_BITS( 4, 12*8,    data, tsdu->cell_radio_param.pwr_tx_adjust);
            GET_BITS( 4, 12*8+4,  data, tsdu->cell_radio_param.rx_lev_access);
            GET_BITS( 8, 13*8,    data, tsdu->system_time);
            GET_BITS( 8, 14*8,    data, tsdu->cell_access._data);
            GET_BITS( 4, 15*8,    data, tsdu->_unused_1);
            GET_BITS(12, 15*8+4,  data, tsdu->superframe_cpt);
            break;

        default:
            printf("unknown cell_state.mode value %d", tsdu->cell_state.mode);

        case CELL_STATE_MODE_DISC_INTERN_BN:
        case CELL_STATE_MODE_DISC_MAIN_SWITCH:
        case CELL_STATE_MODE_DISC_RADIOSWITCH:
        case CELL_STATE_MODE_DISC_BSC:
            tsdu->cell_state._data &= 0xf0;
            GET_BITS(12, 1*8+4, data, tsdu->cell_id);
            GET_BITS( 8, 3*8,   data, tsdu->bn_id);
            GET_BITS( 8, 4*8,   data, tsdu->u_ch_scrambling);
            GET_BITS( 3, 5*8,   data, tsdu->cell_radio_param.tx_max);
            GET_BITS( 5, 5*8+3, data, tsdu->cell_radio_param.radio_link_timeout);
            GET_BITS( 4, 6*8,   data, tsdu->cell_radio_param.pwr_tx_adjust);
            GET_BITS( 4, 6*8+4, data, tsdu->cell_radio_param.rx_lev_access);
            GET_BITS( 4, 7*8,   data, tsdu->band);
            GET_BITS(12, 7*8+4, data, tsdu->channel_id);
            break;
    }

    return tsdu;
}

tsdu_t *tsdu_decode(const uint8_t *data, int nbits)
{
    CHECK_LEN(nbits, 8, NULL);

    codop_t codop;
    GET_BITS(8, 0, data, codop);

    switch (codop) {
        case D_SYSTEM_INFO:
            return (tsdu_t *)decode_system_info(data, nbits);

        default:
            printf("unsupported TSDU codop %d\n", codop);
    }

    return NULL;
}

static void tsdu_system_info_print(tsdu_system_info_t *tsdu)
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
        printf("\t\tCELL_ID=%d\n", tsdu->cell_id);
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
        printf("\t\tCELL_ID=%d\n", tsdu->cell_id);
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

void tsdu_print(tsdu_t *tsdu)
{
    switch (tsdu->codop) {
        case D_SYSTEM_INFO:
            tsdu_system_info_print((tsdu_system_info_t *)tsdu);
            break;
        default:
            printf("    CODOP=%0x\n", tsdu->codop);
            printf("print not implemented for this TSDU\n");
    }
}

static void decode_cell_id(int cell_id) {

    int cell_id_format, cell_id_bs_id, cell_id_rsw_id;

    cell_id_format=(cell_id & 0xc00) >> 10;
    if (cell_id_format==0) {
        cell_id_bs_id=(cell_id & 0x3f0) >> 4;
        cell_id_rsw_id=(cell_id & 0x0f);
    } else if (cell_id_format==1) {
        cell_id_rsw_id=(cell_id & 0x3f0) >> 4;
        cell_id_bs_id=(cell_id & 0x0f);
    } else
        printf("Bad CELL_ID\n");

    printf("\t\tCELL_ID FORMAT=%i BS_ID=%i RSW_ID=%i\n", cell_id_format, cell_id_bs_id, cell_id_rsw_id);

}

static void decode_key_reference(int key_reference) {
    int key_type, key_index;

    key_type=(key_reference & 0xf0) >>4;
    key_index=(key_reference & 0x0f);

    printf("KEY_REFERENCE=%i KEY_TYPE=%i KEY_INDEX=%i", key_reference, key_type, key_index);
    if (key_index==0)
        printf("CLEAR CALL!!! ");
    // TODO: Decode key_type
}

static void d_group_composition(const uint8_t *t) {

    int og_nb;
    int i;
    int group_id[10];

    og_nb=bits_to_int(t+20, 4);
    group_id[0]=bits_to_int(t+8, 12);
    for (i=1; i<=og_nb; i++)
        group_id[i]=bits_to_int(t+i*12+12, 12);


    printf("\tCODOP=0x93 (D_GROUP_COMPOSITION)\n");
    printf("\t\tOG_NB=%i\n", og_nb);
    for (i=0; i<=og_nb; i++) {
        printf("\t\tGROUP_ID=%04i ", group_id[i]);
        if (group_id[i] < 3500)
            printf("simple OG");
        if ((group_id[i] >= 3500) && (group_id[i] <=3755))
            printf("multi OG");
        if (group_id[i] == 4095)
            printf("all OG");
        printf("\n");
    }
}

static void d_group_activation(const uint8_t *t) {

    int activation_mode, group_id, coverage_id, channel_id, u_ch_scrambling;
    int d_ch_scrambling, key_reference;

    activation_mode=bits_to_int(t+8,4);
    group_id=bits_to_int(t+12, 12);
    coverage_id=bits_to_int(t+24, 8);
    channel_id=bits_to_int(t+36,12);
    u_ch_scrambling=bits_to_int(t+48,8);
    u_ch_scrambling=bits_to_int(t+56,8);
    key_reference=bits_to_int(t+64,8);
    // TODO
    d_ch_scrambling = 0;

    // TODO TTI

    printf("\tCODOP=0x55 (D_GROUP_ACTIVATION)\n");
    printf("\t\tACTIVATION_MODE=%i\n", activation_mode);
    printf("\t\tGROUP_ID=%i\n", group_id);
    printf("\t\tCOVERAGE_ID=%i\n", coverage_id);
    printf("\t\tCHANNEL_ID=%i\n", channel_id);
    printf("\t\tU_CH_SCRAMBLING=%i\n", u_ch_scrambling);
    printf("\t\tD_CH_SCRAMBLING=%i\n", d_ch_scrambling);
    printf("\t\t");
    decode_key_reference(key_reference);
    printf("\n");
}

static void d_group_list(const uint8_t *t) {

    int i;
    int reference_list, revision, csg, cso, dc;
    int index_list, index_list_mode, index_list_index;
    int type_nb, type_nb_type, type_nb_number;
    const uint8_t *type_nb_start;
    int type_nb2, type_nb_type2, type_nb_number2;
    const uint8_t *type_nb_start2;
    int och_coverage_id[64], och_call_priority[64], och_group_id[64], och_och_parameters[64], och_neghbouring_cell[64];

    reference_list=bits_to_int(t+8,8);
    revision=bits_to_int(t+8,3);
    csg=bits_to_int(t+13,1);
    cso=bits_to_int(t+14,1);
    dc=bits_to_int(t+15,1);

    index_list=bits_to_int(t+16,8);
    index_list_mode=bits_to_int(t+16,2);
    index_list_index=bits_to_int(t+18,6);

    type_nb_start=t+24;
    type_nb=bits_to_int(type_nb_start,8);
    type_nb_type=bits_to_int(type_nb_start,2);
    type_nb_number=bits_to_int(type_nb_start+2,6);
    for (i=0; i<type_nb_number; i++) {
        och_coverage_id[i]=bits_to_int(type_nb_start+8+i*40,8);
        och_call_priority[i]=bits_to_int(type_nb_start+16+i*40,4);
        och_group_id[i]=bits_to_int(type_nb_start+20+i*40,12);
        och_och_parameters[i]=bits_to_int(type_nb_start+32+i*40,4);
        och_neghbouring_cell[i]=bits_to_int(type_nb_start+36+i*40,12);
    }
    type_nb_start2=type_nb_start+8+type_nb_number*40;
    type_nb2=bits_to_int(type_nb_start2,8);
    type_nb_type2=bits_to_int(type_nb_start2,2);
    type_nb_number2=bits_to_int(type_nb_start2+2,6);

    //TODO: TKG, EOG

    printf("\tCODOP=0x92 (D_GROUP_LIST)\n");
    printf("\t\tREFERENCE_LIST=%i REVISION=%i CSG=%i CSO=%i DC=%i\n", reference_list, revision, csg, cso, dc);
    printf("\t\tINDEX_LIST=%i MODE=%i INDEX=%i\n", index_list, index_list_mode, index_list_index);
    printf("\t\tTYPE_NB=%i TYPE=%i NUMBER=%i\n", type_nb, type_nb_type, type_nb_number);
    printf("\t\tOCH\n");
    for (i=0; i<type_nb_number; i++) {
        printf("\t\t\tCOVERAGE_ID=%i CALL_PRIORITY=%i GROUP_ID=%i OCH_PARAMETERS=%i NEIGBOURING_CELL=%i\n", och_coverage_id[i], och_call_priority[i], och_group_id[i], och_och_parameters[i], och_neghbouring_cell[i]);
    }
    printf("\t\tTYPE_NB=%i TYPE=%i NUMBER=%i\n", type_nb2, type_nb_type2, type_nb_number2);

}

static void d_neighbouring_cell(const uint8_t *t) {

    int i;
    int ccr_config, ccr_param;
    int bn_nb[16], channel_id[16], adjacent_param[16];
    int bn[16], loc[16], exp[16], rxlev_access[16];
    const uint8_t *cell_id_list_start;
    int cell_id_list, cell_id_list_length;
    int cell_id[256];
    const uint8_t *adjacent_bn_list_start;
    int adjacent_bn_list, adjacent_bn_list_length;
    int adjacent_bn[256];

    ccr_config=bits_to_int(t+12,4);
    ccr_param=bits_to_int(t+16,8);

    for (i=0; i<ccr_config; i++) {
        bn_nb[i]=bits_to_int(t+24+24*i,4);
        channel_id[i]=bits_to_int(t+28+24*i,12);
        adjacent_param[i]=bits_to_int(t+40+24*i,8);
        bn[i]=bits_to_int(t+40+24*i,1);
        loc[i]=bits_to_int(t+41+24*i,1);
        exp[i]=bits_to_int(t+43+24*i,1);
        rxlev_access[i]=bits_to_int(t+44+24*i,4);
    }

    cell_id_list=bits_to_int(t+24+24*ccr_config, 8);
    cell_id_list_length=bits_to_int(t+32+24*ccr_config, 8) / 2;	// 16-bit entries
    cell_id_list_start=t+40+24*ccr_config;

    cell_id_list_length = cell_id_list_length + 1; 			// FIXME: Why +1 ???

    for (i=0; i<cell_id_list_length; i++) {
        cell_id[i]=bits_to_int(cell_id_list_start+16*i,12);
    }

    adjacent_bn_list=bits_to_int(cell_id_list_start+16*cell_id_list_length, 8);
    adjacent_bn_list_length=bits_to_int(cell_id_list_start+8+16*cell_id_list_length, 8);
    adjacent_bn_list_start=cell_id_list_start+16+16*cell_id_list_length;

    adjacent_bn_list_length=adjacent_bn_list_length*8/12;		//12-bit entries

    for (i=0; i<adjacent_bn_list_length; i++) {
        adjacent_bn[i] = bits_to_int(adjacent_bn_list_start+12*i,12);
    }


    printf("\tCODOP=0x94 (D_NEIGHBOURING_CELL)\n");
    printf("\t\tCCR_CONFIG=%i\n", ccr_config);
    printf("\t\tCCR_PARAM=%i\n", ccr_param);

    for (i=0; i<ccr_config; i++) {
        printf("\t\t\tBN_NB=%i ", bn_nb[i]);
        printf("CHANNEL_ID=%i ", channel_id[i]);
        printf("ADJACENT_PARAM=%i BN=%i LOC=%i EXP=%i RXLEV_ACCESS=%i\n", adjacent_param[i], bn[i], loc[i], exp[i], rxlev_access[i]);
    }
    printf("\t\tCELL_ID_LIST=%i CELL_ID_LIST_LENGTH=%i\n", cell_id_list, cell_id_list_length);
    for (i=0; i<cell_id_list_length; i++) {
        printf("\t");
        decode_cell_id(cell_id[i]);
    }
    printf("\t\tADJACENT_BN_ID_LIST=%i ADJACENT_BN_ID_LIST_LENGTH=%i\n", adjacent_bn_list, adjacent_bn_list_length);
    for (i=0; i<adjacent_bn_list_length; i++) {
        printf("\t\t\tADJACENT_BN_ID=%x\n", adjacent_bn[i]);
    }


}

static void d_tti_assignment(const uint8_t *t) {
    printf("\tCODOP=0x?? (D_TTI_ASSIGNMENT)\n");
}


static void d_call_waiting(const uint8_t *t) {

    int appli_sap_id, call_priority, call_id;

    appli_sap_id=bits_to_int(t+8,4);
    call_priority=bits_to_int(t+12,4);
    call_id=bits_to_int(t+16,8);

    // TODO: CALLING_ADDR

    printf("\tCODOP=0x12 (D_CALL_WAITING)\n");
    printf("\t\tAPPLI_SAP_ID=%i ",appli_sap_id);
    switch(appli_sap_id) {

        case 0:
            printf("(BROADCAST)\n");
            break;
        case 1:
            printf("(TRANSPORT PROTOCOL MANAGEMENT)\n");
            break;
        case 2:
            printf("(REGISTRATION)\n");
            break;
        case 3:
            printf("(RESERVED)\n");
            break;
        case 4:
            printf("(PRIVATE CALL)\n");
            break;
        case 5:
            printf("(GROUP COMMUNICATIONS)\n");
            break;
        case 6:
            printf("(EMERGENCY OPEN CHANNEL)\n");
            break;
        case 7:
            printf("(DATA MESSAGE)\n");
            break;
        case 8:
            printf("(RT MANAGEMENT)\n");
            break;
        case 9:
            printf("(RESERVED)\n");
            break;
        case 10:
            printf("(KEY DELIVERY)\n");
            break;
        case 11:
            printf("(DATA FLOW)\n");
            break;
        case 12:
            printf("(INFO_DELIVERY)\n");
            break;
        default:
            printf("(reserved)\n");
    }

    printf("\t\tCALL_PRIORITY=%i\n", call_priority);
    printf("\t\tCALL_ID=%i\n", call_id);
}

static void d_call_alert(const uint8_t *t) {
    printf("\tCODOP=0x31 (D_CALL_ALERT)\n");
}

static void d_connect_cch(const uint8_t *t) {
    printf("\tCODOP=0x62 (D_CONNECT_CCH)\n");
}

static void d_data_end(const uint8_t *t) {
    printf("\tCODOP=0x48 (D_DATA_END)\n");
}

static void d_data_msg_down(const uint8_t *t) {
    printf("\tCODOP=0x45 (D_DATA_MSG_DOWN)\n");
}

static void d_registration_ack(const uint8_t *t) {

    int complete_reg;
    int rt_min_activity;
    int rt_status, fix, pro, chg, ren, tra;
    int li, cna;
    int r1, r2, r3, f, s1, s2, i1, i2, i3;
    int rt_min_registration;
    int tlr_value;
    int rt_data_info;
    int group_id;
    int iei;
    int coverage_id;

    complete_reg=bits_to_int(t+8,8);
    rt_min_activity=bits_to_int(t+16,8);
    rt_status=bits_to_int(t+24,8);
    fix=bits_to_int(t+26,1);
    pro=bits_to_int(t+28,1);
    chg=bits_to_int(t+29,1);
    ren=bits_to_int(t+30,1);
    tra=bits_to_int(t+31,1);

    li=bits_to_int(t+32,1);
    cna=bits_to_int(t+33,3);
    if (cna==1) {
        r1=bits_to_int(t+36,4);
        r2=bits_to_int(t+40,4);
        r3=bits_to_int(t+44,4);
        f=bits_to_int(t+48,4);
        s1=bits_to_int(t+52,4);
        s2=bits_to_int(t+56,4);
        i1=bits_to_int(t+60,4);
        i2=bits_to_int(t+64,4);
        i3=bits_to_int(t+68,4);
    }

    rt_min_registration=bits_to_int(t+72,8);
    tlr_value=bits_to_int(t+80,8);
    rt_data_info=bits_to_int(t+88,8);
    group_id=bits_to_int(t+96,12);
    iei=bits_to_int(t+112,8);
    if (iei == IEI_COVERAGE_ID)
        coverage_id=bits_to_int(t+120,8);

    printf("\tCODOP=0x22 (D_REGISTRATION_ACK)\n");
    printf("\t\tCOMPLETE_REG=%i\n", complete_reg);
    printf("\t\tRT_MIN_ACTIVITY=%i\n", rt_min_activity);
    printf("\t\tRT_STATUS=%i FIX=%i PRO=%i CHG=%i REN=%i TRA=%i\n", rt_status, fix, pro, chg, ren, tra);
    printf("\t\tHOST_ADR LI=%i CNA=%i ", li, cna);
    if (cna==1) {
        printf("RFSI=%x%x%x-%x-%x%x-%x%x%x\n", r1, r2, r3, f, s1, s2, i1, i2, i3);
    } else
        print_buf(t+32,40);
    printf("\t\tRT_MIN_REGISTRATION=%i\n", rt_min_registration);
    printf("\t\tTLR_VALUE=%i\n", tlr_value);
    printf("\t\tRT_DATA_INFO=%i\n", rt_data_info);
    printf("\t\tGROUP_ID=%i\n", group_id);
    if (iei == IEI_COVERAGE_ID)
        printf("\t\tCOVERAGE_ID=%i\n", coverage_id);

}

static void d_call_connect(const uint8_t *t) {
    int call_type;
    int channel_id;
    int u_ch_scrambling;
    int d_ch_scrambling;
    int key_reference;

    call_type=bits_to_int(t+8,8);
    channel_id=bits_to_int(t+20,12);
    u_ch_scrambling=bits_to_int(t+32,8);
    d_ch_scrambling=bits_to_int(t+40,8);
    key_reference=bits_to_int(t+48,8);

    printf("\tCODOP=0x34 (D_CALL_CONNECT)\n");
    printf("\t\tCALL_TYPE=%i\n", call_type);
    printf("\t\tCHANNEL_ID=%i\n", channel_id);
    printf("\t\tU_CH_SCRAMBLING=%i\n", u_ch_scrambling);
    printf("\t\tD_CH_SCRAMBLING=%i\n", d_ch_scrambling);
    printf("\t\t");
    decode_key_reference(key_reference);
    printf("\n");
    printf("\t\tVALID_RT=\n");
    print_buf(t+56,64);
    // TODO: KEY_OF_CALL
}

static void d_functional_short_data(const uint8_t *t) {
    printf("\tCODOP=0x42 (D_FUNCTIONAL_SHORT_DATA)\n");
}

static void d_call_start(const uint8_t *r) {
    printf("\tCODOP=0x3e (D_CALL_START)\n");
}

static void d_registration_nak(const uint8_t *t) {
    printf("\tCODOP=0x21 (D_REGISTRATION_NAK)\n");
}

static void d_call_setup(const uint8_t *t) {
    printf("\tCODOP=0x32 (D_CALL_SETUP)\n");
}

static void d_reject(const uint8_t *t) {
    printf("\tCODOP=0x08 (D_REJECT)\n");
}

static void d_return(const uint8_t *t) {
    printf("\tCODOP=0x10 (D_RETURN)\n");
}

static void d_authentication(const uint8_t *t) {
    printf("\tCODOP=0x13 (D_AUTHENTICATION)\n");
}

static void d_authorisation(const uint8_t *t) {
    printf("\tCODOP=0x16 (D_AUTHORISATION)\n");
}

static void d_channel_init(const uint8_t *t) {
    printf("\tCODOP=0x18 (D_CHANNEL_INIT)\n");
}

static void d_forced_registration(const uint8_t *t) {
    printf("\tCODOP=0x23 (D_FORCED_REGISTRATION)\n");
}

static void d_location_activity_ack(const uint8_t *t) {
    printf("\tCODOP=0x25 (D_LOCATION_ACTIVITY_ACK)\n");
}

static void d_call_switch(const uint8_t *t) {
    printf("\tCODOP=0x35 (D_CALL_SWITCH)\n");
}

static void d_call_end(const uint8_t *t) {
    printf("\tCODOP=0xe2 (D_CALL_END)\n");
}

static void d_explicit_short_data(const uint8_t *t) {
    printf("\tCODOP=0x46 (D_EXPLICIT_SHORT_DATA)\n");
}

static void d_connect_dch(const uint8_t *t) {

    int dch_low_layer;
    int channel_id;
    int u_ch_scrambling;
    int d_ch_scrambling;

    dch_low_layer=bits_to_int(t+8, 8);
    channel_id=bits_to_int(t+16, 12);
    u_ch_scrambling=bits_to_int(t+32, 8);
    u_ch_scrambling=bits_to_int(t+40, 8);
    // TODO
    d_ch_scrambling = 0;

    printf("\tCODOP=0x60 (D_CONNECT_DCH)\n");
    printf("\t\tDCH_LOW_LAYER=%i\n", dch_low_layer);
    printf("\t\tCHANNEL_ID=%i\n", channel_id);
    printf("\t\tU_CH_SCRAMBLING=%i\n", u_ch_scrambling);
    printf("\t\tD_CH_SCRAMBLING=%i\n", d_ch_scrambling);
}

static void d_data_authentication(const uint8_t *t) {
    printf("\tCODOP=0x63 (D_DATA_AUTHENTICATION)\n");
}

void tsdu_process(const uint8_t *t, int data_length, int mod) {

    int codop;

    printf("\tSDCH\n");

    codop=bits_to_int(t, 8);
    switch (codop) {
        case D_GROUP_COMPOSITION:
            d_group_composition(t);
            break;
        case D_GROUP_ACTIVATION:
            d_group_activation(t);
            break;
        case D_GROUP_LIST:
            d_group_list(t);
            break;
        case D_NEIGHBOURING_CELL:
            d_neighbouring_cell(t);
            break;
        case D_CALL_WAITING:
            d_call_waiting(t);
            break;
        case D_CALL_ALERT:
            d_call_alert(t);
            break;
        case D_CONNECT_CCH:
            d_connect_cch(t);
            break;
        case D_DATA_END:
            d_data_end(t);
            break;
        case D_DATA_MSG_DOWN:
            d_data_msg_down(t);
            break;
        case D_REGISTRATION_ACK:
            d_registration_ack(t);
            break;
        case D_CALL_CONNECT:
            d_call_connect(t);
            break;
        case D_FUNCTIONAL_SHORT_DATA:
            d_functional_short_data(t);
            break;
        case D_CALL_START:
            d_call_start(t);
            break;
        case D_REGISTRATION_NAK:
            d_registration_nak(t);
            break;
        case D_CALL_SETUP:
            d_call_setup(t);
            break;
        case D_DATA_AUTHENTICATION:
            d_data_authentication(t);
            break;
        case D_CONNECT_DCH:
            d_connect_dch(t);
            break;
        case D_REJECT:
            d_reject(t);
            break;
        case D_RETURN:
            d_return(t);
            break;
        case D_AUTHENTICATION:
            d_authentication(t);
            break;
        case D_AUTHORISATION:
            d_authorisation(t);
            break;
        case D_CHANNEL_INIT:
            d_channel_init(t);
            break;
        case D_FORCED_REGISTRATION:
            d_forced_registration(t);
            break;
        case D_LOCATION_ACTIVITY_ACK:
            d_location_activity_ack(t);
            break;
        case D_CALL_SWITCH:
            d_call_switch(t);
            break;
        case D_CALL_END:
            d_call_end(t);
            break;
        case D_EXPLICIT_SHORT_DATA:
            d_explicit_short_data(t);
            break;
        case 99999:
            d_tti_assignment(t);
            break;
        default:
            printf("\tCODOP=0x%02x (Unknown) ", codop);
            print_buf(t, 8);
            break;
    }

}
