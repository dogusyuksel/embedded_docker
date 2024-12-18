#!/bin/bash

if [ "$#" -lt 1 ]; then
    echo "you should give the image name"
    exit 1
fi

if [ "$#" -gt 1 ]; then
    argc=$#
    argv=("$@")
    full_string=""
    priv_string=""

    for (( j=1; j<argc; j++ )); do
        full_string="$priv_string${argv[j]} "
        priv_string=$full_string
    done
    # echo "->$full_string<-"
    docker run --rm -t --net=host -v $(pwd):/workspace -v /etc/passwd:/etc/passwd:ro -v /etc/group:/etc/group:ro -u $(id -u ${USER}):$(id -g ${USER}) --entrypoint=/bin/bash ${argv[0]} -c "$full_string"
else
    docker run --rm -it --net=host -v $(pwd):/workspace -v /etc/passwd:/etc/passwd:ro -v /etc/group:/etc/group:ro -u $(id -u ${USER}):$(id -g ${USER}) --entrypoint=/bin/bash $@
fi

