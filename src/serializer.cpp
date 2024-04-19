#include <assert.h>
#include <stdio.h>
#include <unistd.h>

#include "serializer.hpp"

serializer::serializer(uint64_t size)
  : space_(size), occupied_(0)
{
  serialized_ = new uint8_t[size];
  write_curs_ = serialized_;
  read_curs_ = serialized_;
}

serializer::~serializer ()
{
  delete[] serialized_;
}

void serializer::reset()
{
  read_curs_ = serialized_;
  write_curs_ = serialized_;
  space_ += occupied_;
}

void serializer::advance_write(uint64_t offset)
{
  write_curs_ += offset;
}

uint64_t serializer::remaining_space()
{
  return space_;
}

uint64_t serializer::buffer_len()
{
  return occupied_;
}

void *serializer::data()
{
  return (void *) serialized_;
}
