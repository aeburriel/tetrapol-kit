#include <stdio.h>
#include "tsdu.h"


void d_system_info(char *t) {

	int mode, bch, roam, exp;
	int ecch, atta, mux_type, sim, dc;
	int country_code;
	int network, version;
	int loc_area_id_mode, loc_id;
	int bn_id;
	int cell_id_format, cell_id_bs_id, cell_id_rsw_id;
	int cell_bn_r1, cell_bn_r2, cell_bn_r3;
	int u_ch_scrambling;
	int system_time;
	int superframe_cpt;
	int tx_max, radio_link_timeout, pwr_tx_adjust, rxlev_access;
	int min_reg_class, min_service_class;

	mode=bits_to_int(t+8, 3);
	bch=t[11];
	roam=t[14];
	exp=t[15];

	ecch=t[16];
	atta=t[17];
	mux_type=bits_to_int(t+19, 3);
	sim=t[22];
	dc=t[23];

	country_code=bits_to_int(t+24, 8);

	network=bits_to_int(t+32, 4);
	version=bits_to_int(t+36, 4);

	loc_area_id_mode=bits_to_int(t+40, 2);
	loc_id=bits_to_int(t+42, 6);

	bn_id=bits_to_int(t+48, 8);

	cell_id_format=bits_to_int(t+56, 2);
	if (cell_id_format==0) {
		cell_id_bs_id=bn_id=bits_to_int(t+58, 6);
		cell_id_rsw_id=bn_id=bits_to_int(t+64, 4);
	} else if (cell_id_format==1) {
		cell_id_rsw_id=bn_id=bits_to_int(t+58, 6);
		cell_id_bs_id=bn_id=bits_to_int(t+64, 4);
	} else
		printf("Bad CELL_ID\n");

	cell_bn_r1=bits_to_int(t+68, 4);
	cell_bn_r2=bits_to_int(t+72, 4);
	cell_bn_r3=bits_to_int(t+76, 4);

	u_ch_scrambling=bits_to_int(t+81, 7);

	// radio param
	tx_max=bits_to_int(t+88, 3);
	radio_link_timeout=bits_to_int(t+91, 5);
	pwr_tx_adjust=radio_link_timeout = -76+4*bits_to_int(t+96, 4);
	rxlev_access = -92 + 4*bits_to_int(t+100, 4);

	system_time=bits_to_int(t+104, 8);

	// cell access
	min_reg_class=bits_to_int(t+112, 4);
	min_service_class=bits_to_int(t+116, 4);

	superframe_cpt=bits_to_int(t+124, 12);

	if (bch==0)
		mod_set(3);
	

	printf("\t\tCELL_STATE\n");
	printf("\t\t\tMODE=%i%i%i ", t[48], t[49], t[50]);
	switch (mode) {
		case 0:
			printf("normal\n");
			break;
		case 1:
			printf("inter BN disconnected mode\n");
			break;
		case 2:
			printf("main switch disconnected mode\n");
			break;
		case 3:
			printf("radioswitch disconnected mode\n");
			break;
		case 4:
			printf("BSC-disconnected mode\n");
			break;
		default:
			printf("reserved\n");
			break;
	}

	printf("\t\t\tBCH=%i ", bch);
	if (bch==0)
		printf("first BCH block superframe\n");
	else	
		printf("other block\n");

	printf("\t\t\tROAM=%i ", roam);
	if (roam==0)
		printf("ll RT accepted\n");
	else	
		printf("Home RT only\n");

	printf("\t\t\tEXP=%i ", exp);
	if (exp==0)
		printf("normal cell\n");
	else	
		printf("experimental cell\n");
	
	printf("\t\tCELL_CONFIG\n");
	printf("\t\t\tECCH=%i ", ecch);
	if (ecch==0)
		printf("No ECCH in service\n");
	else	
		printf("At least one ECCH in service\n");
	
	printf("\t\t\tATTA=%i ", atta);
	if (atta==0)
		printf("Attach/detach function not supported\n");
	else	
		printf("Attach/detach function supported\n");
	
	printf("\t\t\tMUX_TYPE=%i ", mux_type);
	if (mux_type==0)
		printf("TETRAPOL default type\n");
	else if (mux_type==1)	
		printf("type 2\n");
	else
		printf("reserved\n");
	
	printf("\t\t\tSIM=%i ", sim);
	if (sim==0)
		printf("Single base station cell\n");
	else	
		printf("Simulcast cell\n");
	
	printf("\t\t\tDC=%i ", dc);
	if (dc==0)
		printf("Single cell\n");
	else	
		printf("Double coverage cell\n");
	
	printf("\t\tCOUNTRY_CODE=%i\n", country_code);

	printf("\t\tSYSTEM_ID\n");
	printf("\t\t\tNETWORK=%i\n", network);
	printf("\t\t\tVERSION=%i\n", version);

	printf("\t\tLOC_AREA_ID\n");
	printf("\t\t\tMODE=%i ", loc_area_id_mode);
	switch (loc_area_id_mode) {
		case 0:
			printf("LAI = RSW_ID + BS_ID\n");
			break;
		case 1:
			printf("LAI = BS_ID\n");
			break;
		case 2:
			printf("LAI = LOC_ID\n");
			break;
		case 3:
			printf("reserved\n");
			break;
	}
	printf("\t\t\tLOC_ID=%i\n", loc_id);

	printf("\t\tBN_ID=%i\n", bn_id);

	printf("\t\tCELL_ID\n");
	printf("\t\t\tBS_ID=%i\n", cell_id_bs_id);
	printf("\t\t\tRSW_ID=%i\n", cell_id_rsw_id);

	printf("\t\tCELL_BN\n");
	printf("\t\t\tR1=%i\n", cell_bn_r1);
	printf("\t\t\tR2=%i\n", cell_bn_r2);
	printf("\t\t\tR3=%i\n", cell_bn_r3);

	printf("\t\tU_CH_SCRAMBLING=%i\n", u_ch_scrambling);

	printf("\t\tCELL_RADIO_PARAM\n");
	printf("\t\t\tTX_MAX=%i\n", tx_max);
	printf("\t\t\tRADIO_LINK_TIMEOUT=%i\n", radio_link_timeout);
	printf("\t\t\tPWR_TX_ADJUST= %i dBm\n", pwr_tx_adjust);
	printf("\t\t\tRXLEV_ACCESS= %i dBm\n", rxlev_access);

	printf("\t\tSYSTEM_TIME=%i\n", system_time);

	printf("\t\tCELL_ACCESS\n");
	printf("\t\t\tMIN_REG_CLASS=%i\n", min_reg_class);
	printf("\t\t\tMIN_SERVICE_CLASS=%i\n", min_service_class);

	printf("\t\tSUPERFRAME_CPT=%i\n", superframe_cpt);


}
	
void d_group_composition(char *t) {

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

void d_group_activation(char *t) {

	int activation_mode, group_id, coverage_id, channel_id, u_ch_scrambling, d_ch_scrambling, key_reference, tti;

	activation_mode=bits_to_int(t+8,4);
	group_id=bits_to_int(t+12, 12);
	coverage_id=bits_to_int(t+24, 8);
	channel_id=bits_to_int(t+36,12);
	u_ch_scrambling=bits_to_int(t+48,8);
	u_ch_scrambling=bits_to_int(t+56,8);
	key_reference=bits_to_int(t+64,8);

	// TODO TTI

	printf("\tCODOP=0x55 (D_GROUP_ACTIVATION)\n");
	printf("\t\tACTIVATION_MODE=%i\n", activation_mode);
	printf("\t\tGROUP_ID=%i\n", group_id);
	printf("\t\tCOVERAGE_ID=%i\n", coverage_id);
	printf("\t\tCHANNEL_ID=%i\n", channel_id);
	printf("\t\tU_CH_SCRAMBLING=%i\n", u_ch_scrambling);
	printf("\t\tD_CH_SCRAMBLING=%i\n", d_ch_scrambling);
	printf("\t\tKEY_REFERENCE=%i\n", key_reference);
}

void d_group_list(char *t) {

	printf("\tCODOP=0x92 (D_GROUP_ACTIVATION)\n");
}

void d_neighbouring_cell(char *t) {

	int ccr_config, ccr_param;
	int bn_nb, channel_id, adjacent_param;

	ccr_config=bits_to_int(t+12,4);
	ccr_param=bits_to_int(t+16,8);

	bn_nb=bits_to_int(t+24,4);
	channel_id=bits_to_int(t+28,12);
	adjacent_param=bits_to_int(t+40,8);

	printf("\tCODOP=0x93 (D_NEIGHBOURING_CELL)\n");
	printf("\t\tCCR_CONFIG=%i\n", ccr_config);
	printf("\t\tCCR_PARAM=%i\n", ccr_param);

	printf("\t\tBN_NB=%i\n", bn_nb);
	printf("\t\tCHANNEL_ID=%i\n", channel_id);
	printf("\t\tADJACENT_PARAM=%i\n", adjacent_param);
	
}

void d_tti_assignment(char *t) {
	printf("\tCODOP=0x?? (D_TTI_ASSIGNMENT)\n");
}


void decode_bch(char *t) {

	int codop;

	printf("\tBCH\n");

	printf("\tRT_REF=");
	print_buf(t+24, 4);
	printf("\tBS_REF=");
	print_buf(t+28, 4);
	printf("\tCALL_PRIO=");
	print_buf(t+36, 4);

	codop=bits_to_int(t+40, 8);
	switch (codop) {
		case D_SYSTEM_INFO:
			printf("\tCODOP=0x%02x (D_SYSTEM_INFO)\n", codop);
			d_system_info(t+40);
			break;
		default:
			printf("\tCODOP=0x%02x (Unknown) ", codop);
			print_buf(t+40, 8);
			break;
	}

}

void decode_rch_address(char *t) {

	int a, y, x;

	a = t[0];
	y=bits_to_int(t+1, 3);
	x=bits_to_int(t+4, 12);

	if (a==0) {
		printf("ACK ");
		decode_addr(t);
	} else {
		printf("NACK ");
		if (y==4)
			printf("Noise\n");
		if (y==5)
			printf("Collision\n");
	}
}

void decode_pch(char *t) {

	printf("\tPCH\n");

	// TODO: activation bitmap

	printf("\t\t");
	decode_addr(t+64);
	printf("\t\t");
	decode_addr(t+80);
	printf("\t\t");
	decode_addr(t+96);
	printf("\t\t");
	decode_addr(t+112);
}

void decode_rch(char *t) {

	printf("\tRCH\n");
	printf("\t\tTERMINAL ADDRES 1: ");
	decode_rch_address(t);
	printf("\t\tTERMINAL ADDRES 2: ");
	decode_rch_address(t+16);
	printf("\t\tTERMINAL ADDRES 3: ");
	decode_rch_address(t+32);
}

void tsdu_process(char *t, int data_length, int mod) {

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
		case 99999:
			d_tti_assignment(t);
			break;
		default:
			printf("\tCODOP=0x%02x (Unknown) ", codop);
			print_buf(t+40, 8);
			break;
	}

}
