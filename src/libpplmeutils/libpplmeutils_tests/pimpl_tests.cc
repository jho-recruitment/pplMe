/**
 *  @file
 *  @brief   Tests for pplme::utils::Pimpl.
 *  @author  j.ho
 */


#include <gtest/gtest.h>
#include "libpplmeutils/pimpl.h"


using pplme::utils::Pimpl;


namespace
{
  class DtorTallier
  {
   public:
    DtorTallier(int * tally) : tally_{tally} {}
    ~DtorTallier() { ++*tally_; }

   private:
    int* tally_;
  };
}


TEST(PimplTest, Dtor)
{
  int dtor_tally = 0;
  {
    Pimpl<DtorTallier> dtor_tallier_impl{new DtorTallier{&dtor_tally}};

    ASSERT_EQ(0, dtor_tally);
  }
  ASSERT_EQ(1, dtor_tally);
}
