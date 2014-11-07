/**
 *  @file
 *  @brief   Declaration of the pplme::Pplme(), the client-side function that
 *           requests people information from a pplMe server and displays the
 *           results.
 *  @author  j.ho
 */
#ifndef PPLME_PPLMEC_PPLME_H_
#define PPLME_PPLMEC_PPLME_H_


#include <string>


namespace pplme {


/**
 *  This function is basically the essence of pplmec.  It is responsible for
 *  connecting to the server, constructing a PplmeRequest message, decoding
 *  the resultant PplmeResponse message, and displaying the results.
 *
 *  @param  pplme_server_address is the domainname/hostname/ip of the machine
 *          running the pplMe server.
 *  @param  pplme_server_port is the port on which the pplMe server is
 *          listening.
 *  @param  users_latitude is the WGS 84 decimal latitude of the requesting
 *          user.
 *  @param  users_longitude is the WGS 84 decimal longitude of the requesting
 *          user.
 *
 *  @return  true iff the request was successful (which includes the case of
 *           receiving an empty result set).
 */
bool Pplme(
    std::string const& pplme_server_address,
    unsigned short pplme_server_port,
    float users_latitude,
    float users_longitude,
    int users_age);


}  // namespace pplme


#endif  // PPLME_PPLMEC_PPLME_H_
