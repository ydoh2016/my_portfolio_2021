#!/bin/bash

for i in {1..6} ; do ./word input${i}.txt ; done > .tmp
diff .tmp answers.txt
