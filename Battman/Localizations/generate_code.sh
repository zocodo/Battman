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

function read_po() {
	declare -A rpo_ret
	local current_msgid=""
	local current_msgstr=""
	local in_msgid=1
	while IFS= read i; do
		if [ "$i" == "" ]; then
			if [ "$current_msgid" != "" ]; then
				rpo_ret["$current_msgid"]="$current_msgstr"
			fi
			current_msgid=""
			current_msgstr=""
			in_msgid=1
			continue
		fi
		if [[ "$i" =~ ^# ]]; then
			continue
		fi
		if [[ "$i" =~ ^msgid\ \" ]]; then
			if [ "$current_msgid" != "" ]; then
				echo Duplicated msgid definition in $1 >&2
				echo Aborted >&2
				exit 10
			fi
			current_msgid="${i:7:-1}"
			continue
		fi
		if [[ "$i" =~ ^msgstr\ \" ]]; then
			if [ "$current_msgstr" != "" ]; then
				echo Duplicated msgstr definition in $1 >&2
				echo Aborted >&2
				exit 10
			fi
			current_msgstr="${i:8:-1}"
			in_msgid=0
			continue
		fi
		if [[ "$i" =~ ^\" ]]; then
			if [ $in_msgid == 1 ]; then
				current_msgid="${current_msgid}${i:1:-1}"
			else
				current_msgstr="${current_msgstr}${i:1:-1}"
			fi
		fi
	done<<<`cat $1`
	echo "${rpo_ret[@]@K}"
}


declare -A lkeys="(`read_po ./Localizations/base.pot`)"
locale_files=`ls Localizations/*.po`
for fn in $locale_files; do
	declare -A cur="(`read_po ${fn}`)"
	for i in "${!lkeys[@]}"; do
		curval="${cur[$i]//\"/\\\"}"
		lcs["$fn"]="${lcs[$fn]}\t{(\"$curval\"),CFSTR(\"$curval\")},\n"
	done
done
for i in $locale_files; do
	localize_code="${localize_code}${lcs[${i}]}"
done
echo "#include <CoreFoundation/CFString.h>"
echo
echo "struct localization_arr_entry{const char *pstr;CFStringRef cfstr;};"
echo
echo "struct localization_arr_entry localization_arr[]={"
echo -e "$localize_code"
echo -e "};\n\nint cond_localize_cnt=${#lkeys[@]};\nint cond_localize_language_cnt=`wc -l<<<$locale_files`;\\n"
