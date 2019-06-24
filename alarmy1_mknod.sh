MODULE="alarmy1_dev"
MAJOR=$(awk "\$2==\"$MODULE\" {print \$1}" /proc/devices)
echo $major

mknod /dev/$MODULE c $MAJOR 0
