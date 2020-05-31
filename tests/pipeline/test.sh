#!/bin/bash
IDX=0

PIPELINE="$(dirname "$0" )"
ROOT="$(git rev-parse --show-toplevel)"

source $PIPELINE/libraries/Logcat.sh
source $PIPELINE/libraries/Package.sh

SCRIPT="$(basename "$0")"

function clean() {
	for I in {0..50}; do
		if [[ $INTERACT -eq 1 ]]; then
			info "this log is showed to prevent the ci stop the hanging pipeline"
			sleep 500
		else
			break
		fi
	done

	if [ -f $ROOT/pxeboot/nfs-ganesha.log ]; then
		info """here is the log from nfs-ganesha
-------------------------------------------------------------------------------

$(cat $ROOT/pxeboot/nfs-ganesha.log)
"""
	fi

	if [ -f $ROOT/pxeboot/atftp-syslog.log ]; then
		info """here is the log from atftpd
-------------------------------------------------------------------------------

$(cat $ROOT/pxeboot/atftp-syslog.log)
"""
	fi

	if [ -f $ROOT/pxeboot/atftp-console.log ]; then
		info """here is the log from atftpd console
-------------------------------------------------------------------------------

$(cat $ROOT/pxeboot/atftp-console.log)
"""
	fi
}

trap clean EXIT
source $ROOT/Base/Tests/Pipeline/Libraries/Logcat.sh

PASSED=1

if ! ps -aux | grep dnsmasq | grep -v grep &> /dev/null; then
	error "there are problems during starting dnsmasq"
fi

if ! ps -aux | grep atftpd | grep -v grep &> /dev/null; then
	error "there are problems during starting atftpd"
fi

if ! ps -aux | grep qemu | grep -v grep &> /dev/null; then
	error "there are problems during starting qemu"
fi

if [ -f /.dockerenv ] || [[ $FORCE -eq 1 ]]; then
	if ! ps -aux | grep nfs | grep -v grep &> /dev/null; then
		error "there are problems during starting nfs-ganesha"
	fi
fi

if which qemu-system-x86_64 &> /dev/null; then
	if [[ ${#NGROK} -gt 0 ]]; then
		info "we're opening a tunnel $($ROOT/Base/Tools/Utilities/ngrok.sh ngrok --token $NGROK --port 5901), you can use vnc-client to connect to it"
	fi
fi

info "Begin to test Ibus-Unikey"

for DISTRO in $(ls -1c $ROOT/tests/pipeline/environments); do
	source $ROOT/tests/pipeline/environments/$DISTRO

	SSHOPTs="-o StrictHostKeyChecking=no"

	IDX=$((IDX+1))
	AVAILABLE=0

	for I in {0..50}; do
		if ! sshpass -p "rootroot" ssh $SSHOPTs root@192.168.100.2 -tt exit 0 &> /dev/null; then
			sleep 10
		elif ! sshpass -p "rootroot" ssh $SSHOPTs root@192.168.100.2 -tt "echo '$(username):$(password)' | chpasswd"; then
			error "can't change password account $(username)"
		elif ! sshpass -p "$(password)" ssh $SSHOPTs $(username)@192.168.100.2 -tt exit 0; then
			error "it seems $(username)'s password is set wrong"
		elif ! is_fully_started "192.168.100.2"; then
			sleep 10
		else
			AVAILABLE=1

			info "machine $DISTRO is available now, going to test our test suites"

			for ITEM in $(ls -1c $ROOT/tests); do
				if [ $ITEM = "pipeline" ]; then
					continue
				elif [ ! -d $ROOT/tests/$ITEM ]; then
					continue
				elif [ ! -d $ROOT/tests/$ITEM/steps ]; then
					continue
				fi
				
				export SUITE="$ROOT/tests/$ITEM"
				export MACHINE="$DISTRO"
				export ADDRESS="192.168.100.2"

				for STEP in $(ls -1c $SUITE/steps | sort); do
					. $SUITE/steps/$STEP
				done
			done
			break
		fi
	done

	if [[ $PASSED -eq 0 ]]; then
		warning "there some issue with distro $DISTRO"
	fi

	if [[ $AVAILABLE -eq 0 ]]; then
		error "it seems the VM take so much time to become available"
	elif [ -f $ROOT/pxeboot/pxelinux.cfg/default.${IDX} ]; then
		mv $ROOT/pxeboot/pxelinux.cfg/default.${IDX} $ROOT/pxeboot/pxelinux.cfg/default
	else
		break
	fi
done

if [[ $PASSED -eq 1 ]]; then
	info "Finish testing Ibus-Unikey"
else
	exit -1
fi
