#ifndef _SIP_PRIVATE_H_
#define _SIP_PRIVATE_H_

int sip_pass_args(int argc, char *argv[]);

unsigned int sip_handle_events(void);
int sip_init(void);
void send_request(const char *str);

void  sip_set_local(const char* host, const char* port);
void  sip_set_local(const char* port);
void  sip_set_remote(const char* host, const char* port);
#endif
