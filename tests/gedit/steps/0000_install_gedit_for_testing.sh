#!/bin/bash
ROOT="$(git rev-parse --show-toplevel)"

source $ROOT/tests/pipeline/libraries/Logcat.sh
source $ROOT/tests/pipeline/libraries/Console.sh

if ! install_package_to_test_machine gedit; then
	error "can't install gedit to $MACHINE"
fi

