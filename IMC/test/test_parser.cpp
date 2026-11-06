#include "test_support.h"
TEST(Parser, ParsesWordsQuotesAndEnvironment) {
 setenv("IMC_GTEST_VAR","expanded",1); char s[]="echo 'hello world' $IMC_GTEST_VAR";
 parsed_line_t p{}; char e[256]{}; ASSERT_EQ(parse_line(s,&p,e,sizeof(e)),IMC_OK);
 ASSERT_EQ(p.commands[0].stages[0].argc,3U); EXPECT_STREQ(p.commands[0].stages[0].argv[1],"hello world");
 EXPECT_STREQ(p.commands[0].stages[0].argv[2],"expanded"); free_parsed(&p);
}
TEST(Parser, ParsesPipelineRedirectionAppendBackgroundAndSemicolon) {
 char s[]="cat < in | grep x > out; echo ok >> log &"; parsed_line_t p{}; char e[256]{};
 ASSERT_EQ(parse_line(s,&p,e,sizeof(e)),IMC_OK); ASSERT_EQ(p.command_count,2U);
 EXPECT_EQ(p.commands[0].stage_count,2U); EXPECT_STREQ(p.commands[0].stages[0].input_file,"in");
 EXPECT_STREQ(p.commands[0].stages[1].output_file,"out"); EXPECT_TRUE(p.commands[1].background);
 EXPECT_TRUE(p.commands[1].stages[0].append_output); free_parsed(&p);
}
TEST(Parser, RejectsMalformedInput) {
 parsed_line_t p{}; char e[256]{}; char a[]="echo 'bad"; EXPECT_EQ(parse_line(a,&p,e,sizeof(e)),IMC_PARSE_ERROR);
 char b[]="| echo"; EXPECT_EQ(parse_line(b,&p,e,sizeof(e)),IMC_PARSE_ERROR);
 char c[]="echo >"; EXPECT_EQ(parse_line(c,&p,e,sizeof(e)),IMC_PARSE_ERROR);
 char d[]=""; EXPECT_EQ(parse_line(d,&p,e,sizeof(e)),IMC_OK);
 EXPECT_EQ(parse_line(nullptr,&p,e,sizeof(e)),IMC_ERROR);
}
