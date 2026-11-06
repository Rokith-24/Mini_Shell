#include "test_support.h"
TEST(Config, DefaultsAndMissingConfig) {
 imc_config_t c{}; config_defaults(&c);
 EXPECT_STREQ(c.history_path,"data/history.txt"); EXPECT_STREQ(c.audit_path,"data/audit.log");
 EXPECT_STREQ(c.log_path,"data/imc.log"); EXPECT_GT(c.history_size,0U);
 EXPECT_EQ(config_load(&c,"/tmp/imc_nonexistent_config"),IMC_OK);
}
TEST(Config, LoadsAllSupportedSettingsAndIgnoresInvalidValues) {
 auto f=temp_name("config"); {std::ofstream o(f); o<<"history_path=custom/history\\naudit_path=custom/audit\\nlog_path=custom/log\\nhistory_size=22\\nshow_timestamp=true\\nunknown=x\\nhistory_size=-1\\n";}
 imc_config_t c{}; ASSERT_EQ(config_load(&c,f.c_str()),IMC_OK);
 EXPECT_STREQ(c.history_path,"custom/history"); EXPECT_STREQ(c.audit_path,"custom/audit");
 EXPECT_STREQ(c.log_path,"custom/log"); EXPECT_EQ(c.history_size,22U); EXPECT_TRUE(c.show_timestamp);
 std::remove(f.c_str());
}
TEST(Config, NullArgumentsReturnError) { EXPECT_EQ(config_load(nullptr,"x"),IMC_ERROR); imc_config_t c{}; EXPECT_EQ(config_load(&c,nullptr),IMC_ERROR); config_defaults(nullptr); }
