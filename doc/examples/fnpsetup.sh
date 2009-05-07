#!/bin/sh
#  fnpsetup.sh
#  A program for generating the authentication file used by the FreeNote Probe.
#  Copyright (c) 2005 Petta Technology
#  See the file 'COPYING' in the source tree for more infomation.
FILELOC="$HOME/.freenote/probe.team"
if [ -f "$FILELOC" ]; then
	eval `sed s/:/=/g "$FILELOC"` # Very inexpensive file-parser.
fi

echo "FreeNote Setup:"
echo "Please follow the on-screen instructions.  Defaults are in brackets."

echo "Where do you want to save data?"
echo -n "[$FILELOC]  "
read INPUT
if [ "$INPUT" != "" ]; then
	FILELOC="$INPUT"
fi

echo "What is your team name?"
echo -n "[$team_name]  "
read INPUT
if [ "$INPUT" != "" ]; then
	TEAM_NAME="$INPUT"
else
	TEAM_NAME="$team_name"
fi
while [ "$TEAM_NAME" = "" ]; do
	echo -n "Blank team name.  Please re-enter:  "
	read TEAM_NAME;
done

echo "What is your PIN code?"
echo -n "[$pin_code]  "
read INPUT
if [ "$INPUT" != "" ]; then
	PIN_CODE="$INPUT"
else
	PIN_CODE="$pin_code"
fi
while [ "$PIN_CODE" = "" ]; do
	echo -n "Blank PIN code.  Please re-enter:  "
	read PIN_CODE;
done

echo 
echo "About to write the following information to $FILELOC..."
echo "Team Name:  $TEAM_NAME"
echo "PIN Code:  $PIN_CODE"
echo "Press enter to continue or ^C to cancel."
read Waiting

echo "team_name:$TEAM_NAME;" > $FILELOC
echo "pin_code:$PIN_CODE;" >> $FILELOC

echo "Done."
