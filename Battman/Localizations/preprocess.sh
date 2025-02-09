#! /bin/bash

msgids=`sed -n 's/msgid "\(.\+\)"/\1/p' Localizations/base.pot`
locale_files=`ls Localizations/*.po`

val=`cat $1`

lid=1
while read i; do
	val=`sed "s/_ID_(\"$i\")/(const char *)$lid/g;s/_(\"$i\")/cond_localize($lid)/g;s/\\([,({: 	]\\)_(\\([^\"]\\+\\))/\\1cond_localize((int)(unsigned long long)\\2)/g" <<<$val`
	lid=$((lid+1))
done<<<"$msgids"
if [ "$1" == "main.m" ]; then
	#echo sed "s^return nil; // !COND_LOCALIZE_CODE!^$(./Localizations/generate_code.sh)^"
	#val=`sed "s^return nil; // !COND_LOCALIZE_CODE!^$(./Localizations/generate_code.sh)return nil;^" <<<$val`
	val=`sed "s^// !LOCALIZATION_ARR_CODE!^$(./Localizations/generate_code.sh)^" <<<$val`
fi

echo "$val"