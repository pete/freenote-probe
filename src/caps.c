#include <stdio.h>

#include "caps.h"

/* These are the has_x_cap functions; they return their own flag if this probe
   with this configuration has this capability; otherwise they return zero.
*/
static int has_http_cap();
static int has_https_cap();
static int has_ping_cap();
static int has_dns_cap();

/**
	caps_flags
	Returns the capabilities flags, one or more of those in the header.
*/
int caps_flags()
{
	return 	has_http_cap() |
		has_https_cap() |
		has_ping_cap() |
		has_dns_cap();
}

/**
	has_caps
	Accepts one or more capabilities flags or'd together, and returns true
	if the probe is capable.
*/
int has_caps(int caps)
{
	return caps == (caps_flags() & caps);
}

/**
	caps_to_s
	Writes the capabilites as a string to the space you provide.  Make sure
	the string has at least (ceil(sizeof(int) * 2.5) + 1) bytes of space, 
	because for now we send a set of flags or'd together.
*/
void caps_to_s(char *caps)
{
	sprintf(caps, "%d", caps_flags());
}

static int has_http_cap()
{
	/* For now, there is no such thing as a probe without http. */
	return CAPS_HTTP;
}

static int has_https_cap()
{
	/* Same for https. */
	return CAPS_HTTPS;
}

static int has_ping_cap()
{
	/* Only root can ping. */
	if(getuid() == 0 || geteuid() == 0) {
		return CAPS_PING;
	}

	return 0;
}

static int has_dns_cap()
{
	/* NOBODY has DNS for now.  The protocol doesn't support it.  */
	return 0; 
}
