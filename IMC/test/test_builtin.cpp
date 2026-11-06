#include "test_support.h"
TEST(Builtin, ParentOnlyClassification) {
 EXPECT_TRUE(builtin_is_parent_only("cd")); EXPECT_TRUE(builtin_is_parent_only("exit"));
 EXPECT_TRUE(builtin_is_parent_only("fg")); EXPECT_TRUE(builtin_is_parent_only("bg"));
 EXPECT_FALSE(builtin_is_parent_only("pwd")); EXPECT_FALSE(builtin_is_parent_only(nullptr));
}
TEST(Builtin, HelpAndExitAreHandled) {
 shell_context_t c{}; bool handled=false; char h[]="help"; command_stage_t s{}; s.argv[0]=h; s.argc=1;
 EXPECT_EQ(builtin_execute(&c,&s,&handled),0); EXPECT_TRUE(handled);
 char ex[]="exit", code[]="7"; s.argv[0]=ex; s.argv[1]=code; s.argc=2; handled=false;
 EXPECT_EQ(builtin_execute(&c,&s,&handled),0); EXPECT_TRUE(handled); EXPECT_TRUE(c.should_exit); EXPECT_EQ(c.last_status,7);
}
TEST(Builtin, UnknownCommandIsNotHandled) { shell_context_t c{}; bool h=true; char n[]="not_a_builtin"; command_stage_t s{}; s.argv[0]=n; s.argc=1; builtin_execute(&c,&s,&h); EXPECT_FALSE(h); }
