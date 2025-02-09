#! /bin/bash

msgids=`sed -n 's/msgid "\(.\+\)"/\1/p' base.pot`
locale_files=`ls *.po`

val=`cat $1`

lid=1
while read i; do
	val=`sed "s/_(\"$i\")/cond_localize($lid)/" <<<$val`
	lid=$((lid+1))
done<<<"$msgids"

untranslated=`sed -n 's/.*_("\([^"]\+\)").*/\1/p' <<<$val`

if [ "$untranslated" != "" ]; then
	echo "$untranslated">>untranslated.pot.tmp
fi

echo "$val"