#include "test_support.h"
TEST(Shell, NullContextRejected) { EXPECT_EQ(shell_run(nullptr),IMC_ERROR); }
TEST(Shell, ReadsExitFromPipe) {
 int fds[2];ASSERT_EQ(pipe(fds),0);const char cmd[]="exit\\n";ASSERT_EQ(write(fds[1],cmd,sizeof(cmd)-1),(ssize_t)(sizeof(cmd)-1));close(fds[1]);
 pid_t child=fork();ASSERT_GE(child,0);if(child==0){dup2(fds[0],STDIN_FILENO);close(fds[0]);shell_context_t c{};c.next_job_id=1;config_defaults(&c.config);_exit(shell_run(&c)==0?0:1);}
 close(fds[0]);int st=0;waitpid(child,&st,0);EXPECT_TRUE(WIFEXITED(st));EXPECT_EQ(WEXITSTATUS(st),0);
}
