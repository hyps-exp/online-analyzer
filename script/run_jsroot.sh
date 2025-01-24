#!/bin/sh

. $(dirname $(readlink -f $0))/setebhost
program=jsroot_hyps

#_______________________________________________________________________________
# if [ -z $ROOTSYS ]; then
#     echo "ROOTSYS must be set"
#     exit 1
# fi
# thisroot_sh=$ROOTSYS/bin/thisroot.sh

#_______________________________________________________________________________
top_dir=$(dirname $(readlink -f $0))/..
server=$top_dir/bin/$program

conf=/misc/software/param/pro/conf/analyzer_hyps_jsroot.conf
if [ -z "$1" ]; then
    data=${ebhost}:8901
else
    data=$top_dir/data/run$1.dat
fi

# if [ -z "$1" ]; then
#     conf=/param/conf/analyzer.conf
# else
#     conf=$1
# fi
# if [ -z "$2" ]; then
#     data=eb0:8901
# else
#     data=$2
# fi

#_______________________________________________________________________________
# screen -AmdS K18OnlineServer \
#     sh -c ". $thisroot_sh && while true; do $server $conf $data; done"
# screen -AmdS K18OnlineServer \
#     sh -c "while true; do $server $conf $data 2>/dev/null; done"
name=$program
session=`tmux ls | grep $name`
if [ -z "$session" ]; then
    echo "create new session $name"
    tmux new-session -d -s $name \
	"while true; do $server $conf $data 2>/dev/null; done"
else
    echo "reattach session $name"
    tmux a -t $name
fi
