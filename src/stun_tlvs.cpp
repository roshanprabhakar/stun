#include "stun.hpp"

#include <stdio.h>

// Tlvs that appear in BINDING_RESPONSE messages.

// MAPPED_ADDRESS
mapped_address::mapped_address(uint32_t ipv4_addr, uint16_t port)
  : address_tlv(MAPPED_ADDRESS, ipv4_addr, port)
{ }

// SOURCE_ADDRESS
source_address::source_address(uint32_t ipv4_addr, uint16_t port)
  : address_tlv(SOURCE_ADDRESS, ipv4_addr, port)
{ }

// CHANGED_ADDRESS
changed_address::changed_address(uint32_t ipv4_addr, uint16_t port)
  : address_tlv(CHANGED_ADDRESS, ipv4_addr, port)
{ }

// Tlvs that appear in BINDING_REQUEST messages.

#define CHANGE_IP_BIT (1 << 29)
#define CHANGE_PORT_BIT (1 << 30)

change_request::change_request(bool change_ip, bool change_port)
  : stun_tlv(CHANGE_REQUEST, CHANGE_REQUEST_V_LEN), flags_(0)
{
  if (change_ip) { flags_ |= CHANGE_IP_BIT; }
  if (change_port) { flags_ |= CHANGE_PORT_BIT; }
}

void change_request::print_debug()
{
  printf(
      "change_ip: %d change_port: %d",
      flags_ & CHANGE_IP_BIT,
      flags_ & CHANGE_PORT_BIT
  );
}

void change_request::serialize(serializer &s)
{
  stun_tlv::serialize(s);

  if (s.remaining_space() < CHANGE_REQUEST_V_LEN)
    {
      throw std::runtime_error("Serialize too small to emplace into.");
    }

  s.emplace_back(flags_);
}
