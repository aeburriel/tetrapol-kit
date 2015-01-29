#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

// include, we are testing static methods
#include "phys_ch.c"


// the goal is just to make sure the function provides the same results
// after refactorization
static void test_frame_decode(void **state)
{
    {
        const frame_t f = {
            .data = {
                1, 1, 0, 1, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0, 1, 0,
                1, 0, 1, 0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0, 1, 0,
                1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1,
                1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
                1, 0, 1, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                1, 1, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 1,
                1, 0, 1, 0, 0, 1, 1, 1,
            }
        };
        const uint8_t res_exp[] = {
            1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1,
            0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            1, 1, 0, 0, 0, 0, 1, 1, 1, 1,
        };

        uint8_t res[74];
        memset(res, 3, sizeof(res));
        decode_data_frame(&f, res);
        assert_memory_equal(res_exp, res, sizeof(res_exp));
    }
    {
        const frame_t f = {
            .data = {
                1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
                0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 1, 1, 1, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 1, 1, 1,
                1, 1, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1,
                1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1, 1, 0,
                1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0,
                1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0,
                1, 1, 0, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0,
                1, 0, 0, 0, 1, 0, 1, 1,
            }
        };
        const uint8_t res_exp[] = {
            1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0,
            1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0,
            0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0,
            1, 1, 0, 0, 0, 1, 0, 1, 0, 1,
        };

        uint8_t res[74];
        memset(res, 3, sizeof(res));
        decode_data_frame(&f, res);
        assert_memory_equal(res_exp, res, sizeof(res_exp));
    }
    {
        const frame_t f = {
            .data = {
                1, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1,
                0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0,
                0, 1, 1, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0,
                1, 1, 0, 1, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1,
                1, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 0,
                1, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0,
                0, 1, 0, 0, 1, 0, 1, 1, 1, 1, 0, 1, 1, 0, 0, 1,
                0, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 0, 0, 1, 1, 0,
                1, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0,
                0, 1, 1, 0, 0, 1, 1, 1,
            }
        };
        const uint8_t res_exp[] = {
            1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 1,
            1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1,
            1, 1, 0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0, 1, 1, 1,
            0, 1, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1,
            1, 1, 0, 0, 0, 1, 0, 1, 1, 1,
        };

        uint8_t res[74];
        memset(res, 3, sizeof(res));
        decode_data_frame(&f, res);
        assert_memory_equal(res_exp, res, sizeof(res_exp));
    }
}

// the goal is just to make sure the function provides the same results
// after refactorization
static void test_frame_deinterleave(void **state)
{
    (void) state;   // unused

    frame_t data;
    for (int i = 0; i < FRAME_DATA_LEN; ++i) {
        data.data[i] = 0x7f & (i + 1 + 8);
    }

    uint8_t data_exp[FRAME_DATA_LEN] = {
        0x0a, 0x56, 0x2f, 0x7b,
        0x1d, 0x69, 0x44, 0x10, 0x0c, 0x58, 0x32, 0x7e, 0x20, 0x6c, 0x47, 0x13,
        0x0e, 0x5a, 0x35, 0x01, 0x23, 0x6f, 0x4a, 0x16, 0x11, 0x5d, 0x38, 0x04,
        0x26, 0x72, 0x4d, 0x19, 0x14, 0x60, 0x3b, 0x07, 0x29, 0x75, 0x50, 0x1c,
        0x17, 0x63, 0x3e, 0x0a, 0x2c, 0x78, 0x53, 0x1f, 0x1a, 0x66, 0x41, 0x0d,
        0x2e, 0x79, 0x55, 0x1d, 0x0b, 0x61, 0x31, 0x7c, 0x1c, 0x6a, 0x43, 0x0e,
        0x0d, 0x54, 0x34, 0x7f, 0x1f, 0x6d, 0x46, 0x11, 0x10, 0x5e, 0x37, 0x02,
        0x22, 0x70, 0x49, 0x14, 0x13, 0x5b, 0x3a, 0x05, 0x25, 0x73, 0x4c, 0x17,
        0x16, 0x64, 0x3d, 0x08, 0x28, 0x76, 0x52, 0x1a, 0x19, 0x67, 0x40, 0x0b,
        0x2b, 0x7a, 0x4f, 0x20, 0x09, 0x59, 0x30, 0x7d, 0x1e, 0x68, 0x42, 0x0f,
        0x0f, 0x57, 0x33, 0x00, 0x21, 0x6b, 0x45, 0x12, 0x12, 0x5c, 0x36, 0x03,
        0x24, 0x6e, 0x48, 0x15, 0x15, 0x5f, 0x39, 0x06, 0x27, 0x71, 0x4b, 0x18,
        0x18, 0x62, 0x3c, 0x09, 0x2a, 0x74, 0x4e, 0x1b, 0x1b, 0x65, 0x3f, 0x0c,
        0x2d, 0x77, 0x51, 0x1e,
    };

    frame_deinterleave(&data);
    assert_memory_equal(data_exp, data.data, FRAME_DATA_LEN);
}

// the goal is just to make sure the function provides the same results
// after refactorization
static void test_frame_diff_dec(void **state)
{
    (void) state;   // unused

    frame_t data_in;
    for (int i = 0; i < FRAME_DATA_LEN; ++i) {
        data_in.data[i] = 1 << (i % 7);
    }
    uint8_t data_exp[FRAME_DATA_LEN] = {
        0x01, 0x03, 0x06, 0x0c, 0x18, 0x30, 0x60, 0x21, 0x03, 0x06, 0x0a, 0x18,
        0x30, 0x50, 0x41, 0x03, 0x05, 0x0c, 0x18, 0x28, 0x60, 0x41, 0x42, 0x06,
        0x0c, 0x14, 0x30, 0x60, 0x21, 0x03, 0x06, 0x0a, 0x18, 0x30, 0x50, 0x41,
        0x03, 0x05, 0x0c, 0x18, 0x28, 0x60, 0x41, 0x42, 0x06, 0x0c, 0x14, 0x30,
        0x60, 0x21, 0x03, 0x06, 0x0a, 0x18, 0x30, 0x50, 0x41, 0x03, 0x05, 0x0c,
        0x18, 0x28, 0x60, 0x41, 0x42, 0x06, 0x0c, 0x14, 0x30, 0x60, 0x21, 0x03,
        0x06, 0x0a, 0x18, 0x30, 0x50, 0x41, 0x03, 0x06, 0x0c, 0x18, 0x30, 0x50,
        0x41, 0x03, 0x05, 0x0c, 0x18, 0x28, 0x60, 0x41, 0x42, 0x06, 0x0c, 0x14,
        0x30, 0x60, 0x21, 0x03, 0x06, 0x0a, 0x18, 0x30, 0x50, 0x41, 0x03, 0x05,
        0x0c, 0x18, 0x28, 0x60, 0x41, 0x42, 0x06, 0x0c, 0x14, 0x30, 0x60, 0x21,
        0x03, 0x06, 0x0a, 0x18, 0x30, 0x50, 0x41, 0x03, 0x05, 0x0c, 0x18, 0x28,
        0x60, 0x41, 0x42, 0x06, 0x0c, 0x14, 0x30, 0x60, 0x21, 0x03, 0x06, 0x0a,
        0x18, 0x30, 0x50, 0x41, 0x03, 0x05, 0x0c, 0x18,
    };

    frame_diff_dec(&data_in);
    assert_memory_equal(data_exp, data_in.data, FRAME_DATA_LEN);
}

static void test_mk_crc5(void **state)
{
    (void) state;   // unused

    {
        const uint8_t in[] = { 1, 0, 1, 0, 1, 0 };
        const uint8_t out_exp[] = { 0, 1, 0, 1, 1 };
        uint8_t out[5];
        mk_crc5(out, in, sizeof(out_exp));
        assert_memory_equal(out, out_exp, sizeof(out_exp));
    }

    {
        const uint8_t in[] = { 0, 0, 0, 0, 0, 0 };
        const uint8_t out_exp[] = { 0, 0, 0, 0, 0 };
        uint8_t out[5];
        mk_crc5(out, in, sizeof(out_exp));
        assert_memory_equal(out, out_exp, sizeof(out_exp));
    }

    {
        const uint8_t in[] = { 1, 1, 1, 1, 1, 1 };
        const uint8_t out_exp[] = { 0, 1, 1, 0, 0 };
        uint8_t out[5];
        mk_crc5(out, in, sizeof(out_exp));
        assert_memory_equal(out, out_exp, sizeof(out_exp));
    }
}

int main(void)
{
    const UnitTest tests[] = {
        unit_test(test_frame_decode),
        unit_test(test_frame_deinterleave),
        unit_test(test_frame_diff_dec),
        unit_test(test_mk_crc5),
    };

    return run_tests(tests);
}
