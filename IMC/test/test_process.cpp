#include "test_support.h"
TEST(Process, ExecutesExternalCommand) {
 shell_context_t c{};config_defaults(&c.config);char line[]="true";parsed_line_t p{};char e[128]{};
 ASSERT_EQ(parse_line(line,&p,e,sizeof(e)),IMC_OK);EXPECT_EQ(shell_execute_pipeline(&c,&p.commands[0]),0);free_parsed(&p);jobs_cleanup(&c);
}
TEST(Process, ExecutesPipeline) {
 shell_context_t c{};config_defaults(&c.config);char line[]="printf x | wc -c";parsed_line_t p{};char e[128]{};
 ASSERT_EQ(parse_line(line,&p,e,sizeof(e)),IMC_OK);EXPECT_EQ(shell_execute_pipeline(&c,&p.commands[0]),0);free_parsed(&p);
}
TEST(Process, RunsBackgroundJob) {
 shell_context_t c{};c.next_job_id=1;config_defaults(&c.config);char line[]="sleep 2 &";parsed_line_t p{};char e[128]{};
 ASSERT_EQ(parse_line(line,&p,e,sizeof(e)),IMC_OK);EXPECT_EQ(shell_execute_pipeline(&c,&p.commands[0]),0);
 ASSERT_NE(c.jobs,nullptr);kill(-c.jobs->pgid,SIGTERM);jobs_cleanup(&c);free_parsed(&p);
}
