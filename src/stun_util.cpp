#include "stun.hpp"

// Stuntlv parents.

// stun_tlv.

stun_tlv::stun_tlv(stun_attribute_type type, uint16_t len)
  : type_(type), len_(len)
{ }

uint16_t stun_tlv::length()
{
  return len_;
}

stun_attribute_type stun_tlv::type()
{
  return type_;
}

void stun_tlv::serialize(serializer &s)
{
  if (TLV_HEADER_LEN + len_ > s.remaining_space())
    {
      throw std::runtime_error("Not enough space in serializer for this tlv.");
    }

  s.emplace_back(htons(type_));
  s.emplace_back(htons(len_));

  // Children responsible for serializing their values after this.
}

// Returns a dynamically allocated tlv object.
stun_tlv *stun_tlv::parse(serializer &s)
{
  stun_attribute_type type = (stun_attribute_type) ntohs(s.explace_front<stun_attribute_type>());
  uint16_t len = ntohs(s.explace_front<uint16_t>());

  if (type == MAPPED_ADDRESS || type == SOURCE_ADDRESS ||
      type == CHANGED_ADDRESS)
    {
      uint8_t null = s.explace_front<uint8_t>();
      if (null != 0) { throw std::runtime_error("Parsing malformed addr-based tlv."); }

      uint8_t family = s.explace_front<uint8_t>();
      if (family != STUN_INET) { throw std::runtime_error("Parsing malformed addr-based tlv."); }

      uint16_t port = ntohs(s.explace_front<uint16_t>());
      uint32_t addr = ntohl(s.explace_front<uint32_t>());

      return new address_tlv(type, addr, port);
    }
  else
    {
      // Unsupported tlv type.

      // Discard len bytes from the serializer.
      for (int i = 0; i < len; ++i) { (void) s.explace_front<uint8_t>(); }

      return new stun_tlv(type, len);
    }

}

void stun_tlv::print_debug()
{
  printf("type: %x len: %u", type_, len_);
}

// address_v.

address_v::address_v(uint32_t ipv4_addr, uint16_t port)
  : null_(0), family_(STUN_INET), ipv4_addr_(ipv4_addr), port_(port)
{ }

void address_v::serialize(serializer &s)
{
  if (ADDRESS_V_LEN > s.remaining_space())
    {
      throw std::runtime_error("Serializer to small to emplace into.\n");
    }

  s.emplace_back(null_);
  s.emplace_back(family_);
  s.emplace_back(htonl(ipv4_addr_));
  s.emplace_back(htons(port_));
}

void address_v::print_debug()
{
  printf(
      "null: %d family: %x ipv4-address: %u.%u.%u.%u port: %u",
      null_, 
      family_, 
      ((uint8_t *) &ipv4_addr_)[3],
      ((uint8_t *) &ipv4_addr_)[2],
      ((uint8_t *) &ipv4_addr_)[1],
      ((uint8_t *) &ipv4_addr_)[0],
      port_
  );
}

// address_tlv.

address_tlv::address_tlv(stun_attribute_type type, uint32_t ipv4_addr, uint16_t port)
  : stun_tlv(type, ADDRESS_V_LEN), address_v(ipv4_addr, port)
{ }

void address_tlv::serialize(serializer &s)
{
  stun_tlv::serialize(s);
  address_v::serialize(s);
}

void address_tlv::print_debug()
{
  stun_tlv::print_debug(); 
  printf(" ");
  address_v::print_debug();
}


