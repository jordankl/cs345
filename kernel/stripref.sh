#!/bin/bash

files=$(ls -1R | sed 's#^\(.*\):$#\1/#')

for f in $files; do
    if [[ "$f" =~ / ]]; then
        d="$f"
    elif [[ ! "$d" =~ \.git ]]; then
        if [[ "$f" =~ \.[ch]$ ]]; then
	echo $d$f
            awk 'BEGIN { jdoc=0; }
                /\/\/ @START_REF_CODE/ { jdoc=1; }
                { if (jdoc==0) { print } }
                /\/\/ @END_REF_CODE/ { jdoc=0; }' < $d$f > $d$f.temp
            mv $d$f.temp $d$f
        fi
    fi
done
