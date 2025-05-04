#! /bin/bash

declare -A lcs 2>/dev/null
if [ $? != 0 ]; then
	echo "generate_code.sh: WARNING: bash required" >&2
	if ! type bash 2>&1 >/dev/null; then
		echo "generate_code.sh: ERROR: no bash found" >&2
		echo "Please, install bash." >&2
		exit 1
	fi
	echo "generate_code.sh: Invoking bash" >&2
	exec bash $0
fi

SED=sed
if ! [[ `sed --version 2>&1` =~ "GNU" ]]; then
	SED=gsed
	if ! [[ `gsed --version 2>&1` =~ "GNU" ]]; then
		echo "generate_code.sh: GNU version of sed is required">&2
		echo "Please, install gsed." >&2
		exit 2
	fi
fi

msgids=`${SED} -n 's/msgid "\(.\+\)"/\1/p' Localizations/base.pot`
locale_files=`ls Localizations/*.po`

while IFS= read i; do
	for fn in $locale_files; do
		msgid=`${SED} "s?/?\\\\\\\\/?g" <<<$i`
		v=`${SED} -nz "s/.*msgid \"$msgid\"\\nmsgstr \"\\(\\(\\\\\\\\\"\\|[^\"]\\)\\+\\)\".*/\\1/p" ${fn}`
		lcs["$fn"]="${lcs[$fn]}\tCFSTR(\"$v\"),\n"
	done
done<<< "$msgids"


for i in $locale_files; do
	localize_code="${localize_code}${lcs[${i}]}"
done
#localize_code="${localize_code}"
echo "#include <CoreFoundation/CFString.h>"
echo
echo "CFStringRef localization_arr[]={"
echo -e "$localize_code"
echo -e "};\n\nint cond_localize_cnt=$(wc -l<<<$msgids);\nint cond_localize_language_cnt=`wc -l<<<$locale_files`;\\n"
