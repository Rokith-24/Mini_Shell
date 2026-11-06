#include "test_support.h"
TEST(Redirection, RedirectsOutput) {
 auto f=temp_name("redirect");pid_t child=fork();ASSERT_GE(child,0);
 if(child==0){command_stage_t s{};std::snprintf(s.output_file,sizeof(s.output_file),"%s",f.c_str());if(redirection_apply(&s)!=IMC_OK)_exit(2);puts("redirected");fflush(stdout);_exit(0);}
 int st=0;waitpid(child,&st,0);ASSERT_TRUE(WIFEXITED(st));EXPECT_EQ(WEXITSTATUS(st),0);
 std::ifstream in(f);std::string line;std::getline(in,line);EXPECT_EQ(line,"redirected");std::remove(f.c_str());
}
TEST(Redirection, InvalidAndMissingInput) { EXPECT_EQ(redirection_apply(nullptr),IMC_ERROR);command_stage_t s{};strcpy(s.input_file,"/tmp/imc_missing_input_zz");EXPECT_EQ(redirection_apply(&s),IMC_ERROR); }
