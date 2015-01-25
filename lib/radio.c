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

#define BUF_LEN ((10*FRAME_LEN))
// max error rate for 2 frame synchronization sequences
#define MAX_FRAME_SYNC_ERR 1

struct _tetrapol_t {
    uint8_t *buf;
    int data_len;
    int fd;
    int last_sync_err;  ///< errors in last frame synchronization sequence
    int total_sync_err; ///< cumulative error in framing
    int invert;         ///< polarity of differentialy encoded bits stream
};

int mod = -1;
static uint8_t scramb_table[127];

static void radio_process_frame(const uint8_t *f, int framelen, int modulo);

void mod_set(int m) {
    mod=m;
}

// set on SIGINT
volatile static int do_exit = 0;

static void sigint_handler(int sig)
{
    do_exit = 1;
}

static const uint8_t frame_sync[] = { 0, 1, 1, 0, 0, 0, 1, 0 };
#define FRAME_SYNC_LEN ((sizeof(frame_sync)))

tetrapol_t *tetrapol_create(int fd)
{
    tetrapol_t *t = malloc(sizeof(tetrapol_t));
    if (t == NULL) {
        return NULL;
    }
    memset(t, 0, sizeof(tetrapol_t));

    if (fcntl(fd, F_SETFL, O_NONBLOCK | fcntl(t->fd, F_GETFL))) {
        goto err_fd;
    }
    t->fd = fd;

    t->buf = malloc(BUF_LEN);
    if (t->buf == NULL) {
        goto err_fd;
    }

    radio_init();

    return t;

//err_buf:
//    free(t->buf);
err_fd:
    free(t);

    return NULL;
}

void tetrapol_destroy(tetrapol_t *t)
{
    free(t->buf);
    free(t);
}

static uint8_t differential_dec(uint8_t *buf, int size, uint8_t last_bit)
{
    while (size--) {
        last_bit = *buf = *buf ^ last_bit;
        ++buf;
    }
    return last_bit;
}

static int tetrapol_recv(tetrapol_t *t)
{
    struct pollfd fds;
    fds.fd = t->fd;
    fds.events = POLLIN;
    fds.revents = 0;

    if (poll(&fds, 1, -1) > 0 && !do_exit) {
        if (! (fds.revents & POLLIN)) {
            return -1;
        }
        int rsize = read(t->fd, t->buf + t->data_len, BUF_LEN - t->data_len);
        if (rsize == -1) {
            return -1;
        }
        t->data_len += rsize;

        return rsize;
    }

    return do_exit ? 0 : -1;
}

static int process_frame(frame_t *frame)
{
    if (mod != -1)
        mod++;
    if (mod==200)
        mod=0;
    radio_process_frame(frame->data, FRAME_LEN, mod);
    return 0;
}

// compare bite stream to differentialy encoded synchronization sequence
static int cmp_frame_sync(const uint8_t *buf, int invert)
{
    const uint8_t frame_dsync_pos[] = { 1, 0, 1, 0, 0, 1, 1, };
    const uint8_t frame_dsync_neg[] = { 0, 1, 0, 1, 1, 0, 0, };
    const uint8_t *frame_dsync = invert ? frame_dsync_neg : frame_dsync_pos;
    int sync_err = 0;
    for(int i = 0; i < sizeof(frame_dsync_pos); ++i) {
        if (frame_dsync[i] != buf[i + 1]) {
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
static int find_frame_sync(tetrapol_t *t)
{
    int offs = 0;
    int sync_err = MAX_FRAME_SYNC_ERR + 1;
    int invert = 0;
    while (offs + FRAME_LEN + FRAME_SYNC_LEN < t->data_len) {
        invert = 0;
        const uint8_t *buf = t->buf + offs;
        sync_err = cmp_frame_sync(buf, invert) +
            cmp_frame_sync(buf + FRAME_LEN, invert);
        if (sync_err <= MAX_FRAME_SYNC_ERR) {
            break;
        }

        invert = 1;
        sync_err = cmp_frame_sync(buf, invert) +
            cmp_frame_sync(buf + FRAME_LEN, invert);
        if (sync_err <= MAX_FRAME_SYNC_ERR) {
            break;
        }

        ++offs;
    }

    t->data_len -= offs;
    memmove(t->buf, t->buf + offs, t->data_len);

    if (sync_err <= MAX_FRAME_SYNC_ERR) {
        t->last_sync_err = 0;
        t->total_sync_err = 0;
        t->invert = invert;
        return 1;
    }

    return 0;
}

/// return number of acquired frames (0 or 1) or -1 on error
static int get_frame(tetrapol_t *t, frame_t *frame)
{
    if (t->data_len < FRAME_LEN) {
        return 0;
    }
    const int sync_err = cmp_frame_sync(t->buf, t->invert);
    if (sync_err + t->last_sync_err > MAX_FRAME_SYNC_ERR) {
        t->total_sync_err = 1 + 2 * t->total_sync_err;
        if (t->total_sync_err >= FRAME_LEN) {
            return -1;
        }
    } else {
        t->total_sync_err = 0;
    }

    t->last_sync_err = sync_err;
    memcpy(frame->data, t->buf, FRAME_LEN);
    // previous input bite for differential decoding is
    // defined only by signal polarity, ignore data from sync bits
    differential_dec(frame->data + FRAME_SYNC_LEN,
            FRAME_LEN - FRAME_SYNC_LEN, t->invert);
    // copy header, decoding it from frame leads only to errors
    memcpy(frame->data, frame_sync, sizeof(frame_sync));
    t->data_len -= FRAME_LEN;
    memmove(t->buf, t->buf + FRAME_LEN, t->data_len);

    return 1;
}

int tetrapol_main(tetrapol_t *t)
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

#define FRAME_DATA_LEN 74

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


static const int K[] = {
    1, 77, 38, 114, 20, 96, 59, 135, 3, 79, 41, 117, 23, 99, 62, 138, 5, 81, 44, 120, 26, 102, 65, 141, 8, 84, 47, 123, 29, 105, 68, 144, 11, 87, 50, 126, 32, 108, 71, 147, 14, 90, 53, 129, 35, 111, 74, 150, 17, 93, 56, 132, 37, 112, 76, 148, 2, 88, 40, 115, 19, 97, 58, 133, 4, 75, 43, 118, 22, 100, 61, 136, 7, 85, 46, 121, 25, 103, 64, 139, 10, 82, 49, 124, 28, 106, 67, 142, 13, 91, 52, 127, 31, 109, 73, 145, 16, 94, 55, 130, 34, 113, 70, 151, 0, 80, 39, 116, 21, 95, 57, 134, 6, 78, 42, 119, 24, 98, 60, 137, 9, 83, 45, 122, 27, 101, 63, 140, 12, 86, 48, 125, 30, 104, 66, 143, 15, 89, 51, 128, 33, 107, 69, 146, 18, 92, 54, 131, 36, 110, 72, 149
};

static void deinterleave_frame(const uint8_t *e, uint8_t *c, int len)
{
    for(int j = 0; j < len; j++) {
        c[j] = e[K[j]];
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

static void diffdec_frame(const uint8_t *e, uint8_t *ex, int len)
{
    // in spec is e[0]+f[7], but since f[7] is always 0 fuck you
    ex[0] = e[0];
    for(int j = 1; j < len; j++) {
        ex[j] = e[j] ^ e[j-pre_cod[j]];
        //		if(is_in_code(j))
        //			ex[j] = e[j] ^ e[j-2];
        //		else
        //			ex[j] = e[j] ^ e[j-1];
    }
}

static void descramble(const uint8_t *in, uint8_t *out, int len, int scr)
{
    if (scr == 0) {
        memcpy(out, in, len);
    } else {
        for(int k=0; k < len; k++)
            out[k] = in[k] ^ scramb_table[(k+scr)%127];
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

static void radio_process_frame(const uint8_t *f, int framelen, int modulo)
{
    int scr, scr2, i, j;
    uint8_t asbx, asby, fn0, fn1;
    uint8_t frame_bord[FRAME_BITORDER_LEN];

    //	printf("s=");
    //	print_buf(scramb_table,127);
    //	printf("f=");
    //	print_buf(f,160);

    //	printf("Attempting descramble\n");
    int scr_ok=0;
    for(scr=0; scr<=127; scr++) {
        //		printf("trying scrambling %i\n", scr);

        uint8_t descr[BUF_LEN];
        descramble(f+8, descr, 152, scr);
        //		printf("descr=");
        //		print_buf(descr, 152);

        uint8_t e[BUF_LEN];
        diffdec_frame(descr, e, 152);
        //		printf("e=");
        //		print_buf(e,152);

        uint8_t c[BUF_LEN];
        deinterleave_frame(e, c, 152);
        //		printf("c=");
        //		print_buf(c,152);

        uint8_t d[FRAME_DATA_LEN];
        decode_data_frame(c, d);
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
        printf("OK mod=%03i fn=%i%i asb=%i%i scr=%03i ", modulo, fn0, fn1, asbx, asby, scr2);
        for (i=0; i<8; i++) {
            for(j=0; j<8; j++)
                printf("%i", frame_bord[i*8+j]);
            printf(" ");
        }
        print_buf(frame_bord, 64);
        multiblock_process(frame_bord, 2*fn0 + fn1, modulo);
    } else {
        printf("ERR2 mod=%03i\n", modulo);
        multiblock_reset();
        segmentation_reset();
    }
}
