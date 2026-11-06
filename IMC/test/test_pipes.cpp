#include "test_support.h"
TEST(Pipes, ConnectsPipeInChild) {
 int p[1][2]; ASSERT_EQ(pipe(p[0]),0); pid_t child=fork(); ASSERT_GE(child,0);
 if(child==0) _exit(pipe_connect_child(p,0,2)==IMC_OK?0:1);
 close(p[0][0]);close(p[0][1]);int st=0;waitpid(child,&st,0);EXPECT_TRUE(WIFEXITED(st));EXPECT_EQ(WEXITSTATUS(st),0);
}
TEST(Pipes, RejectsInvalidArgumentsAndClosesDescriptors) {
 EXPECT_EQ(pipe_connect_child(nullptr,0,1),IMC_ERROR);int p[1][2]={{-1,-1}};
 EXPECT_EQ(pipe_connect_child(p,0,0),IMC_ERROR);pipe_close_all(p,1);pipe_close_all(nullptr,0);
}
