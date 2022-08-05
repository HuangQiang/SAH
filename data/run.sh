#!/bin/bash
g++ -std=c++11 -w -O3 -o generate generate.cc 
qn=100

# # ------------------------------------------------------------------------------
# #  MovieLens
# # ------------------------------------------------------------------------------
# n=69874
# d=100
# dname=MovieLens
# ifile=bin/${dname}/${dname}.users
# ofile=bin/${dname}/${dname}.mips.query

# ./generate ${n} ${d} ${qn} ${ifile} ${ofile}

# # ------------------------------------------------------------------------------
# #  Netflix
# # ------------------------------------------------------------------------------
# n=480189
# d=100
# dname=Netflix
# ifile=bin/${dname}/${dname}.users
# ofile=bin/${dname}/${dname}.mips.query

# ./generate ${n} ${d} ${qn} ${ifile} ${ofile}

# # ------------------------------------------------------------------------------
# #  Automotive
# # ------------------------------------------------------------------------------
# n=3873247
# d=100
# dname=Automotive
# ifile=bin/${dname}/${dname}.users
# ofile=bin/${dname}/${dname}.mips.query

# ./generate ${n} ${d} ${qn} ${ifile} ${ofile}

# # ------------------------------------------------------------------------------
# #  CDs_and_Vinyl
# # ------------------------------------------------------------------------------
# n=75258
# d=100
# dname=CDs_and_Vinyl
# ifile=bin/${dname}/${dname}.users
# ofile=bin/${dname}/${dname}.mips.query

# ./generate ${n} ${d} ${qn} ${ifile} ${ofile}

# # ------------------------------------------------------------------------------
# #  Deep
# # ------------------------------------------------------------------------------
# n=1000000
# d=256
# dname=Deep
# ifile=bin/${dname}/${dname}.data
# ofile=bin/${dname}/${dname}.mips.query

# ./generate ${n} ${d} ${qn} ${ifile} ${ofile}

# # ------------------------------------------------------------------------------
# #  GloVe100
# # ------------------------------------------------------------------------------
# n=1183514
# d=100
# dname=GloVe100
# ifile=bin/${dname}/${dname}.data
# ofile=bin/${dname}/${dname}.mips.query

# ./generate ${n} ${d} ${qn} ${ifile} ${ofile}

# # ------------------------------------------------------------------------------
# #  Sift
# # ------------------------------------------------------------------------------
# n=1000000
# d=128
# dname=Sift
# ifile=bin/${dname}/${dname}.data
# ofile=bin/${dname}/${dname}.mips.query

# ./generate ${n} ${d} ${qn} ${ifile} ${ofile}

# ------------------------------------------------------------------------------
#  Sift_d
# ------------------------------------------------------------------------------
n=985462
d=128
dname=Sift_d
ifile=bin/${dname}/${dname}.data
ofile=bin/${dname}/${dname}.mips.query

./generate ${n} ${d} ${qn} ${ifile} ${ofile}

# ------------------------------------------------------------------------------
#  Gist_d
# ------------------------------------------------------------------------------
n=982694
d=960
dname=Gist_d
ifile=bin/${dname}/${dname}.data
ofile=bin/${dname}/${dname}.mips.query

./generate ${n} ${d} ${qn} ${ifile} ${ofile}

# ------------------------------------------------------------------------------
#  ImageNet
# ------------------------------------------------------------------------------
n=2340373
d=150
dname=ImageNet
ifile=bin/${dname}/${dname}.data
ofile=bin/${dname}/${dname}.mips.query

./generate ${n} ${d} ${qn} ${ifile} ${ofile}

# ------------------------------------------------------------------------------
#  Msong
# ------------------------------------------------------------------------------
n=992272
d=420
dname=Msong
ifile=bin/${dname}/${dname}.data
ofile=bin/${dname}/${dname}.mips.query

./generate ${n} ${d} ${qn} ${ifile} ${ofile}

# ------------------------------------------------------------------------------
#  Tiny1M
# ------------------------------------------------------------------------------
n=1000000
d=384
dname=Tiny1M
ifile=bin/${dname}/${dname}.data
ofile=bin/${dname}/${dname}.mips.query

./generate ${n} ${d} ${qn} ${ifile} ${ofile}

# ------------------------------------------------------------------------------
#  UKBench
# ------------------------------------------------------------------------------
n=1097907
d=128
dname=UKBench
ifile=bin/${dname}/${dname}.data
ofile=bin/${dname}/${dname}.mips.query

./generate ${n} ${d} ${qn} ${ifile} ${ofile}
