# remount root as rw
/sbin/bbx mount -o remount,rw rootfs
/sbin/bbx mkdir /ss
/sbin/bbx chmod 777 /ss

# mount safestrap partition
/sbin/bbx mount -t vfat /dev/block/mmcblk1p25 /ss

# create SS loopdevs
/sbin/bbx mknod -m600 /dev/block/loop-system b 7 99
/sbin/bbx mknod -m600 /dev/block/loop-userdata b 7 98
/sbin/bbx mknod -m600 /dev/block/loop-cache b 7 97

# move real partitions out of the way
/sbin/bbx mv /dev/block/system /dev/block/systemorig
/sbin/bbx mv /dev/block/userdata /dev/block/userdataorig
/sbin/bbx mv /dev/block/cache /dev/block/cacheorig

#SLOT_LOC=$(/sbin/bbx cat /ss/safestrap/active_slot)

# setup loopbacks
/sbin/bbx losetup /dev/block/loop-system /ss/safestrap/rom-slot1/system.img
/sbin/bbx losetup /dev/block/loop-userdata /ss/safestrap/rom-slot1/userdata.img
/sbin/bbx losetup /dev/block/loop-cache /ss/safestrap/rom-slot1/cache.img

# change symlinks
/sbin/bbx ln -s /dev/block/loop-system /dev/block/system
/sbin/bbx ln -s /dev/block/loop-userdata /dev/block/userdata
/sbin/bbx ln -s /dev/block/loop-cache /dev/block/cache

# LOG HERE
/sbin/bbx ls -l /dev/block/* > /ss/block-dir.txt
/sbin/bbx sync

