#!/bin/bash
ROOT="$(git rev-parse --show-toplevel)"
CURRENT=$(pwd)

source $ROOT/tests/pipeline/libraries/Logcat.sh
source $ROOT/tests/pipeline/libraries/Package.sh
source $ROOT/tests/pipeline/libraries/Console.sh

for CASE in $(ls -1c $SUITE); do
	EXT="${CASE##*.}"
	
	if [ $EXT = "py" ]; then
		if ! copy_to_test_machine $SUITE/$CASE; then
			error "can't copy $SUITE/$CASE to $MACHINE" 
		fi
	fi
done

