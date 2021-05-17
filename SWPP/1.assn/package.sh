#!/bin/bash

rm -f submit.tar.gz
ls -A . | xargs tar -cvzf submit.tar.gz
