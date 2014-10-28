/**
 *  @file
 *  @brief   Tests for the pplme::proto::Date-based overloads of
 *           pplme::proto::Convert().
 *  @author  j.ho
 */


#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/optional.hpp>
#include "libpplmeproto/convert_date.h"
#include "libpplmeproto/date.pb.h"
#include "libpplmeutils/testlettes.h"


namespace proto = pplme::proto;
using pplme::proto::Convert;


PPLME_TESTLETTE_TYPE_BEGIN(DateRoundtripTestlette)
  uint16_t year;
  uint16_t month;
  uint16_t day;
PPLME_TESTLETTE_TYPE_END(DateRoundtripTestlette, ConvertDateTest_Roundtrip)

TEST_P(ConvertDateTest_Roundtrip, Tests) {
  // Arrange.
  boost::gregorian::date date_in{GetParam().year,
                                 GetParam().month,
                                 GetParam().day};

  // Act.
  proto::Date proto_date;
  Convert(date_in, &proto_date);
  boost::gregorian::date date_out;
  EXPECT_TRUE(Convert(proto_date, &date_out));

  // Assert.
  ASSERT_EQ(date_in, date_out);
}

PPLME_TESTLETTES_BEGIN(DateRoundtripTestlette, date_roundtrip_testlettes)
  PPLME_TESTLETTE(1400, 1, 1),
  // Boost.Date_Time docs say that the range is from 1400-01-01 to 9999-12-31;
  // however empirical testing says it's actually 1400-01-01 to 10000-12-31.
  PPLME_TESTLETTE(10000, 12, 31),
  PPLME_TESTLETTE(2014, 10, 27),
  // Happy Birthday, L.  <3
  PPLME_TESTLETTE(2015, 2, 16),
PPLME_TESTLETTES_END(date_roundtrip_testlettes, ConvertDateTest_Roundtrip)


PPLME_TESTLETTE_TYPE_BEGIN(InvalidProtoDateTestlette)
  boost::optional<uint32_t> ymd;
PPLME_TESTLETTE_TYPE_END(InvalidProtoDateTestlette,
                         ConvertDateTest_InvalidProtoDate)

TEST_P(ConvertDateTest_InvalidProtoDate, Tests) {
  // Arrange.
  proto::Date proto_date;
  if (GetParam().ymd)
    proto_date.set_ymd(*GetParam().ymd);

  // Act & Assert.
  boost::gregorian::date domain_date;
  ASSERT_FALSE(Convert(proto_date, &domain_date));
}

PPLME_TESTLETTES_BEGIN(InvalidProtoDateTestlette, invalid_proto_date_testlettes)
  PPLME_TESTLETTE(boost::none),
  PPLME_TESTLETTE(0U),
  PPLME_TESTLETTE(0xFFFFFFFF),
  // 1399-12-31:
  PPLME_TESTLETTE(0x1F0C7705),
  // 10001-01-01:
  PPLME_TESTLETTE(0x01011127),
  // 2014-09-31:
  PPLME_TESTLETTE(0x1F09DE07),
  // 2014-00-01:
  PPLME_TESTLETTE(0x0100DE07),
  // 2014-13-01:
  PPLME_TESTLETTE(0x010DDE07),
  // 2014-01-00:
  PPLME_TESTLETTE(0x0001DE07),
  // 2014-01-32:
  PPLME_TESTLETTE(0x2001DE07),
  // 2014-02-29:
  PPLME_TESTLETTE(0x1D02DE07)
PPLME_TESTLETTES_END(invalid_proto_date_testlettes,
                     ConvertDateTest_InvalidProtoDate)
