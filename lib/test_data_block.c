#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

// include, we are testing static methods
#include "data_block.c"

// the goal is just to make sure the function provides the same results
// after refactorization
static void test_frame_decode_data(void **state)
{
    {
        const uint8_t data[] = {
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
        };
        const uint8_t res_exp[] = {
            1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1,
            0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0
        };

        data_block_t res;
        memset(&res, 3, sizeof(res));
        data_block_decode_frame(&res, data, FRAME_NO_UNKNOWN, FRAME_TYPE_DATA);
        assert_memory_equal(res_exp, res.data, sizeof(res_exp));
        assert_int_equal(res.nerrs, 0);
    }
    {
        const uint8_t data[] = {
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
        };
        const uint8_t res_exp[] = {
            1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0,
            1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0,
            0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0,
            1, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0,
        };

        data_block_t res;
        memset(&res, 3, sizeof(res));
        data_block_decode_frame(&res, data, FRAME_NO_UNKNOWN, FRAME_TYPE_DATA);
        assert_memory_equal(res_exp, res.data, sizeof(res_exp));
        assert_int_equal(res.nerrs, 0);
    }
    {
        const uint8_t data[] = {
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
        };
        const uint8_t res_exp[] = {
            1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 1,
            1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1,
            1, 1, 0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0, 1, 1, 1,
            0, 1, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1,
            1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0,
        };

        data_block_t res;
        memset(&res, 3, sizeof(res));
        data_block_decode_frame(&res, data, FRAME_NO_UNKNOWN, FRAME_TYPE_DATA);
        assert_memory_equal(res_exp, res.data, sizeof(res_exp));
        assert_int_equal(res.nerrs, 0);
    }
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
        unit_test(test_frame_decode_data),
        unit_test(test_mk_crc5),
    };

    return run_tests(tests);
}
