#include "test_support.h"
TEST(History, AppendsAndDisplaysCommands) {
 auto f=temp_name("history"); imc_config_t c{}; std::snprintf(c.history_path,sizeof(c.history_path),"%s",f.c_str());
 c.history_size=10; history_add(&c,"pwd"); history_add(&c,"ls");
 std::ifstream in(f); std::string x((std::istreambuf_iterator<char>(in)),{});
 EXPECT_NE(x.find("pwd"),std::string::npos); EXPECT_NE(x.find("ls"),std::string::npos);
 testing::internal::CaptureStdout(); history_show(&c,10); auto out=testing::internal::GetCapturedStdout();
 EXPECT_NE(out.find("pwd"),std::string::npos); std::remove(f.c_str());
}
TEST(History, IgnoresInvalidInputs) { history_add(nullptr,"x"); imc_config_t c{}; history_add(&c,nullptr); history_add(&c,""); history_show(nullptr,2); }
