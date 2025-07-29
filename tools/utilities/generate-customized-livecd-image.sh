#!/bin/bash
# @NOTE: how to use this script?
# ------------------------------
#
# This script is used to customize a bootable ISO image so we can
# use it to deploy VMs or we can modify LiveCD ISO images and use
# these images to deploy directly to VMs and use these machines for
# testing directly and quickly since almost things are runing directly
# on ramdisk.
#
# For example, you can config the ISO image to run with sshd service
# inside, or you can config to deploy webservice or any kind of services
# you would like to do.
#
# To help anyone to use this script, here is the explanation of how
# to define the environment scripts. The input of this script is
# another script. This script will use keyword `source` to load your
# script and use 4 functions: fetch, extract, configure and finish.
#
# As the names are suggested:
# - function `fecth` is used to download the origin ISO image,
# - function `extract` is used to convert downloaded file to dir `root`
# - function `configure` is used to apply additional features to the new
# image
# - function `finish` use to fill necessary files which is used to during
# boot or verify the image.
#

ROOT="$(git rev-parse --show-toplevel)"
BASE="$ROOT/Base"

source "$BASE/Tests/Pipeline/Libraries/Logcat.sh"
source "$BASE/Tests/Pipeline/Libraries/Package.sh"

function clean() {
	rm -fr $ROOT/${NAME}-latest.iso

	if [[ ${#ISQUASHFS} -gt 0 ]] && [ -d $ISQUASHFS ]; then
		$SU umount $ISQUASHFS
	fi

	if [[ ${#OLDCD} -gt 0 ]] && [ -d $OLDCD ]; then
		$SU umount $OLDCD
	fi
	
	if [[ ${#TEMP} -gt 0 ]]; then
		$SU rm -fr $TEMP
	fi
}

function usage() {
	echo """
Usage: $(basename $0) [ --input-type TYPE ] [ --env SCRIPT ] [ --output-type ] || 
                      [ --help ]

	--input-type TYPE 	Which input we would like to use, we could
				use \`iso\` to indicate that we will use
				iso image as the input of this script or
				we can use \`repo\` to indicate that we
				will build a new iso with a bundle of packages

	--output-type TYPE 	Which output we would like to use, we could
				use \`iso\` to indicate that the output should
				be ISO image

	--env SCRIPT		The environment script which is used to help
				to provide stub to build ISO images

	--nfs NETWORK		If we config to use nfsroot we must define the
				subnet where VMs can reach to mount a rootfs

	--no-image		Only affect if we use input type 'iso', it 
				force to not generate a new ISO image which
				takes more time while it's not usable with testing
				on CI.

	--help			Show the script's usage
	"""
}

ITYPE="iso"
OTYPE="nfs"
TEMP="$(mktemp -d --tmpdir=$HOME)"
SCRIPT="$(basename "$0")"
ADDRESS="\*"
CURRENT="$(pwd)"
OLDCD=$TEMP/livecd
GENIMAGE=1
ISQUASHFS=$TEMP/squashfs
WORKSPACE=$TEMP/workspace

while [ $# -gt 0 ]; do
	case $1 in
		--input-type)	ITYPE="$2"; shift;;
		--output-type)	OTYPE="$2"; shift;;
		--no-image)	GENIMAGE=0;;
		--output)	OUTPUT="$2"; shift;;
		--env)		ENVIRONMENT="$2"; shift;;
		--nfs)		ADDRESS="$2"; shift;;
		--help)		usage;;
		(--) 		shift; break;;
		(-*) 		error "unrecognized option $1";;
		(*) 		break;;
	esac
	shift
done

if [[ $GENIMAGE -eq 0 ]]; then
	WORKSPACE=$OUTPUT
fi

NEWCD=$WORKSPACE/livecd
OSQUASHFS=$WORKSPACE/squashfs

trap clean EXIT

if [[ ${#ENVIRONMENT} -gt 0 ]]; then
	. "$ENVIRONMENT"
else
	error "please provide the environment script"
fi

NAME=$(basename $ENVIRONMENT)
VERSION="$(version)"

if [ "$ITYPE" = 'iso' ]; then
	if ! $SU modinfo squashfs &> /dev/null; then
		if ! $SU modprobe squashfs &> /dev/null; then
			error "your system don't support squashfs"
		fi
	fi
fi

# @TODO: check if our global storage contains the customized livecd. if yes,
# download it and finish. Otherwide, we should generate new livecd and upload
# it to global storage before apply it to our pipeline

info "download the latest release of $NAME"

if [ $OTYPE != 'iso' ] && [[ ${#OUTPUT} -gt 0 ]]; then
	OSQUASHFS=$OUTPUT
fi

mkdir -p $WORKSPACE/livecd/casper
mkdir -p $OSQUASHFS

if [ "$ITYPE" = 'iso' ]; then
	if [[ ${#MEGA} -gt 0 ]]; then
		USER=$(echo $MEGA | awk '{ split($0,a,":"); print a[1] }')
		PASSWORD=$(echo $MEGA | awk '{ split($0,a,":"); print a[2] }')
	fi

	if [[ ${#PROJECT} -eq 0 ]]; then
		PROJECT=$(basename $ROOT)
	fi

	if [[ $GENIMAGE -eq 0 ]]; then
		CACHE=${NAME}.squashfs
		MIRROR=$WORKSPACE/livecd/casper
	else
		CACHE=${NAME}.iso
		MIRROR=$OUTPUT
	fi

	if [[ ${#MEGA} -gt 0 ]] && megaget --path $(pwd)/${NAME}.ver --username $USER --password $PASSWORD /Root/$PROJECT/${NAME}.ver; then
		if [ "$VERSION" != "$(cat $(pwd)/${NAME}.ver)" ]; then
			MEGA=""

			megarm --username $USER --password $PASSWORD /Root/$PROJECT/${NAME}.ver
			megarm --username $USER --password $PASSWORD /Root/$PROJECT/${CACHE}
		fi
	else
		MEGA=""
	fi

	# @NOTE: this script supports expecially with ISO image input so i
	# will provide features to help to build new ISO image easier 

	if [[ ${#MEGA} -eq 0 ]] || ! megaget --path $MIRROR --username $USER --password $PASSWORD /Root/$PROJECT/$CACHE; then
		DONE=0

		if ! fetch $ITYPE $TEMP ${NAME}-latest.iso; then
			error "can't fetch $NAME's release"
		fi

		mkdir -p $OLDCD
		mkdir -p $ISQUASHFS
	
		# @NOTE: these steps are used to clone a new workspace which base
		# on the inpu ISO image
	
		info "copy $ISQUASHFS to $OSQUASHFS"
	
		if ! $SU mount -o loop $TEMP/${NAME}-latest.iso $OLDCD &> /dev/null; then
			error "can't mount $TEMP/${NAME}-latest.iso"
		elif ! $SU mount -t squashfs -o loop $OLDCD/casper/filesystem.squashfs $ISQUASHFS &> /dev/null; then
			error "can't mount $OLDCD/casper/filesystem.squashfs"
		elif ! $SU cp -a $ISQUASHFS/* $OSQUASHFS &> /dev/null; then
			error "can't copy file in $ISQUASHFS to $OSQUASHFS"
		fi	
	elif [[ $GENIMAGE -eq 0 ]]; then
		DONE=1

		if ! fetch $ITYPE $TEMP ${NAME}-latest.iso; then
			error "can't fetch $NAME's release"
		fi

		mkdir -p $OLDCD
		mkdir -p $ISQUASHFS	

		if ! $SU mount -o loop $TEMP/${NAME}-latest.iso $OLDCD &> /dev/null; then
			error "can't mount $TEMP/${NAME}-latest.iso"
		fi
	else
		exit 0
	fi
elif ! fetch $ITYPE $TEMP $WORKSPACE; then
	error "can't fetch files to build a new ISO image"
fi

if ! extract $ITYPE $TEMP $WORKSPACE $OSQUASHFS $MIRROR $DONE; then
	error "extract packages to rootfs"
fi

if [[ $DONE -eq 1 ]]; then
	exit 0
fi

$SU cp /etc/resolv.conf $OSQUASHFS/etc/resolv.conf
$SU cp $ENVIRONMENT $OSQUASHFS/$NAME

info "reconfigure our new livecd"

if ! $SU chroot $OSQUASHFS /bin/bash -c """
#!/bin/bash
mount -t proc none /proc/
mount -t sysfs none /sys/

if [[ \$(ls -id / | awk '{ print \$1 }') -ne 2 ]]; then
	source /$NAME

	if configure $ITYPE; then
		CODE=\$?
	fi

	rm -rf /$NAME
	umount /proc
	umount /sys

	exit \$CODE
else
	rm -rf /$NAME
	umount /proc
	umount /sys
	exit -1
fi
"""; then
	error "can't configure a custom livecd"
else
	if [ "$OTYPE" = 'iso' ]; then
		if ! finish $OTYPE $NEWCD $WORKSPACE $OSQUASHFS; then
			error "can't finish preparing stub to build a new ISO image"
		fi
	else
		if ! finish $OTYPE $OLDCD $WORKSPACE $OSQUASHFS; then
			error "can't finish preparing stub to build a new ISO image"
		fi
	fi

	info "generate $ROOT/${NAME}.iso base on the latest release $NAME's livecd"

	if [ "$OTYPE" = 'iso' ]; then
		if [[ $GENIMAGE -ne 0 ]]; then
			cd $NEWCD || error "can't cd to $NEWCD"
	
			$SU mkisofs -r -V "Ubuntu-Live" 	\
				-b isolinux/isolinux.bin 	\
				-c isolinux/boot.cat 		\
				-cache-inodes -J -l 		\
				-no-emul-boot 			\
				-boot-load-size 4 		\
				-boot-info-table 		\
				-o $OUTPUT/${NAME}.iso .
		
			if [[ $? -ne 0 ]] || [ ! -f $ROOT/${NAME}.iso ]; then
				exit -1
			else
				MIRROR=$OUTPUT/${NAME}.iso
			fi
		else
			MIRROR=$(find $NEWCD -name *.squashfs)
		fi

		if [[ ${#USER} -gt 0 ]] && [[ ${#PASSWORD} -gt 0 ]]; then
			info "upload $MIRROR to global storage"

			echo "$VERSION" > $(pwd)/${NAME}.ver

			if ! megaput --path /Root/$PROJECT/$CACHE --username $USER --password $PASSWORD $MIRROR; then
				warning "can't upload $MIRROR to global storage"
			fi

			if ! megaput --path /Root/$PROJECT/${NAME}.ver --username $USER --password $PASSWORD $(pwd)/${NAME}.ver; then
				warning "can't upload $(pwd)/${NAME}.ver to global storage"
			fi
		fi
	elif [ $OTYPE = 'nfs' ]; then
		if echo "$OUTPUT $ADDRESS(ro,sync,no_wdelay,insecure_locks,no_root_squash,insecure)" | $SU tee -a /etc/exports &> /dev/null; then
			if ! $SU systemctl restart nfs-kernel-server; then
				error "can't restart nfs-kernel-server"
			fi
		else
			error "can't edit file /etc/exports"
		fi
	fi

	info "clean our workspace"
	$SU umount $ISQUASHFS
	$SU umount $OLDCD
	$SU rm -fr $TEMP
fi

cd $CURRENT || error "can't cd to $CURRENT"
