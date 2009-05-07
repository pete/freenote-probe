#ifndef __CAPS_H
#define __CAPS_H

int caps_flags();
int has_caps(int caps);
void caps_to_s(char *caps);

#define CAPS_HTTP	0x1
#define CAPS_HTTPS	0x2
#define CAPS_PING	0x4
#define CAPS_DNS	0x8

#endif
