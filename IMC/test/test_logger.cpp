#include "test_support.h"
TEST(Logger, WritesMessagesAndCloses) {
 auto f=temp_name("logger"); logger_init(f.c_str()); logger_log(LOG_INFO,"message %d",42);
 logger_log(LOG_WARNING,"warning"); logger_log(LOG_ERROR,"error"); logger_close();
 std::ifstream in(f); std::string x((std::istreambuf_iterator<char>(in)),{});
 EXPECT_NE(x.find("message 42"),std::string::npos); EXPECT_NE(x.find("warning"),std::string::npos); std::remove(f.c_str());
}
TEST(Logger, NullPathAndUnknownLevelAreHandled) { logger_init(nullptr); logger_log(LOG_DEBUG,"debug"); logger_log((log_level_t)99,"unknown"); logger_close(); }
