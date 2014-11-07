/**
 *  @file
 *  @brief   Definition of pplme::Server, pplmed's server engine.
 *  @author  j.ho
 */
#ifndef PPLME_PPLMED_SERVER_H_
#define PPLME_PPLMED_SERVER_H_


#include "libpplmeutils/pimpl.h"


namespace pplme {


/**
 *  This class is the guy that fronts all the pplMe magic.  It is responsible
 *  for both populating the pplMe database and getting the networking bits up
 *  and running.  Basically, you just instantiate and Go()...
 */
class Server {
 public:
  /**
   *  @param  port is the TCP port to listen on for pplMe Requests.
   *  @param  test_db_size is the number of random entries that should be
   *          smashed into the pplMe test database.
   *  @param  max_age_difference is the maximum number of years difference in
   *          age for a person to be considered a match.
   */
  Server(int port, int test_db_size, int max_age_difference);
  ~Server();

  /** Go, pplMe, go! */
  bool Go();
  
 private:
  class Impl;
  utils::Pimpl<Impl> impl_;
};


}  // namespace pplme


#endif  // PPLME_PPLMED_SERVER_H_
