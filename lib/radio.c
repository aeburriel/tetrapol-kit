#include "tetrapol.h"
#include "frame.h"
#include "multiblock.h"
#include "tpdu.h"
#include "radio.h"
#include "misc.h"

#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#define DEBUG

// max error rate for 2 frame synchronization sequences
#define MAX_FRAME_SYNC_ERR 1

struct _tetrapol_phys_ch_t {
    int fd;
    int last_sync_err;  ///< errors in last frame synchronization sequence
    int total_sync_err; ///< cumulative error in framing
    int data_len;
    uint8_t data[10*FRAME_LEN];
};

int mod = -1;
static uint8_t scramb_table[127];

static int process_frame(frame_t *frame);

void mod_set(int m) {
    mod=m;
}

// set on SIGINT
volatile static int do_exit = 0;

static void sigint_handler(int sig)
{
    do_exit = 1;
}

tetrapol_phys_ch_t *tetrapol_create(int fd)
{
    tetrapol_phys_ch_t *t = malloc(sizeof(tetrapol_phys_ch_t));
    if (t == NULL) {
        return NULL;
    }
    memset(t, 0, sizeof(tetrapol_phys_ch_t));

    if (fcntl(fd, F_SETFL, O_NONBLOCK | fcntl(t->fd, F_GETFL))) {
        goto err_fd;
    }
    t->fd = fd;

    radio_init();

    return t;

//err_data:
//    free(t->data);
err_fd:
    free(t);

    return NULL;
}

void tetrapol_destroy(tetrapol_phys_ch_t *t)
{
    free(t);
}

static uint8_t differential_dec(uint8_t *data, int size, uint8_t last_bit)
{
    while (size--) {
        last_bit = *data = *data ^ last_bit;
        ++data;
    }
    return last_bit;
}

static int tetrapol_recv(tetrapol_phys_ch_t *t)
{
    struct pollfd fds;
    fds.fd = t->fd;
    fds.events = POLLIN;
    fds.revents = 0;

    // hack, buffer is full, but return 0 means EOF
    if ((sizeof(t->data) - t->data_len) == 0) {
        return 1;
    }

    if (poll(&fds, 1, -1) > 0 && !do_exit) {
        if (! (fds.revents & POLLIN)) {
            return -1;
        }
        int rsize = read(t->fd, t->data + t->data_len, sizeof(t->data) - t->data_len);
        if (rsize == -1) {
            return -1;
        }
        t->data_len += rsize;

        return rsize;
    }

    return do_exit ? 0 : -1;
}

// compare bite stream to differentialy encoded synchronization sequence
static int cmp_frame_sync(const uint8_t *data)
{
    const uint8_t frame_dsync[] = { 1, 0, 1, 0, 0, 1, 1, };
    int sync_err = 0;
    for(int i = 0; i < sizeof(frame_dsync); ++i) {
        if (frame_dsync[i] != data[i + 1]) {
            ++sync_err;
        }
    }
    return sync_err;
}

/**
  Find 2 consecutive frame synchronization sequences.

  Using raw stream (before differential decoding) simplyfies search
  because only signal polarity must be considered,
  there is lot of troubles with error handlig after differential decoding.
  */
static int find_frame_sync(tetrapol_phys_ch_t *t)
{
    int offs = 0;
    int sync_err = MAX_FRAME_SYNC_ERR + 1;
    while (offs + FRAME_LEN + FRAME_HDR_LEN < t->data_len) {
        const uint8_t *data = t->data + offs;
        sync_err = cmp_frame_sync(data) +
            cmp_frame_sync(data + FRAME_LEN);
        if (sync_err <= MAX_FRAME_SYNC_ERR) {
            break;
        }

        ++offs;
    }

    t->data_len -= offs;
    memmove(t->data, t->data + offs, t->data_len);

    if (sync_err <= MAX_FRAME_SYNC_ERR) {
        t->last_sync_err = 0;
        t->total_sync_err = 0;
        return 1;
    }

    return 0;
}

/// return number of acquired frames (0 or 1) or -1 on error
static int get_frame(tetrapol_phys_ch_t *t, frame_t *frame)
{
    if (t->data_len < FRAME_LEN) {
        return 0;
    }
    const int sync_err = cmp_frame_sync(t->data);
    if (sync_err + t->last_sync_err > MAX_FRAME_SYNC_ERR) {
        t->total_sync_err = 1 + 2 * t->total_sync_err;
        if (t->total_sync_err >= FRAME_LEN) {
            return -1;
        }
    } else {
        t->total_sync_err = 0;
    }

    t->last_sync_err = sync_err;
    memcpy(frame->data, t->data + FRAME_HDR_LEN, FRAME_DATA_LEN);
    differential_dec(frame->data, FRAME_DATA_LEN, 0);
    t->data_len -= FRAME_LEN;
    memmove(t->data, t->data + FRAME_LEN, t->data_len);

    return 1;
}

int tetrapol_main(tetrapol_phys_ch_t *t)
{
    signal(SIGINT, sigint_handler);

    while (!do_exit) {
        int recv = tetrapol_recv(t);
        if (recv <= 0) {
            return recv;
        }

        if (find_frame_sync(t)) {
            int r = 1;

            fprintf(stderr, "Frame sync found\n");
            multiblock_reset();
            segmentation_reset();

            while (r >= 0 && !do_exit) {
                frame_t frame;

                while ((r = get_frame(t, &frame)) > 0) {
                    process_frame(&frame);
                }

                recv = tetrapol_recv(t);
                if (recv <= 0) {
                    return recv;
                }
            }
            fprintf(stderr, "Frame sync lost\n");
        }
        mod = -1;
    }

    return 0;
}

// http://ghsi.de/CRC/index.php?Polynom=10010
static void mk_crc5(uint8_t *res, const uint8_t *input, int input_len)
{
    uint8_t do_invert;
    memset(res, 0, 5);

    for (int i=0; i<input_len; ++i)
    {
        do_invert = input[i] ^ res[0];         // XOR required?

        res[0] = res[1];
        res[1] = res[2];
        res[2] = res[3] ^ do_invert;
        res[3] = res[4];
        res[4] = do_invert;
    }
}

static int check_data_crc(const uint8_t *d)
{
    uint8_t crc[5];
    int res;

    mk_crc5(crc, d, 69);
    res = memcmp(d+69, crc, 5);
    //	printf("crc=");
    //	print_buf(d+69,5);
    //	printf("crcc=");
    //	print_buf(crc,5);
    return res ? 0 : 1;
}

static void decode_data_frame(const uint8_t *c, uint8_t *d)
{
    uint8_t b1[26];
    uint8_t b2[50];

    int j, check=1;

    for (j=0; j<=25; j++) b1[j] = 2;
    for (j=0; j<=49; j++) b2[j] = 2;
    for (j=0; j<=73; j++) d[j] = 2;

    // b'(25)=b'(-1)
    // b'(j-1)=C(2j)-C(2j+1)

    // j=0 
    b1[25]=c[0] ^ c[1];
    for(j=1; j<=25; j++) {
        b1[j-1]=c[2*j] ^ c[2*j+1];
    }

    //	printf("b1=");
    //	print_buf(b1,26);

    b2[0] = c[53];
    for(j=2; j<=48; j++) {
        b2[j-1]=c[2*j+52] ^ c[2*j+53];
    }

    //	printf("b2=");
    //	print_buf(b2,48);

    for(j=0; j<=25; j++)
        d[j]=b1[j];
    for(j=0; j<=47; j++)
        d[j+26]=b2[j];

    if ((c[150] != c[151]) || ((c[148] ^ c[149]) != c[150]) || (c[52] != c[53]))
        check=0;

    for (j=3; j < 23; j++) {
        if (c[2*j] != (b1[j]^b1[j-1]^b1[j-2]))
            check=0;
    }
    for (j=3; j < 45; j++) {
        if (c[2*j+52] != (b2[j]^b2[j-1]^b2[j-2]))
            check=0;
    }
    if (!check)
        d[0]=2;
}

// PAS 0001-2 6.1.4.1
static const int interleave_voice_UHF[] = {
    1, 77, 38, 114, 20, 96, 59, 135,
    3, 79, 41, 117, 23, 99, 62, 138,
    5, 81, 44, 120, 26, 102, 65, 141,
    8, 84, 47, 123, 29, 105, 68, 144,
    11, 87, 50, 126, 32, 108, 71, 147,
    14, 90, 53, 129, 35, 111, 74, 150,
    17, 93, 56, 132, 37, 113, 73, 4,
    0, 76, 40, 119, 19, 95, 58, 137,
    151, 80, 42, 115, 24, 100, 60, 133,
    12, 88, 48, 121, 30, 106, 66, 139,
    18, 91, 51, 124, 28, 104, 67, 146,
    10, 89, 52, 131, 34, 110, 70, 149,
    13, 97, 57, 130, 36, 112, 75, 148,
    6, 82, 39, 116, 16, 92, 55, 134,
    2, 78, 43, 122, 22, 98, 61, 140,
    9, 85, 45, 118, 27, 103, 63, 136,
    15, 83, 46, 125, 25, 101, 64, 143,
    7, 86, 49, 128, 31, 107, 69, 142,
    21, 94, 54, 127, 33, 109, 72, 145,
};

// PAS 0001-2 6.2.4.1
static const int interleave_data_UHF[] = {
    1, 77, 38, 114, 20, 96, 59, 135,
    3, 79, 41, 117, 23, 99, 62, 138,
    5, 81, 44, 120, 26, 102, 65, 141,
    8, 84, 47, 123, 29, 105, 68, 144,
    11, 87, 50, 126, 32, 108, 71, 147,
    14, 90, 53, 129, 35, 111, 74, 150,
    17, 93, 56, 132, 37, 112, 76, 148,
    2, 88, 40, 115, 19, 97, 58, 133,
    4, 75, 43, 118, 22, 100, 61, 136,
    7, 85, 46, 121, 25, 103, 64, 139,
    10, 82, 49, 124, 28, 106, 67, 142,
    13, 91, 52, 127, 31, 109, 73, 145,
    16, 94, 55, 130, 34, 113, 70, 151,
    0, 80, 39, 116, 21, 95, 57, 134,
    6, 78, 42, 119, 24, 98, 60, 137,
    9, 83, 45, 122, 27, 101, 63, 140,
    12, 86, 48, 125, 30, 104, 66, 143,
    15, 89, 51, 128, 33, 107, 69, 146,
    18, 92, 54, 131, 36, 110, 72, 149,
};

static void frame_deinterleave(frame_t *f)
{
    uint8_t tmp[FRAME_DATA_LEN];
    memcpy(tmp, f->data, FRAME_DATA_LEN);

    for (int j = 0; j < FRAME_DATA_LEN; ++j) {
        f->data[j] = tmp[interleave_data_UHF[j]];
    }
}

// inline int is_in_code(uint8_t j) {
//	uint8_t pre_cod[] = {7, 10, 13, 16, 19, 22, 25, 28, 31, 34, 37, 40, 43, 46, 49, 52, 55, 58, 61, 64, 67, 70, 73, 76, 83, 86, 89, 92, 95, 98, 101, 104, 107, 110, 113, 116, 119, 122, 125, 128, 131, 134, 137, 140, 143, 146, 149};
//	int i;
//	for(i=0; i<sizeof(pre_cod); i++) {
//		if(pre_cod[i]==j)
//			return 1;
//	}
//	return 0;
//}

static const int pre_cod[] = {
    1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

static void frame_diff_dec(frame_t *f)
{
    for (int j = FRAME_DATA_LEN - 1; j > 0; --j) {
        f->data[j] ^= f->data[j - pre_cod[j]];
    }
}

static void frame_descramble(frame_t *f, int scr)
{
    if (scr == 0) {
        return;
    }

    for(int k = 0 ; k < FRAME_DATA_LEN; k++) {
        f->data[k] ^= scramb_table[(k + scr) % 127];
    }
}

#define FRAME_BITORDER_LEN 64

static void bitorder_frame(const uint8_t *d, uint8_t *out)
{
    for (int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            out[8*i + j] =  d[i*8 + 7-j];
        }
    }
}

void radio_init(void)
{
    for(int i = 0; i < 127; i++) {
        scramb_table[i] = (i < 7) ? 1 : (scramb_table[i-1] ^ scramb_table[i-7]);
    }
}

static int process_frame(frame_t *f)
{
    int scr, scr2, i, j;
    uint8_t asbx, asby, fn0, fn1;
    uint8_t frame_bord[FRAME_BITORDER_LEN];

    if (mod != -1)
        mod++;
    if (mod==200)
        mod=0;

    //	printf("s=");
    //	print_buf(scramb_table,127);
    //	printf("f=");
    //	print_buf(f,160);

    //	printf("Attempting descramble\n");
    int scr_ok=0;
    for(scr=0; scr<=127; scr++) {
        //		printf("trying scrambling %i\n", scr);

        frame_t f_;
        memcpy(&f_, f, sizeof(f_));

        frame_descramble(&f_, scr);
        frame_diff_dec(&f_);
        frame_deinterleave(&f_);

        uint8_t d[FRAME_DATA_LEN];
        decode_data_frame(f_.data, d);
        //		printf("d=");
        //		print_buf(d,74);

        if(d[0]!=1) {
            //			printf("not data frame!\n");
            continue;
        }

        if(!check_data_crc(d)) {
            //			printf("crc mismatch!\n");
            continue;
        }
        //		printf("b=");
        //		print_buf(d+1, 68);

        scr2=scr;
        asbx=d[67];			// maybe x=68, y=67
        asby=d[68];
        fn0=d[2];
        fn1=d[1];
        bitorder_frame(d+3, frame_bord);

        scr_ok++;
    }
    if(scr_ok==1) {
        printf("OK mod=%03i fn=%i%i asb=%i%i scr=%03i ", mod, fn0, fn1, asbx, asby, scr2);
        for (i=0; i<8; i++) {
            for(j=0; j<8; j++)
                printf("%i", frame_bord[i*8+j]);
            printf(" ");
        }
        print_buf(frame_bord, 64);
        multiblock_process(frame_bord, 2*fn0 + fn1, mod);
    } else {
        printf("ERR2 mod=%03i\n", mod);
        multiblock_reset();
        segmentation_reset();
    }

    return 0;
}
