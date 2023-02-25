#!/bin/bash

cmake -B build

cwd=$(pwd)
cd ${cwd}/build &&
make 