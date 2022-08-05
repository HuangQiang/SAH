#!/bin/bash

# # ------------------------------------------------------------------------------
# #  compilation
# # ------------------------------------------------------------------------------
# rm simpfer
# g++ -O3 -o simpfer main.cpp -std=c++14 -fopenmp -w

# ------------------------------------------------------------------------------
#  MovieLens
# ------------------------------------------------------------------------------
name=MovieLens
n=10581
m=69874
qn=100
d=100
items=../data/bin/${name}/${name}.items 
users=../data/bin/${name}/${name}.users 
query=../data/bin/${name}/${name}.query 
folder=../results/${name}/

k=70
./simpfer -n ${n} -m ${m} -qn ${qn} -d ${d} -is ${items}_${k} -us ${users} \
  -qs ${query}_${k} -of ${folder}${k}/
