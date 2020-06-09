#!/bin/bash
# - File: build.sh
# - Description: This bash script will be run right after prepare.sh and it will
# be used to build based on current branch you want to Tests

PIPELINE="$(dirname "$0" )"
source $PIPELINE/libraries/Logcat.sh
source $PIPELINE/libraries/Package.sh

SCRIPT="$(basename "$0")"

if [[ $# -gt 0 ]]; then
	MODE=$2
else
	MODE=0
fi

if [[ $# -gt 2 ]]; then
	NODE=$3
fi

info "You have run on machine ${machine} script ${SCRIPT}"
info "Your current dir now is $(pwd)"

if [ $(which git) ]; then
	# @NOTE: jump to branch's test suite and perform build
	ROOT="$(git rev-parse --show-toplevel)"
	BUILDER=$ROOT/Base/Tools/Builder/build

	if [[ $MODE -eq 1 ]] && [[ ${#NODE} -gt 0 ]]; then
		FORCE=0
		OTYPE="iso"
		IDX=0

		# @NOTE: fetch the latest release of supported distros so we can
       		# use them to verify our ibus-unikey black build before we deliver
		# this to the marketplace.

		mkdir -p $ROOT/pxeboot/{pxelinux.cfg,rootfs}

		if ! cp /usr/lib/PXELINUX/pxelinux.0 $ROOT/pxeboot; then
			error "can't copy pxelinux.0 to $ROOT/pxeboot"
		elif ! cp /usr/lib/syslinux/modules/bios/{ldlinux.c32,libcom32.c32,libutil.c32,vesamenu.c32} $ROOT/pxeboot; then
			error "can't copy syslinux to $ROOT/pxeboot"
		fi

		if [ ! -f /.dockerenv ]; then
			$SU systemctl stop nfs-kernel-server
			$SU systemctl stop nfs-ganesha
			$SU systemctl stop dnsmasq
			$SU systemctl stop atftpd
		fi

		if [ ! -f $ROOT/pxeboot/nfs-ganesha.config ]; then
			if [ -f /.dockerenv ] || [[ $FORCE -eq 1 ]]; then
				cat > $ROOT/pxeboot/nfs-ganesha.config << EOF
EXPORT_DEFAULTS
{
	SecType = sys,krb5,krb5i,krb5p;

	# Restrict all exports to NFS v4 unless otherwise specified
	Protocols = 3;
}

LOG {
	# Default log level for all components
	Default_Log_Level = FULL_DEBUG;

	# Where to log
	Facility {
	       name = FILE;
	       destination = "$ROOT/pxeboot/nfs-ganesha.log";
	       enable = active;
	}
}
EOF
			fi
		fi

		for DISTRO in $(ls -1c $ROOT/tests/pipeline/environments); do
			IDX=$((IDX+1))

			if [ $OTYPE = 'iso' ]; then
				BOOT="livecd/casper"
				INITRD="initrd=rootfs/$DISTRO/$BOOT/initrd"
			else
				BOOT="boot"	
			fi

			mkdir -p $ROOT/pxeboot/rootfs/$DISTRO

			if $ROOT/tools/utilities/generate-customized-livecd-image.sh 	\
					--input-type iso 				\
					--output-type $OTYPE				\
					--no-image					\
					--output $ROOT/pxeboot/rootfs/$DISTRO		\
					--env $ROOT/tests/pipeline/environments/$DISTRO; then
				info "successful generate $DISTRO's ISO image to start a new simulator"
			else
				error "can't build $DISTRO's ISO image"
			fi

			if [ $OTYPE = 'iso' ]; then
				LIVECD="$ROOT/pxeboot/rootfs/$DISTRO/livecd"
				MODE="ro,sync,no_wdelay,insecure_locks,no_root_squash,insecure"
				
				if [ -f /.dockerenv ] || [[ $FORCE -eq 1 ]]; then
					cat >> $ROOT/pxeboot/nfs-ganesha.config << EOF
EXPORT
{
	Export_Id = $IDX;
	Path = "$LIVECD";
	Pseudo = "/";
	Tag = exp$IDX;

	# Override the default set in EXPORT_DEFAULTS
	Protocols = 3;
	MaxRead = 65536;
	MaxWrite = 65536;
	PrefRead = 65536;
	PrefWrite = 65536;

	Transports = "UDP", "TCP";

	# All clients for which there is no CLIENT block that specifies a
	# different Access_Type will have RW access (this would be an unusual
	# specification in the real world since barring a firewall, this
	# export is world readable and writeable).
	Access_Type = RW;

	# FSAL block
	#
	# This is required to indicate which Ganesha File System Abstraction
	# Layer (FSAL) will be used for this export.
	#
	# The only option available for all FSALs is:
	#
	# Name (required)	The name of the FSAL
	#
	# Some FSALs have additional options, see individual FSAL documentation.

	FSAL
	{
		Name = VFS;
	}

	# CLIENT blocks
	#
	# An export may optionally have one or more CLIENT blocks. These blocks
	# specify export options for a restricted set of clients. The export
	# permission options specified in the EXPORT block will apply to any
	# client for which there is no applicable CLIENT block.
	#
	# All export permissions options are available, as well as the
	# following:
	#
	# Clients (required)	The list of clients these export permissions
	#			apply to. Clients may be specified by hostname,
	#			ip address, netgroup, CIDR network address,
	#			host name wild card, or simply "*" to apply to
	#			all clients.

	CLIENT
	{
		Clients = *;
		Squash = No_Root_Squash;
		Access_Type = RW;
	}
}
EOF
				elif echo "$LIVECD 192.168.100.0/24($MODE)" | $SU tee -a /etc/exports &> /dev/null; then
					if ! $SU systemctl restart nfs-kernel-server; then
						error """can't restart nfs-kernel-server, here is nfs-kernel-server's status:
-------------------------------------------------------------------------------

$(systemctl status nfs-kernel-server -l)
-------------------------------------------------------------------------------

list of used ports:
-------------------------------------------------------------------------------

$(netstat -tunlap)
"""
					fi
				else
					error "can't edit file /etc/exports"
				fi
			else
				LIVECD="$ROOT/pxeboot/rootfs/$DISTRO"
			fi

			if [ ! -f $ROOT/pxeboot/rootfs/$DISTRO/$BOOT/vmlinuz ]; then
				error "can't generate vmlinuz"
			elif [ ! -f $ROOT/pxeboot/rootfs/$DISTRO/$BOOT/initrd ]; then
				error "can't generate initrd"
			fi

			if [ ! -f $ROOT/pxeboot/pxelinux.cfg/default ]; then
				source $ROOT/tests/pipeline/environments/$DISTRO

				cat > $ROOT/pxeboot/pxelinux.cfg/default << EOF
default install
prompt   1
timeout  1
  
label install
	kernel rootfs/$DISTRO/$BOOT/vmlinuz
	append $INITRD netboot=nfs nfsroot=192.168.100.1:$LIVECD ip=dhcp rw
EOF
			else
				cat > $ROOT/pxeboot/pxelinux.cfg/default.${IDX} << EOF
default install
prompt   1
timeout  1
  
label install
	kernel rootfs/$DISTRO/$BOOT/vmlinuz
	append $INITRD netboot=nfs nfsroot=192.168.100.1:$LIVECD ip=dhcp rw
EOF
			fi
		done

		# @NOTE: it seems the developer would like to test with a 
		# virtual machine, so we should generate file .environment
		# here to contain approviated variables to control steps to
		# build and test Unikey with our LiveCD collection

		cat > $ROOT/.environment << EOF
source $ROOT/Base/Tests/Pipeline/Libraries/Package.sh
source $ROOT/Base/Tests/Pipeline/Libraries/Logcat.sh
source $ROOT/Base/Tests/Pipeline/Libraries/QEmu.sh

export VNC="rootroot:$NODE"
export RAM="4G"
export KER_FILENAME=""
export RAM_FILENAME=""
export IMG_FILENAME=""
export TIMEOUT="timeout 60"

if lsmod | grep 'kvm-intel\\|kvm-amd' &> /dev/null; then
	export CPU="host"
fi

function snift() {
	# @NOTE: override this function to prevent starting tcppxeboot since we don't
	# need this feature

	return 0
}

function start_dhcpd() {
	IP=\$(get_ip_interface \$1)
	MASK=\$(get_netmask_interface \$1)

	if ! which dnsmasq >& /dev/null; then
		return 1
	else
		info "start dnsmasq to control pxeboot"
	fi

	if [ -f /.dockerenv ] || [ $FORCE = 1 ]; then
		screen -ls "nfsd.pid" | grep -E '\\s+[0-9]+\\.' | awk -F ' ' '{print \$1}' | while read s; do screen -XS \$s quit; done

		screen -S "nfsd.pid" -dm					\
			$SU ganesha.nfsd -F -p $ROOT/pxeboot/nfs-ganesha.pid	\
					 -f $ROOT/pxeboot/nfs-ganesha.config	\
					 -N NIV_FULL_DEBUG
	fi

	$SU chmod +x $ROOT/pxeboot

	screen -ls "dhcpd.pid" | grep -E '\\s+[0-9]+\\.' | awk -F ' ' '{print \$1}' | while read s; do screen -XS \$s quit; done
	screen -ls "atftp.pid" | grep -E '\\s+[0-9]+\\.' | awk -F ' ' '{print \$1}' | while read s; do screen -XS \$s quit; done

	screen -S "dhcpd.pid" -dm 					\
		$SU dnsmasq --listen-address=192.168.100.1		\
			    --interface=\$1				\
			    --bind-interfaces	 			\
			    --dhcp-boot=pxelinux.0			\
			    --dhcp-range=192.168.100.2,192.168.100.2	\
			    --dhcp-option=3,0.0.0.0			\
			    --dhcp-option=6,0.0.0.0 --dhcp-script \$2

	echo """
logfile $ROOT/pxeboot/atftp-console.log
logfile flush 1
log on
logtstamp after 1
logtstamp string "[ %t: %Y-%m-%d %c:%s ]\\012"
logtstamp on
"""> $ROOT/vms/atftp.conf
	screen -S "atftp.pid" -c $ROOT/vms/atftp.conf -dmL		\
		$SU atftpd --daemon --no-fork $ROOT/pxeboot		\
			   --user root --group root			\
			   --trace --verbose 7				\
			   --logfile $ROOT/pxeboot/atftp-syslog.log

        $SU iptables -A INPUT -p tcp -m tcp --dport 111 -j ACCEPT
        $SU iptables -A INPUT -p tcp -m tcp --dport 2049 -j ACCEPT
        $SU iptables -A INPUT -p tcp -m tcp --dport 20048 -j ACCEPT
        $SU iptables -A INPUT -p udp -m udp --dport 111 -j ACCEPT
        $SU iptables -A INPUT -p udp -m udp --dport 2049 -j ACCEPT
        $SU iptables -A INPUT -p udp -m udp --dport 20048 -j ACCEPT
}

function stop_dhcpd() {
	info "stop dnsmasq"

	screen -ls "nfsd.pid" | grep -E '\\s+[0-9]+\\.' | awk -F ' ' '{print \$1}' | while read s; do screen -XS \$s quit; done
	screen -ls "dhcpd.pid" | grep -E '\\s+[0-9]+\\.' | awk -F ' ' '{print \$1}' | while read s; do screen -XS \$s quit; done
	screen -ls "atftp.pid" | grep -E '\\s+[0-9]+\\.' | awk -F ' ' '{print \$1}' | while read s; do screen -XS \$s quit; done
}
EOF

		$SU chmod -R 755 $ROOT/pxeboot

		info "going to test with virtual machine"
	elif ! $BUILDER --root $ROOT --debug 1 --rebuild 0 --mode $MODE; then
		exit -1
	fi
else
	error "Please install git first"
fi

info "Congratulation, you have passed ${SCRIPT}"
