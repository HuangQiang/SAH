#!/bin/bash

# # ------------------------------------------------------------------------------
# #  compilation
# # ------------------------------------------------------------------------------
# rm simpfer
# g++ -O3 -o simpfer main.cpp -std=c++14 -fopenmp -w

# ------------------------------------------------------------------------------
#  Automotive
# ------------------------------------------------------------------------------
name=Automotive
n=925285
m=3873247
qn=100
d=100
items=../data/bin/${name}/${name}.items 
users=../data/bin/${name}/${name}.users 
query=../data/bin/${name}/${name}.query 
folder=../results/${name}/

k=30
./simpfer -n ${n} -m ${m} -qn ${qn} -d ${d} -is ${items}_${k} -us ${users} \
  -qs ${query}_${k} -of ${folder}${k}/
