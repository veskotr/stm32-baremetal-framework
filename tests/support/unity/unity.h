#pragma once

#include <stddef.h>

void UnityBegin(const char *name);
int UnityEnd(void);
void UnityRunTest(void (*test_func)(void), const char *name);
void UnityAssert(int condition, const char *message, const char *file, int line);
void UnityAssertEqualInt(int expected, int actual, const char *message, const char *file, int line);
void UnityAssertEqualUInt(unsigned int expected, unsigned int actual, const char *message, const char *file, int line);
void UnityAssertEqualString(const char *expected, const char *actual, const char *message, const char *file, int line);
void UnityAssertEqualMemory(const void *expected,
                            const void *actual,
                            size_t length,
                            const char *message,
                            const char *file,
                            int line);

#define UNITY_BEGIN() UnityBegin(__FILE__)
#define UNITY_END() UnityEnd()
#define RUN_TEST(test_func) UnityRunTest((test_func), #test_func)

#define TEST_ASSERT_TRUE(condition) UnityAssert((condition), #condition, __FILE__, __LINE__)
#define TEST_ASSERT_FALSE(condition) UnityAssert(!(condition), "!(" #condition ")", __FILE__, __LINE__)
#define TEST_ASSERT_NULL(pointer) UnityAssert((pointer) == NULL, #pointer " == NULL", __FILE__, __LINE__)
#define TEST_ASSERT_NOT_NULL(pointer) UnityAssert((pointer) != NULL, #pointer " != NULL", __FILE__, __LINE__)
#define TEST_ASSERT_EQUAL(expected, actual) UnityAssertEqualInt((int)(expected), (int)(actual), NULL, __FILE__, __LINE__)
#define TEST_ASSERT_EQUAL_INT(expected, actual) UnityAssertEqualInt((expected), (actual), NULL, __FILE__, __LINE__)
#define TEST_ASSERT_EQUAL_UINT(expected, actual) UnityAssertEqualUInt((expected), (actual), NULL, __FILE__, __LINE__)
#define TEST_ASSERT_EQUAL_UINT16(expected, actual) UnityAssertEqualUInt((unsigned int)(expected), (unsigned int)(actual), NULL, __FILE__, __LINE__)
#define TEST_ASSERT_EQUAL_STRING(expected, actual) UnityAssertEqualString((expected), (actual), NULL, __FILE__, __LINE__)
#define TEST_ASSERT_EQUAL_MEMORY(expected, actual, length) UnityAssertEqualMemory((expected), (actual), (length), NULL, __FILE__, __LINE__)
