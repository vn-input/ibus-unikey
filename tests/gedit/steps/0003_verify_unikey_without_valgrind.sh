#!/bin/bash
ROOT="$(git rev-parse --show-toplevel)"
BUILDER="/home/$(username)/ibus-unikey/Base/Tools/Builder/build"
CURRENT=$(pwd)

source $ROOT/tests/pipeline/libraries/Logcat.sh
source $ROOT/tests/pipeline/libraries/Package.sh
source $ROOT/tests/pipeline/libraries/Console.sh

SPECs=("Sanitize" "Debug")

for SPEC in ${SPECs[@]}; do
	METHODs=('telex')

	info "start testing with build mode $SPEC"
	
	DEFAULT_ENGINE=$(exec_on_test_machine "DISPLAY=:0 GTK_IM_MODULE=ibus ibus engine")

	if ! exec_on_test_machine_without_output "cd ~/ibus-unikey/build/$SPEC && sudo make install"; then
		error "can't install to /home/$(username)/$SPEC"
	elif ! exec_on_test_machine "DISPLAY=:0 GTK_IM_MODULE=ibus ibus engine Unikey"; then
		error "can't switch to use ibus-unikey"
	fi

	for METHOD in ${METHODs[@]}; do
		if ! exec_on_test_machine "DISPLAY=:0 GTK_IM_MODULE=ibus gsettings set org.freedesktop.ibus.engine.unikey input-method $METHOD"; then
			error "can't switch to method $METHOD"
		fi

		for CASE in $(ls -1c $SUITE); do
			EXT="${CASE##*.}"
	
			if [ $EXT = "py" ]; then
				warning "run test case $CASE"

				if ! exec_on_test_machine "DISPLAY=:0 GTK_IM_MODULE=ibus python ~/$CASE $METHOD"; then
					error "fail stressing test case $CASE"
				fi
			fi
		done
	done
	
	if ! exec_on_test_machine "DISPLAY=:0 GTK_IM_MODULE=ibus ibus engine $DEFAULT_ENGINE"; then
		error "can't switch back to default engine $DEFAULT_ENGINE"
	fi
done
