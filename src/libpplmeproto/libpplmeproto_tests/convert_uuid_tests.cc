/**
 *  @file
 *  @brief   Tests for the pplme::proto::Uuid-based overloads of
 *           pplme::proto::Convert().
 *  @author  j.ho
 */


#include <boost/optional.hpp>
#include <boost/uuid/string_generator.hpp>
#include "libpplmeproto/convert_uuid.h"
#include "libpplmeproto/uuid.pb.h"
#include "libpplmeutils/testlettes.h"


namespace proto = pplme::proto;
using pplme::proto::Convert;


PPLME_TESTLETTE_TYPE_BEGIN(UuidRoundtripTestlette)
  char const* uuid;
PPLME_TESTLETTE_TYPE_END(UuidRoundtripTestlette, ConvertUuidTest_Roundtrip)

TEST_P(ConvertUuidTest_Roundtrip, Tests) {
  // Arrange.
  auto uuid_in = boost::uuids::string_generator()(GetParam().uuid);

  // Act.
  proto::Uuid proto_uuid;
  Convert(uuid_in, &proto_uuid);
  boost::uuids::uuid uuid_out;
  EXPECT_TRUE(Convert(proto_uuid, &uuid_out));
  
  // Assert.
  ASSERT_EQ(uuid_in, uuid_out);
}

PPLME_TESTLETTES_BEGIN(UuidRoundtripTestlette, uuid_roundtrip_testlettes)
  PPLME_TESTLETTE("00000000-0000-0000-0000-000000000000"),
  PPLME_TESTLETTE("962dc9b7-1279-4aa2-bdef-d7fe53f7336b")
PPLME_TESTLETTES_END(uuid_roundtrip_testlettes, ConvertUuidTest_Roundtrip)


PPLME_TESTLETTE_TYPE_BEGIN(InvalidProtoUuidTestlette)
  boost::optional<std::vector<uint8_t>> octets;
PPLME_TESTLETTE_TYPE_END(InvalidProtoUuidTestlette,
                         ConvertUuidTest_InvalidProtoUuid)

TEST_P(ConvertUuidTest_InvalidProtoUuid, Tests) {
  // Arrange.
  proto::Uuid proto_uuid;
  if (GetParam().octets)
    proto_uuid.set_octets(GetParam().octets->data(), GetParam().octets->size());

  // Act & Assert.
  boost::uuids::uuid domain_uuid;
  ASSERT_FALSE(Convert(proto_uuid, &domain_uuid));
}

PPLME_TESTLETTES_BEGIN(InvalidProtoUuidTestlette, invalid_proto_uuid_testlettes)
  PPLME_TESTLETTE(boost::none),
  PPLME_TESTLETTE(std::vector<uint8_t>({})),
  PPLME_TESTLETTE(std::vector<uint8_t>({0})),
  PPLME_TESTLETTE(std::vector<uint8_t>({0, 0})),
  PPLME_TESTLETTE(std::vector<uint8_t>({0, 0, 0})),
  PPLME_TESTLETTE(std::vector<uint8_t>({0, 0, 0, 0})),
  PPLME_TESTLETTE(std::vector<uint8_t>({0, 0, 0, 0, 0})),
  PPLME_TESTLETTE(std::vector<uint8_t>({0, 0, 0, 0, 0, 0})),
  PPLME_TESTLETTE(std::vector<uint8_t>({0, 0, 0, 0, 0, 0, 0})),
  PPLME_TESTLETTE(std::vector<uint8_t>({0, 0, 0, 0, 0, 0, 0, 0})),
  PPLME_TESTLETTE(std::vector<uint8_t>({0, 0, 0, 0, 0, 0, 0, 0, 0})),
  PPLME_TESTLETTE(std::vector<uint8_t>({0, 0, 0, 0, 0, 0, 0, 0, 0, 0})),
  PPLME_TESTLETTE(std::vector<uint8_t>({0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0})),
  PPLME_TESTLETTE(std::vector<uint8_t>({0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0})),
  PPLME_TESTLETTE(std::vector<uint8_t>(
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0})),
  PPLME_TESTLETTE(std::vector<uint8_t>(
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0})),
  PPLME_TESTLETTE(std::vector<uint8_t>(
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0})),
PPLME_TESTLETTES_END(invalid_proto_uuid_testlettes,
                     ConvertUuidTest_InvalidProtoUuid)
