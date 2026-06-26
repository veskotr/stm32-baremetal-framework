#include "unity.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int unity_failures;
static int unity_tests;
static const char *unity_current_test;

void UnityBegin(const char *name)
{
    unity_failures = 0;
    unity_tests = 0;
    unity_current_test = NULL;
    printf("==== %s ====\n", name);
}

int UnityEnd(void)
{
    printf("==== %d test(s), %d failure(s) ====\n", unity_tests, unity_failures);
    return unity_failures == 0 ? 0 : 1;
}

void UnityRunTest(void (*test_func)(void), const char *name)
{
    unity_current_test = name;
    unity_tests++;
    printf("RUN %s\n", name);
    test_func();
}

void UnityAssert(int condition, const char *message, const char *file, int line)
{
    if (condition)
    {
        return;
    }

    unity_failures++;
    printf("%s:%d: FAIL %s: %s\n", file, line, unity_current_test, message);
}

void UnityAssertEqualInt(int expected, int actual, const char *message, const char *file, int line)
{
    if (expected == actual)
    {
        return;
    }

    unity_failures++;
    printf("%s:%d: FAIL %s: expected %d got %d", file, line, unity_current_test, expected, actual);
    if (message != NULL)
    {
        printf(" (%s)", message);
    }
    printf("\n");
}

void UnityAssertEqualUInt(unsigned int expected, unsigned int actual, const char *message, const char *file, int line)
{
    if (expected == actual)
    {
        return;
    }

    unity_failures++;
    printf("%s:%d: FAIL %s: expected %u got %u", file, line, unity_current_test, expected, actual);
    if (message != NULL)
    {
        printf(" (%s)", message);
    }
    printf("\n");
}

void UnityAssertEqualString(const char *expected, const char *actual, const char *message, const char *file, int line)
{
    const int matches = (expected == NULL && actual == NULL) ||
                        (expected != NULL && actual != NULL && strcmp(expected, actual) == 0);
    if (matches)
    {
        return;
    }

    unity_failures++;
    printf("%s:%d: FAIL %s: expected \"%s\" got \"%s\"", file, line, unity_current_test,
           expected == NULL ? "(null)" : expected,
           actual == NULL ? "(null)" : actual);
    if (message != NULL)
    {
        printf(" (%s)", message);
    }
    printf("\n");
}

void UnityAssertEqualMemory(const void *expected,
                            const void *actual,
                            size_t length,
                            const char *message,
                            const char *file,
                            int line)
{
    if (expected != NULL && actual != NULL && memcmp(expected, actual, length) == 0)
    {
        return;
    }

    unity_failures++;
    printf("%s:%d: FAIL %s: memory differs over %zu byte(s)", file, line, unity_current_test, length);
    if (message != NULL)
    {
        printf(" (%s)", message);
    }
    printf("\n");
}
