#include "test_support.h"
TEST(Jobs, AddFindRemoveAndCleanup) {
 shell_context_t c{}; c.next_job_id=1; jobs_add(&c,12345,"sleep 1",JOB_RUNNING);
 ASSERT_NE(c.jobs,nullptr); EXPECT_EQ(c.jobs->job_id,1U); EXPECT_EQ(jobs_find(&c,1)->pgid,12345);
 EXPECT_EQ(jobs_find(&c,99),nullptr); jobs_remove(&c,1); EXPECT_EQ(c.jobs,nullptr);
 jobs_add(&c,12346,"sleep 2",JOB_STOPPED); jobs_cleanup(&c); EXPECT_EQ(c.jobs,nullptr);
}
TEST(Jobs, NullAndEmptyOperationsAreSafe) {
 EXPECT_EQ(jobs_find(nullptr,1),nullptr); jobs_add(nullptr,1,"x",JOB_RUNNING);
 jobs_remove(nullptr,1); jobs_list(nullptr); jobs_reap(nullptr); jobs_cleanup(nullptr);
 shell_context_t c{}; jobs_list(&c); jobs_reap(&c); jobs_cleanup(&c);
}
