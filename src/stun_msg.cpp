#include <stdlib.h>
#include <stdio.h>

#include <arpa/inet.h>

#include <stdexcept>

#include "stun.hpp"
#include "serializer.hpp"

uint16_t stun_msg::max_msg_len = ~(uint16_t) 0;

stun_msg::stun_msg (stun_msg_type msg_type, uint64_t tid_hi, uint64_t tid_lo) 
  : type_(msg_type), len_(0), tid_hi_(tid_hi), tid_lo_(tid_lo), elems_()
{ }

stun_msg::stun_msg ()
  : type_(STUN_NULL_MSG_TYPE), len_(0), tid_hi_(0), tid_lo_(0), elems_()
{ }

// Get state.
uint32_t stun_msg::packet_length()
{
  return STUN_HEADER_LEN + length();
}

uint16_t stun_msg::length()
{
  return len_;
}
// End get.

// Pass a dynamically allocated stun_tlv object.
// Expected usage: msg.add_tlv(new mapped_address(...));
uint16_t stun_msg::add_tlv(stun_tlv *tlv)
{
  uint16_t tlv_size = TLV_HEADER_LEN + tlv->length();

  // Update msg header.
  len_ += tlv_size;

  // Transfer pointer ownership, 
  elems_.push_back(std::unique_ptr<stun_tlv>(tlv));

  return tlv_size;
}

uint64_t stun_msg::get_new_hi()
{
  static uint64_t next_hi = 0x1011011011011011;
  ++next_hi;
  return next_hi;
}

uint64_t stun_msg::get_new_lo()
{
  static uint64_t next_lo = 0xbadbadbadbadbadb;
  ++next_lo;
  return next_lo;
}

void stun_msg::serialize(serializer &s)
{
  if (STUN_HEADER_LEN + length() > s.remaining_space())
    {
      throw std::runtime_error("Not enough space in serializer.");
    }

  s.emplace_back(htons(type_));
  s.emplace_back(htons(len_));
  s.emplace_back(htonll(tid_hi_));
  s.emplace_back(htonll(tid_lo_));

  for (int i = 0; i < elems_.size(); ++i)
    {
      elems_[i]->serialize (s);
    }
}

void stun_msg::parse(serializer &s)
{
  type_ = (stun_msg_type) ntohs(s.explace_front<stun_msg_type>());
  uint16_t len = ntohs(s.explace_front<uint16_t>());
  tid_hi_ = ntohll(s.explace_front<uint64_t>());
  tid_lo_ = ntohll(s.explace_front<uint64_t>());

  len_ = 0;
  uint16_t cur = 0;

  while (cur != len) { cur += add_tlv(stun_tlv::parse(s)); }
}

void stun_msg::print_debug()
{
  printf("[stunmsg type: %x len: %u tidhi: %llx tidlo: %llx numtlvs: %lu ",
      type_, len_, tid_hi_, tid_lo_, elems_.size());
  for (int i = 0; i < elems_.size(); ++i)
    {
      printf(" [%d]: ", i);
      elems_[i]->print_debug();
    }
  printf("\n");
}
