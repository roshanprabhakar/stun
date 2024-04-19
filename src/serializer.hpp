#ifndef __SERIALIZER__
#define __SERIALIZER__

#include <stdbool.h>
#include <stdint.h>

#include <stdexcept>
#include <stdio.h>

class serializer
{
  public:
    serializer(uint64_t);
    ~serializer(void);

    template<typename T>
    void emplace_back(T value)
    {
      if (space_ < sizeof(T))
        {
          throw std::runtime_error("Serializer emplace failed due to lack of space.\n");
        }

      *(T *)write_curs_ = value;
      write_curs_ = (uint8_t *) ((T *) write_curs_ + 1);
      space_ -= sizeof(T);
      occupied_ += sizeof(T);
    }

    template<typename T>
    T explace_front(void)
    {
      if (write_curs_ - read_curs_ < sizeof(T))
        {
          throw std::runtime_error("Not enough bytes in serializer to pop.\n");
        }
      
      T ret = *(T *)read_curs_;
      read_curs_ = (uint8_t *) ((T *) read_curs_ + 1);
      return ret;
    }

    // Reset internal state.
    void reset(void);

    void advance_write(unsigned long long);

    uint64_t remaining_space(void);
    uint64_t buffer_len(void);
    void *data(void);

  private:
    uint8_t *serialized_;
    uint8_t *write_curs_;
    uint8_t *read_curs_;
    uint64_t space_;
    uint64_t occupied_;
};

#endif // __SERIALIZER__
