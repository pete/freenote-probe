
#include <stdio.h>

#include "probe.h"
#include "setup.h"
#include "options.h"
#include "lib/fileman.h"
#include "util.h"

#define TEAM_MAXLEN	256
#define PIN_MAXLEN	16

static int do_setup();

/**
	maybe_setup
	Creates the ~/.freenote/probe.team file, based on user input.
	Assumes that we are /not/ daemonized yet.
	Returns 0 for success (i.e., successful setup -or- the probe has
	already been set up), -1 for failure (ex., couldn't write to the file).
*/
int maybe_setup()
{
	fm_init_base();
	if(read_str_opt("auto-setup")) {
		return set_teamstring(read_str_opt("auto-setup"));
	}
	if(read_int_opt("setup") || 
		!(fm_exists(FM_TEAM_INFO) || read_int_opt("no-tokens"))) {
		try(do_setup());
		p_exit(PEXIT_NORMAL);
	}
	return 0;
}

/*
	do_setup
	Does the setup.  Returns 0 for success, -1 for failure.
	It doesn't use io_* because it's interactive.
*/
static int do_setup()
{
	char	team[TEAM_MAXLEN],
		pin[PIN_MAXLEN],
		*tmp = fm_abs(FM_TEAM_INFO);
	int	r;

	//  It's a little long.
	printf(	"FreeNote Probe Setup:\n"
		"Please follow the on-screen instructions.\n"
		"\n"
		"If you do not have a team, you can visit http://freenote."
		"petta-tech.com\nto register.\n"
		"If you don't want to register, you can run the probe with"
		"the\n'--no-tokens'/'-n' option, "
		"or add 'no-tokens' to ~/.freenote/rc.\n"
		"Please enter your team name:  ");
	fgets(team, TEAM_MAXLEN, stdin);
	chomp(team);
	printf("Please enter your team's pin code:  ");
	fgets(pin, PIN_MAXLEN, stdin);
	chomp(pin);
	printf("Writing the information to %s...\n", tmp);
	r = set_team_info(team, pin);
	try(r);
	printf("Ready to roll!  Run the probe again, without --setup.\n");

	return r;
}

/**
	set_team_info
	Generates ~/.freenote/probe.team from the two arguments (the team
	name and the pin code).  Returns 0 for success, -1 for failure.
*/
int set_team_info(const char *team_name, const char *pin_code)
{
	// strlen("team_name:", ";\n", "pin_code:", ";\n\0") = 24
	char buffer[TEAM_MAXLEN + PIN_MAXLEN + 24];

	sprintf(buffer, "team_name:%s;\npin_code:%s;\n", team_name, pin_code);
	return fm_write(FM_TEAM_INFO, buffer, strlen(buffer)) - 1;
}

/**
	set_teamstring
	Generates ~/.freenote/probe.team from the argument specified, which
	must be of the form "team_name:pin_code".  Returns 0 for success,
	-1 for failure.
*/
int set_teamstring(const char *teamstring)
{
	char	team[TEAM_MAXLEN],
		pin[PIN_MAXLEN],
		*colon;

	colon = strchr(teamstring, ':');
	if(colon == NULL || (colon - teamstring) > TEAM_MAXLEN)
		return -1;

	strncpy(pin, colon + 1, PIN_MAXLEN);
	strncpy(team, teamstring, colon - teamstring);
	team[colon - teamstring] = '\0';
	return set_team_info(team, pin);
}
