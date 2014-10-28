/**
 *  @file
 *  @brief   Tests for the pplme::proto::GeoPosition-based overloads of
 *           pplme::proto::Convert().
 *  @author  j.ho
 */


#include <boost/optional.hpp>
#include "libpplmecore/geo_position.h"
#include "libpplmeproto/convert_geo_position.h"
#include "libpplmeproto/geo_position.pb.h"
#include "libpplmeutils/testlettes.h"


namespace core = pplme::core;
namespace proto = pplme::proto;
using pplme::proto::Convert;


PPLME_TESTLETTE_TYPE_BEGIN(GeoPositionRoundtripTestlette)
  float latitude;
  float longitude;
PPLME_TESTLETTE_TYPE_END(GeoPositionRoundtripTestlette,
                         ConvertGeoPositionTest_Roundtrip)

TEST_P(ConvertGeoPositionTest_Roundtrip, Tests) {
  // Arrange.
  core::GeoPosition geopos_in{
      core::GeoPosition::DecimalLatitude{GetParam().latitude},
      core::GeoPosition::DecimalLongitude{GetParam().longitude}};

  // Act.
  proto::GeoPosition proto_geopos;
  Convert(geopos_in, &proto_geopos);
  core::GeoPosition geopos_out;
  EXPECT_TRUE(Convert(proto_geopos, &geopos_out));
  
  // Assert.
  ASSERT_EQ(geopos_in.latitude(), geopos_out.latitude());
  ASSERT_EQ(geopos_in.longitude(), geopos_out.longitude());
}

PPLME_TESTLETTES_BEGIN(GeoPositionRoundtripTestlette,
                       geo_position_roundtrip_testlettes)
  PPLME_TESTLETTE(0, 0),
  PPLME_TESTLETTE(-90, 0),
  PPLME_TESTLETTE(90, 0),
  PPLME_TESTLETTE(0, -180),
  PPLME_TESTLETTE(0, 180),
  PPLME_TESTLETTE(12.34, -56.789)
PPLME_TESTLETTES_END(geo_position_roundtrip_testlettes,
                     ConvertGeoPositionTest_Roundtrip)


PPLME_TESTLETTE_TYPE_BEGIN(InvalidProtoGeoPositionTestlette)
  boost::optional<float> latitude;
  boost::optional<float> longitude;
PPLME_TESTLETTE_TYPE_END(InvalidProtoGeoPositionTestlette,
                         ConvertGeoPositionTest_InvalidProtoGeoPosition)

TEST_P(ConvertGeoPositionTest_InvalidProtoGeoPosition, Tests) {
  // Arrange.
  proto::GeoPosition proto_geopos;
  if (GetParam().latitude)
    proto_geopos.set_latitude(*GetParam().latitude);
  if (GetParam().longitude)
    proto_geopos.set_longitude(*GetParam().longitude);

  // Act & Assert.
  core::GeoPosition domain_geopos;
  ASSERT_FALSE(Convert(proto_geopos, &domain_geopos));
}

PPLME_TESTLETTES_BEGIN(InvalidProtoGeoPositionTestlette, invalid_proto_geo_position_testlettes)
  PPLME_TESTLETTE(boost::none, boost::none),
  PPLME_TESTLETTE(0.0f, boost::none),
  PPLME_TESTLETTE(boost::none, 0.0f),
PPLME_TESTLETTES_END(invalid_proto_geo_position_testlettes,
                     ConvertGeoPositionTest_InvalidProtoGeoPosition)
