#ifndef _TEST
#define _TEST

/* internals */
typedef int (*TEST__fn)(void);
int TEST__register(const char *name, TEST__fn fn);
int TEST__deregister(const char *name);
int TEST__run(const char *name);
int TEST__assert(int cond, const char *fail_msg, ...);

// register a test
#define TEST_REGISTER(name) TEST__register((#name), (name))
// deregisters a test
#define TEST_DEREGISTER(name) TEST__deregister(#name)
// run a test case
#define TEST_RUN(name) TEST__run(#name)
// assert
#define TEST_ASSERT(cond, fail_msg) TEST__assert((cond), (fail_msg))


#define EXPECT_EQ(a, b) TEST__assert((a) == (b),                \
    "expected '%s' to be equal-to '%s' (==)",                   \
    #a, #b                                                      \
  )
#define EXPECT_NE(a, b) TEST__assert((a) != (b),                \
    "expected '%s' to be not-equal-to '%s' (!=)",               \
    #a, #b                                                      \
  )
#define EXPECT_GT(a, b) TEST__assert((a) > (b),                 \
    "expected '%s' to be greater-than '%s' (>)",                \
    #a, #b                                                      \
  )
#define EXPECT_GE(a, b) TEST__assert((a) >= (b),                \
    "expected '%s' to be greater-than-or-equal-to '%s' (>=)",   \
    #a, #b                                                      \
  )
#define EXPECT_LT(a, b) TEST__assert((a) < (b),                 \
    "expected '%s' to be less-than '%s' (<)",                   \
    #a, #b                                                      \
  )
#define EXPECT_LE(a, b) TEST__assert((a) <= (b),                \
    "expected '%s' to be less-than-or-equal-to '%s' (<=)",      \
    #a, #b,                                                     \
  )
#define EXPECT_TRUE(a) TEST__assert((a),                        \
    "expected '%s' to be true, but got false",                  \
    #a                                                          \
  )
#define EXPECT_FALSE(a) TEST__assert(!(a),                      \
    "expected '%s' to be false, but got true",                  \
    #a                                                          \
  )

#endif // _TEST

