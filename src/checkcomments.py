#!/usr/bin/python

import sys

def trimline(line):
  append = False
  trimmedline = ''
  for i in range (1,len(line)):
    if line[i] != ' ':
      append = True
    if append:
      trimmedline = trimmedline + line[i]
  return trimmedline
    
def fileinfo(name,ln):
  info = str(ln)
  while len(info) < 5:
    info = info + ' '
  info = name + ' line ' + info;
  return info    

for i in range(1,len(sys.argv)):
  f = open(sys.argv[i])
  line = f.readline()
  conti = True
  ln = 1
  while line:
    tline = trimline(line)
    if tline[0:6] == '///@@@':
#      if not conti:
#        print
      print fileinfo(sys.argv[i],ln) + ':' + tline[6:len(tline)-1]
      conti = True
    else:
      conti = False
    line = f.readline()
    ln = ln + 1
  f.close
  
  
