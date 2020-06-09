#!/bin/bash
IDX=0

PIPELINE="$(dirname "$0" )"
ROOT="$(git rev-parse --show-toplevel)"

source $PIPELINE/libraries/Logcat.sh
source $PIPELINE/libraries/Package.sh
source $PIPELINE/libraries/Console.sh

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
	BUILDER="/home/$(username)/ibus-unikey/Base/Tools/Builder/build"

	for I in {0..50}; do
		export MACHINE="$DISTRO"
		export ADDRESS="192.168.100.2"

		if ! sshpass -p "rootroot" ssh $SSHOPTs root@192.168.100.2 -tt exit 0 &> /dev/null; then
			sleep 10
			continue
		elif [ -d $ROOT/build ]; then
			rm -fr $ROOT/build

			if ! sshpass -p "rootroot" ssh $SSHOPTs root@192.168.100.2 -tt "echo '$(username):$(password)' | chpasswd" &> /dev/null; then
				error "can't change password account $(username)"
			elif ! sshpass -p "$(password)" ssh $SSHOPTs $(username)@192.168.100.2 -tt exit 0 &> /dev/null; then
				error "it seems $(username)'s password is set wrong"
			elif ! rsync_to_test_machine $ROOT --exclude={'pxeboot','vms'}; then
				error "can't rsync $ROOT to /home/$(username)/ibus-unikey"
			elif exec_on_test_machine_without_output "mkdir /home/$(username)/ibus-unikey/build"; then
				if ! exec_on_test_machine "cd ~/ibus-unikey && BUILD='-DCMAKE_INSTALL_PREFIX=/usr' $BUILDER --root /home/$(username)/ibus-unikey --rebuild 0 --mode 1"; then
					error "can't build ibus-unikey with machine $MACHINE"
				fi
			fi
		fi

		if ! is_fully_started "192.168.100.2"; then
			sleep 10
		else
			AVAILABLE=1
			CURRENT=$(pwd)

			info "machine $DISTRO is available now, going to test our test suites"
				
			if exec_on_test_machine 'loginctl session-status 1 | grep Service: | grep wayland'; then
				export DISPLAY="WAYLAND_DISPLAY=wayland-0 DISPLAY=:0"

				warning "machine $MACHINE is using wayland"
			else
				export DISPLAY="DISPLAY=:0"
			fi

			for ITEM in $(ls -1c $ROOT/tests); do
				cd $CURRENT

				if [ $ITEM = "pipeline" ]; then
					continue
				elif [ ! -d $ROOT/tests/$ITEM ]; then
					continue
				elif [ ! -d $ROOT/tests/$ITEM/steps ]; then
					continue
				fi
			 	
				export SUITE="$ROOT/tests/$ITEM"

				for STEP in $(ls -1c $SUITE/steps | sort); do
					EXT="${STEP##*.}"
					
					warning """starting $STEP of suite $(basename $SUITE)"

					if [ $EXT = 'sh' ]; then
						. $SUITE/steps/$STEP
					else
						echo """-------------------------------------------------------------------------------
"""
						if ! copy_to_test_machine $SUITE/steps/$STEP; then
							error "fail to copy $STEP to $MACHINE"
						elif ! exec_on_test_machine "$DISPLAY ~/$STEP"; then
							error "fail at step $STEP"
						fi

						echo "-------------------------------------------------------------------------------"
					fi

					info "Done $STEP of suite $(basename $SUITE)"
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
