#!/bin/bash

# ------------------------------------------------------------------------------
#  compilation
# ------------------------------------------------------------------------------
rm simpfer
g++ -O3 -o simpfer main.cpp -std=c++14 -fopenmp -w


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

./simpfer -n ${n} -m ${m} -qn ${qn} -d ${d} -is ${items} -us ${users} \
  -qs ${query} -of ${folder}


# ------------------------------------------------------------------------------
#  Netflix
# ------------------------------------------------------------------------------
name=Netflix
n=17670
m=480189
qn=100
d=100
items=../data/bin/${name}/${name}.items 
users=../data/bin/${name}/${name}.users 
query=../data/bin/${name}/${name}.query 
folder=../results/${name}/

./simpfer -n ${n} -m ${m} -qn ${qn} -d ${d} -is ${items} -us ${users} \
  -qs ${query} -of ${folder}


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

./simpfer -n ${n} -m ${m} -qn ${qn} -d ${d} -is ${items} -us ${users} \
  -qs ${query} -of ${folder}


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

./simpfer -n ${n} -m ${m} -qn ${qn} -d ${d} -is ${items} -us ${users} \
  -qs ${query} -of ${folder}


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

./simpfer -n ${n} -m ${m} -qn ${qn} -d ${d} -is ${items} -us ${users} \
  -qs ${query} -of ${folder}

