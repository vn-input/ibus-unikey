#!/bin/bash
ROOT="$(git rev-parse --show-toplevel)"

source $ROOT/tests/pipeline/libraries/Logcat.sh
source $ROOT/tests/pipeline/libraries/Console.sh

for CASE in $(ls -1c $SUITE); do
	EXT="${CASE##*.}"

	if [ $EXT = "py" ]; then
		if ! copy_to_test_machine $SUITE/$CASE; then
			error "can't copy $SUITE/$CASE to $MACHINE" 
		fi

		warning "run test case $CASE"

		if ! exec_on_test_machine 60 "DISPLAY=:0 python ~/$CASE"; then
			error "fail test case $CASE"
		fi
	fi
done
