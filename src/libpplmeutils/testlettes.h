/**
 *  @file
 *  @brief   Helpers for writing _testlette_-esque test cases.
 *  @author  j.ho
 */


#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/variadic/to_seq.hpp>
#include <gtest/gtest.h>


/**
 *  @file
 *
 *  @details
 *  A testlette is basically a specific input to a parameterized test case,
 *  and the macros provided here slightly improve on the Google Test story
 *  by reducing the amount of boilerplate required, and improving the diag-
 *  nostics when a test fails.
 *
 *  Example:
 *  @code
 *  class Result
 *  {
 *   public:
 *    explicit Result(bool ok) : ok_{ok} {}
 *  
 *    bool IsOk() const { return ok_; }
 *    
 *   private:
 *   bool ok_;
 *  };
 *      
 *  PPLME_TESTLETTE_TYPE_BEGIN(IsOkTestlette)
 *    bool ok;
 *  PPLME_TESTLETTE_TYPE_END(IsOkTestlette, ResultTest_IsOk)
 *  
 *  TEST_P(ResultTest_IsOk, Tests)
 *  {
 *    Result result{GetParam().ok};
 *    ASSERT_EQ(GetParam().ok, result.IsOk());
 *  }
 *  
 *  PPLME_TESTLETTES_BEGIN(IsOkTestlette, is_ok_testlettes)
 *    PPLME_TESTLETTE(false),
 *    PPLME_TESTLETTE(true)
 *  PPLME_TESTLETTES_END(is_ok_testlettes, ResultTest_IsOk)
 *  @endcode
 *
 *  @{
 */
#define PPLME_TESTLETTE_TYPE_BEGIN(testlette_type)  \
struct testlette_type {
#define PPLME_TESTLETTE_TYPE_END(testlette_type, test_case_name)  \
  std::string const testlette;                                           \
};  \
\
std::ostream& operator<<(std::ostream& stream,  \
                         testlette_type const& testlette)  \
{  \
  return stream << "Testlette: " << testlette.testlette;  \
}  \
\
class test_case_name : public testing::TestWithParam<testlette_type> {};


#define PPLME_TESTLETTES_BEGIN(testlette_type, testlettes_var)  \
testlette_type const testlettes_var[] = {
#define PPLME_DETAIL_RENDER_ARG(r, junk, arg) BOOST_PP_STRINGIZE(arg) ", "
#define PPLME_TESTLETTE(...)  \
  { __VA_ARGS__,  \
    "{ "  \
    BOOST_PP_SEQ_FOR_EACH(PPLME_DETAIL_RENDER_ARG,  \
                          junk,  \
                          BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))  \
    "}" }
#define PPLME_TESTLETTES_END(testlettes_var, test_case_name)  \
};  \
\
INSTANTIATE_TEST_CASE_P(testlettes_var,  \
                        test_case_name,  \
                        testing::ValuesIn(testlettes_var));
/** @} */
