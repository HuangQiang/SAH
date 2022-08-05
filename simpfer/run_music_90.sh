#!/bin/bash

# # ------------------------------------------------------------------------------
# #  compilation
# # ------------------------------------------------------------------------------
# rm simpfer
# g++ -O3 -o simpfer main.cpp -std=c++14 -fopenmp -w

# ------------------------------------------------------------------------------
#  Music100
# ------------------------------------------------------------------------------
name=Music100
n=999900
m=999900
qn=100
d=100
items=../data/bin/${name}/${name}.items 
users=../data/bin/${name}/${name}.users 
query=../data/bin/${name}/${name}.query 
folder=../results/${name}/

k=90
./simpfer -n ${n} -m ${m} -qn ${qn} -d ${d} -is ${items}_${k} -us ${users} \
  -qs ${query}_${k} -of ${folder}${k}/
