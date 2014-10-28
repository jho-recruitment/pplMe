/**
 *  @file
 *  @brief   Declaration of functionality for converting between domain and
 *           protocol Person types.
 *  @author  j.ho
 */
#ifndef PPLME_LIBPPLMEPROTO_CONVERTPERSON_H_
#define PPLME_LIBPPLMEPROTO_CONVERTPERSON_H_


namespace pplme {
namespace core {
class Person;
}
namespace proto {
class Person;
}
}


namespace pplme {
namespace proto {


void Convert(core::Person const& from, Person* to);
bool Convert(Person const& from, core::Person* to);


}  // namespace proto
}  // namespace pplme


#endif  // PPLME_LIBPPLMEPROTO_CONVERTPERSON_H_
