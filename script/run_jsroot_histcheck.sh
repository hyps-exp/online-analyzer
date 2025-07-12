#!/bin/sh

. $(dirname $(readlink -f $0))/setebhost
program=jsroot_hyps

top_dir=$(dirname $(readlink -f $0))/..
server=$top_dir/bin/$program

conf=/misc/software/param/pro/conf/analyzer_hyps_jsroot.conf
data=${ebhost}:8901
port=9092

if [ -z "$1" ]; then
    threshould=500000
else
    threshould=$1
fi

name="$histcheck_{program}"
session=`tmux ls | grep $name`
if [ -z "$session" ]; then
    echo "create new session $name"
    tmux new-session -d -s $name \
        "while true; do $server $conf $data $port $theshould 2>/dev/null; done"
else
    echo "reattach session $name"
    tmux a -t $name
fi
