#!/bin/bash

# # ------------------------------------------------------------------------------
# #  compilation
# # ------------------------------------------------------------------------------
# rm simpfer
# g++ -O3 -o simpfer main.cpp -std=c++14 -fopenmp -w

# ------------------------------------------------------------------------------
#  CDs and Vinyl
# ------------------------------------------------------------------------------
name=CDs_and_Vinyl
n=64343
m=75258
qn=100
d=100
items=../data/bin/${name}/${name}.items 
users=../data/bin/${name}/${name}.users 
query=../data/bin/${name}/${name}.query 
folder=../results/${name}/

k=50
./simpfer -n ${n} -m ${m} -qn ${qn} -d ${d} -is ${items}_${k} -us ${users} \
  -qs ${query}_${k} -of ${folder}${k}/
