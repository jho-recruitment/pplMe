/**
 *  @file
 *  @brief   Definition of functionality for converting between domain and
 *           proto date types.
 *  @author  j.ho
 */


#include <arpa/inet.h>
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <glog/logging.h>
#include "convert_date.h"
#include "date.pb.h"


namespace pplme {
namespace proto {


void Convert(boost::gregorian::date const& from, Date* to) {
  CHECK_NOTNULL(to);
  
  uint32_t ymd = from.year();
  ymd <<= 16;
  ymd += static_cast<uint32_t>(from.month()) << 8;
  ymd += from.day();

  to->set_ymd(htonl(ymd));
}


bool Convert(Date const& from, boost::gregorian::date* to) {
  CHECK_NOTNULL(to);
  
  if (!from.has_ymd()) {
    DLOG(ERROR) << "Date does not have ymd";
    return false;
  }

  uint32_t ymd = ntohl(from.ymd());

  uint32_t year = ymd >> 16;
  uint32_t month = (ymd & 0xFF00) >> 8;
  uint32_t day = ymd & 0xFF;
  
  try {
    *to = boost::gregorian::date(year, month, day);
    return true;
  }
  catch (std::out_of_range const&) {
    DLOG(ERROR) << "Invalid Date: " << year << "-" << month << "-" << day;
    return false;
  }
}


}  // namespace proto
}  // namespace pplme
