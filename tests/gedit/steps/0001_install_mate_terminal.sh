#!/bin/bash
ROOT="$(git rev-parse --show-toplevel)"
BUILDER="/home/$(username)/ibus-unikey/Base/Tools/Builder/build"
CURRENT=$(pwd)

source $ROOT/tests/pipeline/libraries/Logcat.sh
source $ROOT/tests/pipeline/libraries/Package.sh
source $ROOT/tests/pipeline/libraries/Console.sh

rm -fr $ROOT/build

if ! exec_on_test_machine_without_output "sudo apt install -y mate-terminal"; then
	# @NOTE: this package is essential since i found that gnome-terminal can't be 
	# detected by dogtail. It should be possible issue from this framework only
	# and it should be fixed later so this workaround is applied to overcome this

	error "can't install \`mate-terminal\` onto $MACHINE"
fi

