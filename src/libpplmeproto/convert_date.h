/**
 *  @file
 *  @brief   Declaration of functionality for converting between domain and
 *           protocol date types.
 *  @author  j.ho
 */
#ifndef PPLME_LIBPPLMEPROTO_CONVERTDATE_H_
#define PPLME_LIBPPLMEPROTO_CONVERTDATE_H_


namespace boost {
namespace gregorian {
class date;
}
}
namespace pplme {
namespace proto {
class Date;
}
}


namespace pplme {
namespace proto {


void Convert(boost::gregorian::date const& from, Date* to);
bool Convert(Date const& from, boost::gregorian::date* to);


}  // namespace proto
}  // namespace pplme


#endif  // PPLME_LIBPPLMEPROTO_CONVERTDATE_H_
