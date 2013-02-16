#! /bin/bash
# TODO Consider rewrite in perl/python to make this a bit less crappy

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PATCHES_DIR=${DIR}/patches
LAST_REVISION_FILE=${DIR}/last-merged-ioq3-revision
LAST_REVISION_TEMP_FILE=${LAST_REVISION_FILE}.temp

if [ -f ${LAST_REVISION_TEMP_FILE} ]
then
  LAST_REVISION=`cat ${LAST_REVISION_TEMP_FILE}`
else
  LAST_REVISION=`cat ${LAST_REVISION_FILE}`
fi

set -f
# Things that exist in ioq3 which we don't want
EXCLUSIONS="BUGS ChangeLog README ./*.txt
  NOTTODO TODO misc/* *.mak src/cgame/*
  src/game/* src/ui/* src/q3_ui/* src/botlib/* ui/*"

EXCLUDE_PARAMETERS=""
for EXCLUSION in ${EXCLUSIONS}
do
  EXCLUDE_PARAMETERS+="--exclude=${EXCLUSION} "
done

set +f

PATCHES=`ls ${PATCHES_DIR}/*.patch 2> /dev/null`
if [ -z "${PATCHES}" ]
then
  echo "Fetching and generating patches..."
  git fetch https://github.com/ioquake/ioq3.git

  mkdir -p ${PATCHES_DIR}
  git format-patch -o ${PATCHES_DIR} ${LAST_REVISION}..FETCH_HEAD
fi

if [ -d ".git/rebase-apply" ]
then
  echo "Failed patch detected."

  git diff --quiet --exit-code
  if [ "$?" -ne 0 ]
  then
    echo "Unstaged changes present; git add any that are pending:"
    git status
    exit 1
  fi

  PATCH=`ls ${PATCHES_DIR}/*.patch | head -n 1`
  SHA=`cat ${PATCH} | head -n 1 | awk '{print $2;}'`
  echo "Processing ${SHA} ${PATCH}..."

  DIFF=`git diff --cached`
  if [ -z "${DIFF}" ]
  then
    echo "Patch does nothing; skipping."
    read -p "Confirm skip? "
    git am --skip
  else
    read -p "Confirm resolve? "
    git am --resolved
  fi

  if [ "$?" -ne 0 ]
  then
    echo "Patch failed to apply."
    exit $?
  fi

  echo ${SHA} > ${LAST_REVISION_TEMP_FILE}
  rm ${PATCH}
fi

PATCHES=`ls ${PATCHES_DIR}/*.patch 2> /dev/null`
if [ -n "${PATCHES}" ]
then
  for PATCH in ${PATCHES}
  do
    SHA=`cat ${PATCH} | head -n 1 | awk '{print $2;}'`
    echo "Processing ${SHA} ${PATCH}..."
    cat ${PATCH} | sed -e 's/\([ab]\)\/code\//\1\/src\//g' | \
      git am ${EXCLUDE_PARAMETERS} --quiet --3way

    if [ "$?" -ne 0 ]
    then
      echo "Patch failed to apply."
      git status
      exit $?
    fi

    echo ${SHA} > ${LAST_REVISION_TEMP_FILE}
    rm ${PATCH}
  done
else
  echo "Nothing to merge."
fi

# Finished merging so update the last revision marker
if [ -f ${LAST_REVISION_TEMP_FILE} ]
then
  diff ${LAST_REVISION_FILE} ${LAST_REVISION_TEMP_FILE} &> /dev/null
  if [ "$?" -ne 0 ]
  then
    mv ${LAST_REVISION_TEMP_FILE} ${LAST_REVISION_FILE}
    LAST_REVISION=`cat ${LAST_REVISION_FILE}`
    git add ${LAST_REVISION_FILE}
    git commit -m "Merged ioq3 ${LAST_REVISION}"
  fi
fi
