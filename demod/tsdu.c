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

	system_time=bits_to_int(t+104, 8);

	// cell access

	superframe_cpt=bits_to_int(t+124, 12);
	
	printf("\tCODOP=0x90 (SYSTEM_INFO)\n");

	

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
	
	printf("\t\tCELL_CONFIG=\n");
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

	// radio params

	printf("\t\tSYSTEM_TIME=%i\n", system_time);

	// cell access
	printf("\t\tSUPERFRAME_CPT=%i\n", superframe_cpt);


//	printf("\t\t\t=%i ", );
//	if (==0)
//		printf("\n");
//	else	
//		printf("\n");

	
}
	
void d_group_composition(char *t) {
	printf("\tCODOP=0x93 (D_GROUP_COMPOSITION)\n");
}

void tsdu_process(char* t, int num) {

	int codop;

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
		case D_SYSTEM_INFO:
			d_system_info(t+40);
			break;
		case D_GROUP_COMPOSITION:
			d_group_composition(t+40);
			break;
		default:
			printf("\tCODOP=0x%02x (Unknown) ", codop);
			print_buf(t+40, 8);
			break;
	}

}
