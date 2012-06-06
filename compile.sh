#!/bin/sh

if test ! -x `which gcc`;
then
  echo "Please install gcc."
  exit 1
fi

if test ! -x `which uname`;
then
  echo "You need the uname command in order to compile without editing Makefiles."
  exit 1
fi

if test ! -x `which grep`;
then
  echo "You need the grep command in order to compile without editing Makefiles."
  exit 1
fi

uname=`uname`
if test "$?" -ne "0"; then
  echo "You need the uname command in order to compile without editing Makefiles."
  exit 1
fi

echo "$uname" | grep "BSD" >/dev/null 2>&1
if test "$?" -eq "0"; then
  echo "You are using a BSD system."
  cat src/Makefile.dist | sed s/@OS@/bsd/ >src/Makefile
  echo "Now type 'make all' to build the program."
  exit 0
fi

echo "$uname" | grep "Linux" >/dev/null 2>&1
if test "$?" -eq "0"; then
  echo "You are using a Linux system."
  cat src/Makefile.dist | sed s/@OS@/linux/ >src/Makefile
  echo "Now type 'make all' to build the program."
  exit 0
fi

echo "I don't recognize you're system..."
cat src/Makefile.dist | sed s/@OS@/unix/ >src/Makefile
echo "Yry make..."
exit 1
