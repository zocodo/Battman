#! /bin/bash

SED=sed

msgids=`${SED} -n 's/msgid "\(.\+\)"/\1/p' Localizations/base.pot`
locale_files=`ls Localizations/*.po`
declare -A lcs

while read i; do
	for fn in $locale_files; do
		v=`${SED} -nz "s/.*msgid \"$i\"\\nmsgstr \"\\([^\"]\\+\\)\".*/\\1/p" ${fn}`
		lcs["$fn"]="${lcs[$fn]}\\tCFSTR(\"$v\"),\\n"
	done
done<<< "$msgids"


for i in $locale_files; do
	localize_code="${localize_code}${lcs[${i}]}"
done
#localize_code="${localize_code}"
echo "#include <CoreFoundation/CFString.h>"
echo
echo "CFStringRef localization_arr[]={"
printf "$localize_code"
printf "};\\n\\nint cond_localize_cnt=$(wc -l<<<$msgids);\\nint cond_localize_language_cnt=`wc -l<<<$locale_files`;\\n"
