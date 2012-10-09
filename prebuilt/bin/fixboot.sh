#!/sbin/bbx sh
SS_MNT=/ss
SS_DIR=$SS_MNT/safestrap
SS_PART=emstorage

# remount root as rw
/sbin/bbx mount -o remount,rw rootfs
/sbin/bbx mkdir $SS_MNT
/sbin/bbx chmod 777 $SS_MNT
#/sbin/bbx mount -o remount,ro rootfs

# mount safestrap partition
/sbin/bbx mount -t vfat /dev/block/$SS_PART $SS_MNT
SLOT_LOC=$(/sbin/bbx cat $SS_DIR/active_slot)

/sbin/bbx mv /dev/block/system /dev/block/systemorig
/sbin/bbx mv /dev/block/userdata /dev/block/userdataorig
/sbin/bbx mv /dev/block/cache /dev/block/cacheorig
if [ -f "$SS_DIR/$SLOT_LOC/system.img" ] && [ -f "$SS_DIR/$SLOT_LOC/userdata.img" ] && [ -f "$SS_DIR/$SLOT_LOC/cache.img" ]; then
	# setup loopbacks
	/sbin/bbx losetup /dev/block/loop7 $SS_DIR/$SLOT_LOC/system.img
	/sbin/bbx losetup /dev/block/loop6 $SS_DIR/$SLOT_LOC/userdata.img
	/sbin/bbx losetup /dev/block/loop5 $SS_DIR/$SLOT_LOC/cache.img

	# change symlinks
	/sbin/bbx ln -s /dev/block/loop7 /dev/block/system
	/sbin/bbx ln -s /dev/block/loop6 /dev/block/userdata
	/sbin/bbx ln -s /dev/block/loop5 /dev/block/cache
else
	echo "stock" > /ss/safestrap/active_slot
	/sbin/bbx ln -s /dev/block/systemorig /dev/block/system
	/sbin/bbx ln -s /dev/block/userdataorig /dev/block/userdata
	/sbin/bbx ln -s /dev/block/cacheorig /dev/block/cache
fi

