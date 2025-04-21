#!/bin/sh

. $(dirname $(readlink -f $0))/setebhost
program=jsroot_scaler

#______________________________________________________________________________
top_dir=$(dirname $(readlink -f $0))/..
server=$top_dir/bin/$program

conf=/misc/software/param/pro/conf/analyzer_jsroot_scaler.conf
if [ -z "$1" ]; then
    data=${ebhost}:8901
else
    data=$top_dir/data/run$1.dat.gz
fi

#______________________________________________________________________________
name=jsroot_scaler
session=`tmux ls 2>/dev/null | grep $name`
if [ -z "$session" ]; then
    echo "create new session $name"
    tmux new-session -d -s $name \
	"while true; do $server $conf $data; sleep 1; done"
else
    echo "reattach session $name"
    tmux a -t $name
fi
