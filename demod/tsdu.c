#include <stdio.h>
#include "tsdu.h"


int bits_to_int(char *bits, int num) {

	int i, ret=0;

	for (i=0; i<num; i++)
		ret = ret + (bits[i] << (num - i - 1));

	return ret;
}

char stuff[] = {1, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1};

int detect_stuff(char *bits) {
	int i;

	for (i=0; i<40; i++)
		if (memcmp(bits, stuff+i, 40) == 0)
			return 1;
	return 0;

}

void decode_addr(char *t) {
	int x,y,z;

	z=bits_to_int(t, 1);
	y=bits_to_int(t+1, 3);
	x=bits_to_int(t+4, 12);

	if ((z==1) && (y==0))
		printf("RTI:%03x\n", x);
	if ((z==0) && (y==0))
		printf("CGI:%03x\n", x);
	if ((z==0) && (y!=0) && (y!=1)) {
		printf("TTI:%1x%03x",y, x);
		if ((y==7) && (x==0))
			printf(" no ST");
		if ((y==7) && (x==4095))
			printf(" all STs");
		printf("\n");
	}
	if (y==1)
		printf("COI:%03x\n", x);
}

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
	printf("\tCODOP=0x93 (D_GROUP_COMPOSITION)\n");
}

void d_group_activation(char *t) {
	printf("\tCODOP=0x55 (D_GROUP_ACTIVATION)\n");
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
}

void decode_sdch(char *t) {

	int codop;

	printf("\tSDCH\n");
	printf("\tADDR=");
	decode_addr(t);

	if (detect_stuff(t+24)) {
		printf("\tSTUFFED\n");
		return;
	}

	printf("\tRT_REF=");
	print_buf(t+24, 4);
	printf("\tBS_REF=");
	print_buf(t+28, 4);
	printf("\tCALL_PRIO=");
	print_buf(t+36, 4);

	codop=bits_to_int(t+40, 8);
	switch (codop) {
		case D_GROUP_COMPOSITION:
			d_group_composition(t+40);
			break;
		case D_GROUP_ACTIVATION:
			d_group_activation(t+40);
			break;
		case 99999:
			d_tti_assignment(t+40);
			break;
		default:
			printf("\tCODOP=0x%02x (Unknown) ", codop);
			print_buf(t+40, 8);
			break;
	}

}

void tsdu_process(char* t, int num, int mod) {

	int codop;

	if (((mod==-1) && (num==3)) || (mod==0) || (mod==100)) {
		decode_bch(t);
		return;
	}

	if ((mod==98) ||(mod==198)) {
		decode_pch(t);
		return;
	}

	if (mod%25 == 14) {
		decode_rch(t);
		return;
	}

	decode_sdch(t);

}
