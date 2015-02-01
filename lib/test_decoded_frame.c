#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

// include, we are testing static methods
#include "decoded_frame.c"

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
        unit_test(test_mk_crc5),
    };

    return run_tests(tests);
}
