#!/bin/bash

# Do not uncomment these lines to directly execute the script
# Modify the path to fit your need before using this script

INPUT_FILE=/user/ta/PageRank/input-$1
OUTPUT_FILE=PageRank/output
JAR=PageRank.jar
ITERATION=$2

rm -rf pagerank_$1.out
hdfs dfs -rm -r $OUTPUT_FILE
hadoop jar $JAR PageRank.PageRank $INPUT_FILE $OUTPUT_FILE $ITERATION
hdfs dfs -getmerge $OUTPUT_FILE/final pagerank_$1.out
 