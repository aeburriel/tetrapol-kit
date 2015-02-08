#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

// include, we are testing static methods
#include "data_frame.c"

static void test_pack_bits(void **state)
{
    {
        const uint8_t data[] = {
            1, 1, 0, 1,  0, 1, 0, 0,  0, 1, 1, 0,  1, 0, 1, 0,
        };
        const uint8_t res_exp[] = { 0x2b, 0x56, };

        uint8_t res[2];
        memset(res, 0, sizeof(res));
        pack_bits(res, data, 0, sizeof(data));
        assert_memory_equal(res_exp, res, sizeof(res_exp));
    }

    {
        const uint8_t data1[] = {
            1, 1, 0, 1,  0, 1, 0, 0,  0, 1, 1, 0,
        };
        const uint8_t data2[] = {
            1, 0, 1, 0,
            1, 0, 0, 1,  1, 1, 0, 1,  1, 0, 1, 0,  0, 0, 1, 1,
        };
        const uint8_t res_exp[] = { 0x2b, 0x56, 0xb9, 0xc5, };

        uint8_t res[4];
        memset(res, 0, sizeof(res));
        pack_bits(res, data1, 0, sizeof(data1));
        pack_bits(res, data2, sizeof(data1), sizeof(data2));
        assert_memory_equal(res_exp, res, sizeof(res_exp));
    }
}

int main(void)
{
    const UnitTest tests[] = {
        unit_test(test_pack_bits),
    };

    return run_tests(tests);
}
