/**
 *  @file
 *  @brief   A basic encapsulation of an arbitrary message that may be
 *           transferred from client to server, and vice-versa.
 *  @author  j.ho
 */
#ifndef PPLME_LIBPPLMENET_MESSAGE_H_
#define PPLME_LIBPPLMENET_MESSAGE_H_


#include <limits.h>
#include <arpa/inet.h>
#include <memory>


namespace pplme {
namespace net {


/**
 *  @remarks
 *  The approach taken here is to have a very basic message framing mechanism,
 *  with the application level being responsible for assuming and/or
 *  determining the body semantics.  Furthermore, the Header is intentionally
 *  directly serializable, yet access to its contents are encapsulated so that
 *  if we ever want to put some sort of message "flags" within, we can do so
 *  fairly transparently (my thinking is that we may even be able to do that
 *  in a backwardly-compatible (ignoring a conceptual initial client <-> server
 *  handshake) since the current max body length gives us bits to play with.
 */
class Message {
 public:
  /**
   *  @note
   *  This type currently needs to be laid out in memory as it would be on
   *  the wire.
   */
  class __attribute__ ((packed)) Header final {
   public:
    explicit Header(uint32_t body_length = 0) :
        body_length_{htonl(body_length)} {}

    /** @returns  The length of the message body (in host order). */
    uint32_t GetBodyLength() const { return ntohl(body_length_); }
    
   private:
    /** @note  Stored in network order. */
    uint32_t body_length_;
  };
  /** Sanity check that our Header type is good for the wire. */
  static_assert(CHAR_BIT == 8 && sizeof(Header) == 4,
                "Yeah.  Good luck with that.  &;p");

  /**
   *  @remarks
   *  Having this limit is currently quite important as it means we can have
   *  a simple approach to reading in messages without needing to worry about
   *  malicious peers spoofing large messages and DoSing us as a result of us
   *  blindly trying to allocate that amount of memory.
   */
  static uint32_t const kMaxBodyLength = 1048576;

  Message(Header header, std::unique_ptr<uint8_t[]> body) :
      header_{header},
      body_{std::move(body)} {}
  Message(std::unique_ptr<uint8_t[]> body, uint32_t body_length) :
      Message{Header{body_length}, std::move(body)} {}

  /**
   *  Deleted because we don't want accidental copying (read as: have no need
   *  for being able to copy right now).
   *  @{
   */
  Message(Message const&) = delete;
  Message& operator=(Message const&) = delete;
  /** @} */

  /** Simple buffer factory function, mostly because we don't have
      std::make_unique yet. */
  static std::unique_ptr<uint8_t[]> CreateBodyBuffer(uint32_t body_length) {
    return std::unique_ptr<uint8_t[]>{new uint8_t[body_length]};
  }

  Header const& GetHeader() const { return header_; }
  void const* GetBodyOctets() const { return body_.get(); }
  uint32_t GetBodyLength() const { return header_.GetBodyLength(); }
  
 private:
  Header header_;
  std::unique_ptr<uint8_t[]> body_;
};


}  // namespace net
}  // namespace pplme


#endif  // PPLME_LIBPPLMENET_MESSAGE_H_
