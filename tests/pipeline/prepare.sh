#!/bin/bash
# File: Prepare.sh
# Description: this file should be run first and it will fetch the latest
# LibBase before use this library to build a new Pipeline

ROOT="$(git rev-parse --show-toplevel)"
BASE="$ROOT/Base"
CMAKED="$ROOT/CMakeD"

function clean() {
	source "$BASE/Tests/Pipeline/Libraries/Logcat.sh"
	source "$BASE/Tests/Pipeline/Libraries/Package.sh"

	$SU rm -fr $BASE
	$SU rm -fr $ROOT/build
	$SU git clean -fdx
	exit 0
}

DRYRUN=0

while [[ $# -gt 0 ]]; do
	case $1 in
		--verbose)	VERBOSE=1;;
		--dry-run)	DRYRUN=1;;
		--clean)	clean;;
		(--)		shift; break;;
		(*)		if [[ ${#SUBPROJECT} -eq 0 ]]; then SUBPROJECT=$1; else exit -1; fi;;
	esac
	shift
done

if [ ! -d $BASE ]; then
	if ! git clone https://github.com/hung0913208/Base $BASE; then
		exit -1
	fi

	if [[ $DRYRUN -eq 0 ]]; then
		if ! git clone https://github.com/dcarp/cmake-d.git $CMAKED; then
			exit -1
		else
			cp -a $CMAKED/cmake-d/* $BASE/CMakeModules/
		fi

		$BASE/Tests/Pipeline/Prepare.sh Unikey
		exit $?
	else
		exit 0
	fi
else
	source "$BASE/Tests/Pipeline/Libraries/Logcat.sh"
	source "$BASE/Tests/Pipeline/Libraries/Package.sh"

	PIPELINE="$(dirname "$0")"
	CURRENT="$(pwd)"
	SCRIPT="$(basename "$0")"
fi
