#!/bin/sh

. $(dirname $(readlink -f $0))/setebhost
program=jsroot_hyps_hoge

top_dir=$(dirname $(readlink -f $0))/..
server=$top_dir/bin/$program

conf=/misc/software/param/pro/conf/analyzer_hyps_jsroot.conf

#_____ Check Online or Semi-Online _____________________________________________
if [ $# -eq 0 ]; then
    data=${ebhost}:8901
    port=9090
    name=${program}
elif [ $# -eq 1 ]; then
    echo -ne "\033[1;31m [ERROR] \033[0m"
    echo "Port Number should be specified for Semi-Online"
    exit 1
elif [ $# -eq 2 ]; then
    #--- Check if the port is used
    port_check=$(ss -tuln | grep -E "\:$2(\s|$)" > /dev/null; echo $?)
    if [ $port_check -eq 1 ]; then
	run_num=$(printf "%05d" "$1")
	data=/misc/rawdata/run${run_num}.dat
	#--- Check if rawdata exists
	if [ -e $data ]; then
	    port=$2
	    name=semi_${run_num}_${program}
	else
	    echo -ne "\033[1;31m [ERROR] \033[0m"
	    echo ""$data" does not exist"
	    exit 1
	fi
    elif [ $port_check -eq 0 ]; then
	echo -ne "\033[1;31m [ERROR] \033[0m"
	echo "Port "$2" is already used"
	exit 1
    else
	echo -ne "\033[1;31m [ERROR] \033[0m"
	echo "Something wrong"
	exit 1
    fi
elif [ $# -ge 3 ]; then
    echo -ne "\033[1;31m [ERROR] \033[0m"
    echo "Too many arguments"
    exit 1
fi

#_____ Launch the HTTP server _________________________________________________
session_check=$(tmux ls 2>/dev/null | grep "^$name:")
if [ -z "$session_check" ]; then
    echo -ne "\033[1;36m [INFO] \033[0m"
    echo "Create new session: $name"
    tmux new-session -d -s $name \
	 "while true; do $server $conf $data $port 2>/dev/null; done"
else
    echo -ne "\033[1;33m [WARNING] \033[0m"
    if [ $# -eq 0 ]; then
	echo "Online analyzer is already running."
    else
	echo "Semi-Online analyzer for Run "$1" is already running."
    fi
fi
