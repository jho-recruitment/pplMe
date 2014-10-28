/**
 *  @file
 *  @brief   Declaration of functionality for converting between domain and
 *           protocol GeoPosition types.
 *  @author  j.ho
 */
#ifndef PPLME_LIBPPLMEPROTO_CONVERTGEOPOSITION_H_
#define PPLME_LIBPPLMEPROTO_CONVERTGEOPOSITION_H_


namespace pplme {
namespace core {
class GeoPosition;
}
namespace proto {
class GeoPosition;
}
}


namespace pplme {
namespace proto {


void Convert(core::GeoPosition const& from, GeoPosition* to);
bool Convert(GeoPosition const& from, core::GeoPosition* to);


}  // namespace proto
}  // namespace pplme


#endif  // PPLME_LIBPPLMEPROTO_CONVERTGEOPOSITION_H_
