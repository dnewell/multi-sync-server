#!/bin/bash
for (( i=0; i<5000; i++ )); 
do
   ./client zeus 22200 $i
done
