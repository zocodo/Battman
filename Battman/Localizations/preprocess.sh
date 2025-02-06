#! /bin/bash

msgids=`sed -n 's/msgid "\(.\+\)"/\1/p' base.pot`
locale_files=`ls *.po`

val=`cat $1`

lid=1
while read i; do
	#echo sed "s/_(\"$i\")/cond_localize($lid)/g"
	val=`sed "s/_(\"$i\")/cond_localize($lid)/" <<<$val`
	lid=$((lid+1))
done<<<"$msgids"

echo "$val"