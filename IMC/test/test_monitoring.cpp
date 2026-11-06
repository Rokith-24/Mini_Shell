#include "test_support.h"
TEST(Monitoring, UnknownCommandIsNotHandled) {
 shell_context_t c{}; bool h=true; char n[]="not_monitoring"; command_stage_t s{}; s.argv[0]=n; s.argc=1;
 EXPECT_EQ(monitoring_execute(&c,&s,&h),0); EXPECT_FALSE(h);
}
TEST(Monitoring, HealthcheckAndLogOperations) {
 shell_context_t c{}; bool h=false; char health[]="healthcheck"; command_stage_t s{}; s.argv[0]=health;s.argc=1;
 EXPECT_EQ(monitoring_execute(&c,&s,&h),0); EXPECT_TRUE(h);
 auto f=temp_name("monitor"); {std::ofstream o(f);o<<"INFO startup\\nERROR failed\\n";}
 char cmd[]="log-search", key[]="ERROR"; s={};s.argv[0]=cmd;s.argv[1]=key;s.argv[2]=const_cast<char*>(f.c_str());s.argc=3;h=false;
 testing::internal::CaptureStdout(); EXPECT_EQ(monitoring_execute(&c,&s,&h),0); auto out=testing::internal::GetCapturedStdout(); EXPECT_TRUE(h); EXPECT_NE(out.find("ERROR"),std::string::npos);
 char sum[]="log-summary";s={};s.argv[0]=sum;s.argv[1]=const_cast<char*>(f.c_str());s.argc=2;h=false;
 testing::internal::CaptureStdout(); EXPECT_EQ(monitoring_execute(&c,&s,&h),0); testing::internal::GetCapturedStdout(); EXPECT_TRUE(h); std::remove(f.c_str());
}
TEST(Monitoring, InvalidArgumentsAreHandled) { shell_context_t c{}; bool h=false; char n[]="log-search"; command_stage_t s{};s.argv[0]=n;s.argc=1; EXPECT_EQ(monitoring_execute(&c,&s,&h),2); EXPECT_TRUE(h); }
