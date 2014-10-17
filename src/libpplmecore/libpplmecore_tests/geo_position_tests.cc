/**
 *  @file
 *  @brief   Tests for pplme::core::GeoPosition.
 *  @author  j.ho
 */


#include <ostream>
#include <gtest/gtest.h>
#include "libpplmecore/geo_position.h"


using pplme::core::GeoPosition;


TEST(GeoPositionTest, DefaultCtor)
{
  GeoPosition geopos;

  ASSERT_EQ(GeoPosition::DecimalLatitude{}, geopos.latitude());
  ASSERT_EQ(GeoPosition::DecimalLongitude{}, geopos.longitude());
}


TEST(GeoPositionTest, NonDefaultCtor)
{
  GeoPosition geopos{GeoPosition::DecimalLatitude{51.463376},
                     GeoPosition::DecimalLongitude{-0.168723}};
  
  ASSERT_EQ(GeoPosition::DecimalLatitude{51.463376}, geopos.latitude());
  ASSERT_EQ(GeoPosition::DecimalLongitude{-0.168723}, geopos.longitude());
}


namespace {

struct IsValidTestlette {
  float latitude;
  float longitude;
  bool is_valid;
}
  const is_valid_testlettes[] = {
    { 0, 0, true },
    { -90, 0, true },
    { 90, 0, true },
    { 0, -180, true },
    { 0, 180, true },
    { -90.0001, 0, false },
    { 90.0001, 0, false },
    { 0, -180.0001, false },
    { 0, 180.0001, false },
    { -90, -180, true },
    { 90, 180, true }
};

}

class GeoPositionTestIsValid :
      public testing::TestWithParam<IsValidTestlette> {};

TEST_P(GeoPositionTestIsValid, IsValid)
{
  GeoPosition geopos{GeoPosition::DecimalLatitude{GetParam().latitude},
                     GeoPosition::DecimalLongitude{GetParam().longitude}};

  ASSERT_EQ(GetParam().is_valid, geopos.IsValid());
}

INSTANTIATE_TEST_CASE_P(Default,
                        GeoPositionTestIsValid,
                        testing::ValuesIn(is_valid_testlettes));
