#!/bin/bash
export PROGRAMMER="usbtiny"

pushd ..
    make clean
    make all
popd
