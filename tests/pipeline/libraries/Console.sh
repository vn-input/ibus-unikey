#!/bin/bash
ROOT="$(git rev-parse --show-toplevel)"

source $ROOT/tests/pipeline/libraries/Package.sh

function copy_to_test_machine() {
	if [[ ${#MACHINE} -gt 0 ]]; then
		source $ROOT/tests/pipeline/environments/$MACHINE
	else
		return -1
	fi

	if [[ ${#ADDRESS} -gt 0 ]]; then
		SSHOPTs="-o StrictHostKeyChecking=no"

		if ! sshpass -p "$(password)" ssh $SSHOPTs "$(username)@$ADDRESS" "mkdir -p ~/$(dirname $1)"; then
			return -2
		elif ! sshpass -p "$(password)" scp $SSHOPTs -r "$1" "$(username)@$ADDRESS:~/$(basename $1)"; then
			return -3
		fi
	else
		return -4
	fi
}

function rsync_to_test_machine() {
	INPUT=$1
	shift

	if [[ ${#MACHINE} -gt 0 ]]; then
		source $ROOT/tests/pipeline/environments/$MACHINE
	else
		return -1
	fi

	if [[ ${#ADDRESS} -gt 0 ]]; then
		if ! sshpass -p $(password) rsync $@ -a $INPUT $(username)@$ADDRESS:~/; then
			return -2
		fi
	fi
}

function exec_on_test_machine() {
	if [[ ${#MACHINE} -gt 0 ]]; then
		source $ROOT/tests/pipeline/environments/$MACHINE
	else
		return -1
	fi

	while [ $# -gt 0 ]; do
		case $1 in
			--timeout)	TIMEOUT="$2"; shift;;
			--add-arg)	ARGUMENTs="$ARGUMENTs $2"; shift;;
			(--) 		shift; break;;
			(-*)		break;;
			(*) 		break;;
		esac
		shift
	done

	if [[ ${#ADDRESS} -gt 0 ]]; then
		SSHOPTs="-o StrictHostKeyChecking=no"

		if [[ ${#TIMEOUT} -ne 0 ]]; then
			if ! timeout $TIMEOUT sshpass -p "$(password)" ssh $ARGUMENTs $SSHOPTs "$(username)@$ADDRESS" "$@"; then
				return -2
			fi
		elif ! sshpass -p "$(password)" ssh $ARGUMENTs $SSHOPTs "$(username)@$ADDRESS" "$@"; then
			return -2
		fi
	else
		return -3
	fi
}

function exec_on_test_machine_without_output() {
	if [[ ${#MACHINE} -gt 0 ]]; then
		source $ROOT/tests/pipeline/environments/$MACHINE
	else
		return -1
	fi

	while [ $# -gt 0 ]; do
		case $1 in
			--timeout)	TIMEOUT="$2"; shift;;
			--add-arg)	ARGUMENTs="$ARGUMENTs $2"; shift;;
			(--) 		shift; break;;
			(-*)		break;;
			(*) 		break;;
		esac
		shift
	done

	if [[ ${#ADDRESS} -gt 0 ]]; then
		SSHOPTs="-o StrictHostKeyChecking=no"

		if [[ ${#TIMEOUT} -ne 0 ]]; then
			if ! timeout $TIMEOUT sshpass -p "$(password)" ssh $ARGUMENTs $SSHOPTs "$(username)@$ADDRESS" "$@" &> /dev/null; then
				return -2
			fi
		elif ! sshpass -p "$(password)" ssh $ARGUMENTs $SSHOPTs "$(username)@$ADDRESS" "$@" &> /dev/null; then
			return -2
		fi
	else
		return -3
	fi
}

function install_package_to_test_machine() {
	if [[ ${#MACHINE} -gt 0 ]]; then
		source $ROOT/tests/pipeline/environments/$MACHINE
	else
		return -1
	fi

	if [[ ${#INSTALL} -gt  0 ]]; then
		SSHOPTs="-o StrictHostKeyChecking=no"

		if [[ $# -eq 2 ]]; then
			if ! timeout $1 sshpass -p "rootroot" ssh $SSHOPTs "root@$ADDRESS" "$INSTALL $2" &> /dev/null; then
				return -2
			fi
		elif ! sshpass -p "rootroot" ssh $SSHOPTs "root@$ADDRESS" "$INSTALL $1" &> /dev/null; then
			return -2
		fi
	else
		return -3
	fi
}

