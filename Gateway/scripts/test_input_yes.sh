#!/bin/bash

echo "*** Y/N ?"
read yesno
if [ "$yesno" = "y" ] || [ "$yesno" = "Y" ]
	then
		echo "yes"
	else
		echo "no"	
fi

	