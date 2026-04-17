#!/bin/bash

/usr/bin/pkill -x wr-dio-agent
/usr/bin/pkill -x wr-dio-ruler
/sbin/ip link set wr0 down
/sbin/rmmod wr-nic
/sbin/rmmod spec
/sbin/rmmod fmc
