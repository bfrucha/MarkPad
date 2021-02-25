#!/bin/sh

files_nb=$(tar tf $1 | wc -l | sed 's/ //g');
filename=$2
l=${#filename}
output=${filename:0:$((l-4))}$files_nb${filename:$((l-4)):$l}
mv $filename $output
tar rf $1 $output
rm -f $output
bb