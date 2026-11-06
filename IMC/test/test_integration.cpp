#include "test_support.h"
TEST(Integration, BinaryStartsAndAcceptsExit) {
 int result=std::system("printf 'help\\nexit\\n' | ./build/imc >/dev/null 2>&1");
 EXPECT_TRUE(WIFEXITED(result));EXPECT_EQ(WEXITSTATUS(result),0);
}
