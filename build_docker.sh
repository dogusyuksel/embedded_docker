#!/bin/bash

if [ "$#" -lt 1 ]; then
    echo "you should give the image name"
    exit 1
fi

if [ "$#" -gt 1 ]; then
    echo "you should not give additional args"
    exit 1
fi

docker build -t $@ .
