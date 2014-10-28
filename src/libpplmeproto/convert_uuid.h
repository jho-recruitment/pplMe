/**
 *  @file
 *  @brief   Declaration of functionality for converting between domain and
 *           protocol UUID types.
 *  @author  j.ho
 */
#ifndef PPLME_LIBPPLMEPROTO_CONVERTUUID_H_
#define PPLME_LIBPPLMEPROTO_CONVERTUUID_H_


namespace boost {
namespace uuids {
class uuid;
}
}
namespace pplme {
namespace proto {
class Uuid;
}
}


namespace pplme {
namespace proto {


void Convert(boost::uuids::uuid const& from, Uuid* to);
bool Convert(Uuid const& from, boost::uuids::uuid* to);


}  // namespace proto
}  // namespace pplme


#endif  // PPLME_LIBPPLMEPROTO_CONVERTUUID_H_
