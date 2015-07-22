#include "packet.h"

#include "stg/sgcp_utils.h"

#include <ctime>

using STG::SGCP::Packet;

uint64_t Packet::MAGIC = 0x5f8edc0fdb6d3113; // Carefully picked random 64-bit number :)
uint16_t Packet::VERSION = 1;

Packet::Packet(uint16_t ver, uint16_t t, uint16_t sz)
    : magic(MAGIC),
      senderTime(time(NULL)),
      version(ver),
      type(t),
      size(s)
{
}

Packet hton(Packet value)
{
    value.magic = hton(value.magic);
    value.senderTime = hton(value.senderTime);
    value.version = hton(value.version);
    value.type = hton(value.type);
    value.size = hton(value.size);
}
