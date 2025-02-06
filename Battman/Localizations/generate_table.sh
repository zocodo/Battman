#! /bin/bash

msgids=`sed -n 's/msgid "\(.\+\)"/\1/p' base.pot`
locale_files=`ls *.po`
declare -A lcs

lid=1

while read i; do
	for fn in $locale_files; do
		v=`sed -nz "s/.*msgid \"$i\"\\nmsgstr \"\\([^\"]\\+\\)\".*/\\1/p" ${fn}`
		lcs["$fn"]="${lcs[$fn]}if(localize_id==${lid}) {\nreturn @\"$v\";\n}\n"
	done
	lid=$((lid+1))
done<<< "$msgids"

lid=0
for i in $locale_files; do
	localize_code="${localize_code}if(preferred_language==${lid}) {\n${lcs[${i}]}\n}\n"
	lid=$((lid+1))
done

printf "$localize_code"