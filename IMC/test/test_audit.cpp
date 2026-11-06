#include "test_support.h"
TEST(Audit, WritesSuccessAndFailureRecordsAndReports) {
 auto f=temp_name("audit"); imc_config_t c{}; std::snprintf(c.audit_path,sizeof(c.audit_path),"%s",f.c_str());
 audit_record(&c,getpid(),"pwd",0); audit_record(&c,getpid(),"bad",1);
 std::ifstream in(f); std::string x((std::istreambuf_iterator<char>(in)),{});
 EXPECT_NE(x.find("SUCCESS"),std::string::npos); EXPECT_NE(x.find("FAILED"),std::string::npos);
 testing::internal::CaptureStdout(); audit_report(&c); auto out=testing::internal::GetCapturedStdout();
 EXPECT_NE(out.find("total=2"),std::string::npos); std::remove(f.c_str());
}
TEST(Audit, MissingFileAndNullArguments) {
 imc_config_t c{}; std::snprintf(c.audit_path,sizeof(c.audit_path),"/tmp/imc_absent_audit");
 testing::internal::CaptureStdout(); audit_report(&c); auto out=testing::internal::GetCapturedStdout();
 EXPECT_NE(out.find("No audit"),std::string::npos); audit_record(nullptr,1,"x",0); audit_record(&c,1,nullptr,0); audit_report(nullptr);
}
