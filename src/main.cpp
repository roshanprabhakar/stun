/* Communicating with a STUN server to discover host public IP address.
 */

#include <stdint.h>
#include <sys/types.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <unistd.h>

// #include <openssl/ssl.h>
// #include <openssl/tls1.h>

#include <stdio.h>
#include <string.h>

#include <assert.h>

#include "stun.hpp"
#include "serializer.hpp"

// Public STUN server information.
// const char *STUN_HOSTNAME = "173.194.219.127";
const char *STUN_HOSTNAME = "stun.l.google.com";
const uint16_t STUN_PORT = htons (19302);

const uint16_t DEFAULT_LOCAL_PORT = 1440;

// NOTE saddr fields in network byte order already.
int get_stun_addr(struct sockaddr_in *s_addr)
{
  struct hostent *ent = gethostbyname(STUN_HOSTNAME);
  if (ent == NULL) { return -1; }

  assert(ent->h_addrtype == PF_INET);
  s_addr->sin_family = PF_INET;
  s_addr->sin_port = STUN_PORT;
  s_addr->sin_addr.s_addr = *(uint32_t *)ent->h_addr;
  memset(&s_addr->sin_zero, 0, sizeof (s_addr->sin_zero));
  return 0;
}

// Get a socket 
int init(struct sockaddr_in *stun_target)
{
  int cfd = socket(PF_INET, SOCK_DGRAM, 0);
  if (cfd == -1) { throw std::runtime_error("Unable to open local socket.\n"); }

  // App-level bind to the provided port, at all local network interfaces.
  struct sockaddr_in local_addr = 
    {
      .sin_len =    sizeof(struct sockaddr_in),
      .sin_family = PF_INET,
      .sin_port =   htons(DEFAULT_LOCAL_PORT)
    };
  local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  memset(&local_addr.sin_zero, 0, sizeof (local_addr.sin_zero));

  if (bind(cfd, (struct sockaddr *) &local_addr, sizeof(struct sockaddr)) == -1)
    {
      close(cfd);
      throw std::runtime_error("Failed to bind socket to local address and port.\n");
    }

  // Get target stun IP address.
  if (get_stun_addr(stun_target) == -1)
    {
      close(cfd);
      throw std::runtime_error("Unable to retrieve stun server address.\n");
    }

  return cfd;
}

int main()
{
  // Returns a local bound socket, populates stuntarget with the address of a 
  // STUN server.
  struct sockaddr_in stun_target;
  int fd = init(&stun_target);

  /*
  stun_msg msg;
  msg.add_tlv(new stun_tlv(STUN_NULL_ATTRIBUTE, 0));
  msg.add_tlv(new address_tlv(MAPPED_ADDRESS, 0x1, 2));
  msg.add_tlv(new changed_address(0x3, 4));
  msg.print_debug();

  printf("\n\n\n");

  std::vector<std::unique_ptr<stun_tlv>> elems;
  elems.push_back(std::unique_ptr<stun_tlv>(new stun_tlv(STUN_NULL_ATTRIBUTE, 0)));
  elems.push_back(std::unique_ptr<stun_tlv>(new address_tlv(MAPPED_ADDRESS, 0x1, 2)));
  elems.push_back(std::unique_ptr<stun_tlv>(new changed_address(0x3, 4)));

  for (int i = 0; i < elems.size(); ++i)
    {
      elems[i]->print_debug();
    }
  */
  
  // Prepare two serializers to send and receive stun messages.
  serializer mem_out(stun_msg::max_msg_len);
  serializer mem_in(stun_msg::max_msg_len);

  // Prepare a single outbound message.
  stun_msg outbound(BINDING_REQUEST, stun_msg::get_new_hi(), stun_msg::get_new_lo());
  outbound.add_tlv(new change_request(false, false));
  outbound.print_debug();
  outbound.serialize(mem_out);

  ssize_t sent = sendto(
      fd, 
      mem_out.data(), 
      mem_out.buffer_len(), 
      0, 
      (struct sockaddr *) &stun_target,
      sizeof(struct sockaddr)
   );

  printf("Message sent! Now waiting for response...\n");
  
  /*
  socklen_t target_addr_len = sizeof(struct sockaddr);
  ssize_t recd = recvfrom(
      fd,
      mem_in.data(),
      mem_in.remaining_space(),
      0,
      (struct sockaddr *) &stun_target,
      &target_addr_len
  );
  */

  ssize_t recd = recv(
      fd,
      mem_in.data(),
      mem_in.remaining_space(),
      0
  );

  mem_in.advance_write(recd);

  printf("Message received!\n");
  
  stun_msg inbound;
  inbound.parse(mem_in);

  printf("Received length: %ld, parsed length: %d\nReceived packet: ", recd, inbound.packet_length());

  inbound.print_debug();

  // Cleanup.
  close(fd);
  return 0;
}
