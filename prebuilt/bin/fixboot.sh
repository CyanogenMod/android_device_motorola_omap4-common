#!/sbin/bbx sh
# By: Hashcode
# Last Editted: 11/07/2012
BLOCK_DIR=/dev/block
BLOCKNAME_DIR=$BLOCK_DIR

SS_PART=emstorage
IMG_TYPE=vfat
SS_MNT=/ss
SS_DIR=$SS_MNT/safestrap

# check for SS loopdevs
if [ ! -f "$BLOCK_DIR/loop-system" ]; then
	# create SS loopdevs
	/sbin/bbx mknod -m600 /dev/block/loop-system b 7 99
	/sbin/bbx mknod -m600 /dev/block/loop-userdata b 7 98
	/sbin/bbx mknod -m600 /dev/block/loop-cache b 7 97
fi

# check for systemorig partition alias so we don't run this twice
if [ ! -f "$BLOCKNAME_DIR/systemorig" ]; then
	# move real partitions out of the way
	/sbin/bbx mv $BLOCKNAME_DIR/system $BLOCKNAME_DIR/systemorig
	/sbin/bbx mv $BLOCKNAME_DIR/userdata $BLOCKNAME_DIR/userdataorig
	/sbin/bbx mv $BLOCKNAME_DIR/cache $BLOCKNAME_DIR/cacheorig

	# remount root as rw
	/sbin/bbx mount -o remount,rw rootfs
	/sbin/bbx mkdir $SS_MNT
	/sbin/bbx chmod 777 $SS_MNT

	# mount safestrap partition
	/sbin/bbx mount -t $IMG_TYPE $BLOCKNAME_DIR/$SS_PART $SS_MNT
	SLOT_LOC=$(/sbin/bbx cat $SS_DIR/active_slot)

	if [ -f "$SS_DIR/$SLOT_LOC/system.img" ] && [ -f "$SS_DIR/$SLOT_LOC/userdata.img" ] && [ -f "$SS_DIR/$SLOT_LOC/cache.img" ]; then
		# setup loopbacks
		/sbin/bbx losetup $BLOCK_DIR/loop-system $SS_DIR/$SLOT_LOC/system.img
		/sbin/bbx losetup $BLOCK_DIR/loop-userdata $SS_DIR/$SLOT_LOC/userdata.img
		/sbin/bbx losetup $BLOCK_DIR/loop-cache $SS_DIR/$SLOT_LOC/cache.img

		# change symlinks
		/sbin/bbx ln -s $BLOCK_DIR/loop-system $BLOCKNAME_DIR/system
		/sbin/bbx ln -s $BLOCK_DIR/loop-userdata $BLOCKNAME_DIR/userdata
		/sbin/bbx ln -s $BLOCK_DIR/loop-cache $BLOCKNAME_DIR/cache
	else
		echo "stock" > $SS_DIR/active_slot
		/sbin/bbx ln -s $BLOCKNAME_DIR/systemorig $BLOCKNAME_DIR/system
		/sbin/bbx ln -s $BLOCKNAME_DIR/userdataorig $BLOCKNAME_DIR/userdata
		/sbin/bbx ln -s $BLOCKNAME_DIR/cacheorig $BLOCKNAME_DIR/cache
	fi
fi

