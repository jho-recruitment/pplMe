/**
 *  @file
 *  @brief   Tests for the pplme::proto::Person-based overloads of
 *           pplme::proto::Convert().
 *  @author  j.ho
 */


#include <boost/optional.hpp>
#include <boost/uuid/string_generator.hpp>
#include "libpplmecore/person.h"
#include "libpplmeproto/convert_person.h"
#include "libpplmeproto/person.pb.h"
#include "libpplmeutils/testlettes.h"


namespace core = pplme::core;
namespace proto = pplme::proto;
using pplme::proto::Convert;


namespace {

core::Person CreateTokenPerson() {
  core::PersonId const kTokenPersonId{boost::uuids::string_generator()(
      "323e7b4a-4e19-4064-a586-9b9d97a0a32d")};
  core::GeoPosition const kTokenPos{core::GeoPosition::DecimalLatitude{65.4},
                                    core::GeoPosition::DecimalLongitude{-10.9}};
  return core::Person{kTokenPersonId,
                      "Baberaham Lincoln",
                      boost::gregorian::date{2014, 10, 28},
                      kTokenPos};
}

}  // namespace


TEST(ConvertPersonTest, Roundtrip) {
  // Arrange.
  auto person_in = CreateTokenPerson();

  // Act.
  proto::Person proto_person;
  Convert(person_in, &proto_person);
  core::Person person_out;
  EXPECT_TRUE(Convert(proto_person, &person_out));
  
  // Assert.
  ASSERT_EQ(person_in.id(), person_out.id());
  ASSERT_EQ(person_in.name(), person_out.name());
  ASSERT_EQ(person_in.date_of_birth(), person_out.date_of_birth());
  ASSERT_EQ(person_in.location_of_home().latitude(),
            person_out.location_of_home().latitude());
  ASSERT_EQ(person_in.location_of_home().longitude(),
            person_out.location_of_home().longitude());
}


namespace {

proto::Person CreateTokenValidProtoPerson() {
  proto::Person proto_person;
  Convert(CreateTokenPerson(), &proto_person);
  return proto_person;
}

}  // namespace


TEST(ConvertPersonTest, ConvertFailsForProtoPersonWithoutId) {
  // Arrange.
  auto proto_person = CreateTokenValidProtoPerson();
  proto_person.clear_id();

  // Act & Assert.
  core::Person domain_person;
  ASSERT_FALSE(Convert(proto_person, &domain_person));
}


TEST(ConvertPersonTest, ConvertFailsForProtoPersonWithoutName) {
  // Arrange.
  auto proto_person = CreateTokenValidProtoPerson();
  proto_person.clear_name();

  // Act & Assert.
  core::Person domain_person;
  ASSERT_FALSE(Convert(proto_person, &domain_person));
}


TEST(ConvertPersonTest, ConvertFailsForProtoPersonWithoutDateOfBirth) {
  // Arrange.
  auto proto_person = CreateTokenValidProtoPerson();
  proto_person.clear_date_of_birth();

  // Act & Assert.
  core::Person domain_person;
  ASSERT_FALSE(Convert(proto_person, &domain_person));
}


TEST(ConvertPersonTest, ConvertFailsForProtoPersonWithoutLocationOfHome) {
  // Arrange.
  auto proto_person = CreateTokenValidProtoPerson();
  proto_person.clear_location_of_home();

  // Act & Assert.
  core::Person domain_person;
  ASSERT_FALSE(Convert(proto_person, &domain_person));
}
