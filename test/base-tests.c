#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <regex.h>
#include <cmocka.h>

static void test_gnu_regex(void **state)
{
    regex_t rc;

    // MAC address
    assert_int_equal(regcomp(&rc, "^([0-9A-Fa-f]{2}[:-]{1}){5}[0-9A-Fa-f]{2}$", REG_EXTENDED), 0);
    assert_int_equal(regexec(&rc, "00:00:00:00:00:00", 0, 0, 0), 0);
    assert_int_equal(regexec(&rc, "01:0a:02:03:ff:32", 0, 0, 0), 0);
    assert_int_equal(regexec(&rc, "ba:d0:be:ef:0b:ad", 0, 0, 0), 0);
    assert_int_equal(regexec(&rc, "00:00:00:00:00:00", 0, 0, 0), 0);
    assert_int_equal(regexec(&rc, "00:00:00:00:00:00", 0, 0, 0), 0);
    assert_int_not_equal(regexec(&rc, "00:00:00:00:00:00:00", 0, 0, 0), 0);
    assert_int_not_equal(regexec(&rc, "00:00:00:00:00", 0, 0, 0), 0);
    assert_int_not_equal(regexec(&rc, "00:00:00:00#00:00", 0, 0, 0), 0);
    assert_int_not_equal(regexec(&rc, "00:00:00:0000:00", 0, 0, 0), 0);
    assert_int_not_equal(regexec(&rc, "000:00:00:00:00:00", 0, 0, 0), 0);
    assert_int_not_equal(regexec(&rc, "ffff00:00:00:00:00:00", 0, 0, 0), 0);
    assert_int_not_equal(regexec(&rc, "00:00:0x:00:00:00", 0, 0, 0), 0);
    assert_int_not_equal(regexec(&rc, "00:00::00:00:00", 0, 0, 0), 0);
    regfree(&rc);
}

int main(int argc, char **argv)
{
    const struct CMUnitTest tests[] = {
            cmocka_unit_test(test_gnu_regex),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}