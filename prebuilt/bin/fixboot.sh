#!/sbin/bbx sh

exec 1<&-
exec 2<&-
exec 1<>/dev/kmsg
exec 2>&1

# mount safestrap partition
/sbin/bbx mount -t vfat -o uid=1023,gid=1023,fmask=0007,dmask=0007,allow_utime=0020 /dev/block/emstorage /ss

SLOT_LOC=$(/sbin/bbx cat /ss/safestrap/active_slot)

if [ "$SLOT_LOC" != "stock" ]; then
# setup loopbacks
/sbin/bbx losetup /dev/block/loop-system /ss/safestrap/$SLOT_LOC/system.img
/sbin/bbx losetup /dev/block/loop-userdata /ss/safestrap/$SLOT_LOC/userdata.img
/sbin/bbx losetup /dev/block/loop-cache /ss/safestrap/$SLOT_LOC/cache.img

# move real partitions out of the way
/sbin/bbx mv /dev/block/system /dev/block/systemorig
/sbin/bbx mv /dev/block/userdata /dev/block/userdataorig
/sbin/bbx mv /dev/block/cache /dev/block/cacheorig

# change symlinks
/sbin/bbx ln -s /dev/block/loop-system /dev/block/system
/sbin/bbx ln -s /dev/block/loop-userdata /dev/block/userdata
/sbin/bbx ln -s /dev/block/loop-cache /dev/block/cache
else
/sbin/bbx umount /ss
fi
