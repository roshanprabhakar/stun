#ifndef __STUN_APPLIB__
#define __STUN_APPLIB__

#include <stdint.h>

#include <vector>
#include <memory>

#include "serializer.hpp"

#define STUN_INET 0x01

enum stun_msg_type : uint16_t
{
  STUN_NULL_MSG_TYPE =            0x0000,
  BINDING_REQUEST =               0x0001,
  BINDING_RESPONSE =              0x0101,
  BINDING_ERROR_RESPONSE =        0x0111,
  SHARED_SECRET_REQUEST =         0x0002,
  SHARED_SECRET_RESPONSE =        0x0102,
  SHARED_SECRET_ERROR_RESPONSE =  0x0112
};

enum stun_attribute_type : uint16_t
{
  STUN_NULL_ATTRIBUTE =           0x0000,
  MAPPED_ADDRESS =                0x0001,
  RESPONSE_ADDRESS =              0x0002,
  CHANGE_REQUEST =                0x0003,
  SOURCE_ADDRESS =                0x0004,
  CHANGED_ADDRESS =               0x0005,
  USERNAME =                      0x0006,
  PASSWORD =                      0x0007,
  MESSAGE_INTEGRITY =             0x0008,
  ERROR_CODE =                    0x0009,
  UNKNOWN_ATTRIBUTES =            0x000a,
  REFLECTED_FROM =                0x000b
};

#define TLV_HEADER_LEN 4 
class stun_tlv
{
  public:
    stun_tlv(stun_attribute_type type, uint16_t len);

    uint16_t length(void);
    stun_attribute_type type(void);

    // Child object must serialize itself into the given 
    // serializer, after calling this function.
    virtual void serialize(serializer &);

    // Generate a stuntlv object from the front of the given
    // serializer.
    static stun_tlv *parse(serializer &);

    // Dump tlv state to stdout.
    virtual void print_debug(void);

  protected:
    stun_attribute_type type_;
    uint16_t len_;
};

#define ADDRESS_V_LEN 8
class address_v
{
  public:
    address_v(uint32_t, uint16_t);

    // Children tlv of this base class can properly serialize
    // themselves by just calling this function.
    void serialize(serializer &);
    void print_debug(void);

  protected:
    uint8_t null_;
    uint8_t family_;
    uint32_t ipv4_addr_;
    uint16_t port_;
};

#define ADDRESS_TLV_LEN (TLV_HEADER_LEN + ADDRESS_V_LEN)
class address_tlv : public stun_tlv, public address_v
{
  public:
    address_tlv(stun_attribute_type, uint32_t, uint16_t);

    void serialize(serializer &) override;
    void print_debug(void) override;
};

class mapped_address : public address_tlv
{
  // The ipv4_addr_,port_ attributes represent the ipv4,port of the 
  // client as perceived by the stun server. Can be used to determine
  // if a NAT is deployed between the client and the stun.

  public:
    mapped_address(uint32_t, uint16_t);
};

class source_address : public address_tlv
{
  // The ipv4_addr_,port_ attributes represent the ipv4,port of the
  // stun server that are being used as a source for the response
  // message.

  public:
    source_address(uint32_t, uint16_t);
};

class changed_address : public address_tlv
{
  // The ipv4_addr_,port_ attributes represent the ipv4,port of the
  // alternative internet location of the stun server. If the request
  // contained a CHANGE_REQUEST tlv with the 'change ip' and 'change 
  // port' flags set, the server would reply from this address.

  public:
    changed_address(uint32_t, uint16_t);
};

#define CHANGE_REQUEST_V_LEN (sizeof (uint32_t))
class change_request : public stun_tlv
{
  private:
    uint32_t flags_;

  public:
    change_request(bool change_ip, bool change_port);

    void serialize(serializer &s) override;
    void print_debug(void) override;
};

#define STUN_HEADER_LEN 20
class stun_msg
{
  public:
    static uint16_t max_msg_len;

    stun_msg(stun_msg_type, uint64_t, uint64_t);
    stun_msg(void);

    // Add an attribute to this message. Returns the 
    // size of the tlv added, including the header.
    uint16_t add_tlv(stun_tlv *);

    // Get size of payload.
    uint32_t packet_length(void);

    // Get size of entire stun packet.
    uint16_t length(void);

    static uint64_t get_new_hi(void);
    static uint64_t get_new_lo(void);

    // Serialize this packet into the given buffer.
    void serialize(serializer &);
    
    // Parse a message from a serializer.
    void parse(serializer &);

    // Dump this message to stdout.
    void print_debug(void);

  private:
    stun_msg_type type_;
    uint16_t len_;
    uint64_t tid_hi_;
    uint64_t tid_lo_;
    std::vector<std::unique_ptr<stun_tlv>> elems_;
};

#endif // __STUN_APPLIB__
