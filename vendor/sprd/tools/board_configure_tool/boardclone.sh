#!/bin/bash

#将file末尾的一些回车符删掉
function clear_file()
{
	declare -i end
	declare -i n
	file=$1
	cd $2
	n=0
	end=0
	file_t="$file""_bak"
	file_n="$file""_bak_b"    #避免屏幕输出
	file_m="$file""_bak_bak"  #computer lines
	sed 's/.$//' $file >$file_m  #删除每行的最后一个字符,可能是"\",引起计算行错误

	cat $file_m | while read  line
	do
		n=n+1
		#echo $line
		#echo ${#line}
		#echo "n="$n
		if [ ${#line} -gt 0 ];then
			end=n
			echo $end >$file_t
		fi
	done
	end=0
	if [[ -f $file_t ]];then
		while read line
		do
	    end=$line
		done < $file_t
		rm $file_t -rf
	fi
	if [[ -f $file_m ]];then
		rm $file_m -rf
	fi
	echo "end="$end
	head -$end $file >$file_n
	mv $file_n $file
}

function add_to_file()
{
	#echo "add_to_file"
	file=$1
	cd $2
	substring=$3
	replacement=$4
	substring_2=0
	var2=$5
	var1=$6
	is_substring_2=0  #is_substring_2用于是否需要替换第二种字符串
	if [ $# -ge 9 ];then
		#echo "$8 $9"
		substring_2=$8
		replacement_2=$9
		is_substring_2=1
	fi
	is_add_tab=0  #是否加tab制表符,0表示不需要,表示从第is_add_tab行后开始tab制表符
	if [ $# -ge 7 ];then
		#echo $7
		is_add_tab=$7
	fi

	declare -i n
	declare -i begin
	declare -i end
	declare -i m
	declare -i k  #k用于计算从哪行开始开始输入tab制表符
	k=0

	#if [ $# -lt 2 ]
	n=0
	begin=0
	file_t="$file""_bak"
	file_n="$file""_bak_b"    #避免屏幕输出
	is_one_line='skip_one_line'


	cat $file | while read  line
	do
		#echo $line
		n=n+1
		echo "$line" |grep -qi "$var2""\>" >$file_n  #字符串向后须全配比
		m=$?
		if [ $begin -gt 0 ];then
			echo "$line" | grep -qi "$var1" >$file_n
			if [ $? -eq 0 ];then
				end=$n
				#echo $begin >$file_t
				echo $end >$file_t
				echo "find"" end=""$end"
				echo $line
				break
			fi
		elif [ $m -eq 0 ];then
			begin=$n
			echo "find"" begin=""$begin"
			echo $line
			echo "$var1" | grep "$is_one_line" >$file_n
			if [ $? -eq 0 ];then
				echo "only add one line"
				line=${line//$substring/$replacement}
				substring_b=`tr '[a-z]' '[A-Z]' <<<"$substring"`
				replacement_b=`tr '[a-z]' '[A-Z]' <<<"$replacement"`
				line=${line//$substring_b/$replacement_b}
				if [ $is_substring_2 -gt 0 ];then
					echo "$line" | grep -i "$substring_2" >$file_n
					if [ $? -eq 0 ];then
						line=${line//$substring_2/$replacement_2}
						substring_b=`tr '[a-z]' '[A-Z]' <<<"$substring_2"`
						replacement_b=`tr '[a-z]' '[A-Z]' <<<"$replacement_2"`
						line=${line//$substring_b/$replacement_b}
					fi
				fi
				sed -e "$n""a\\$line"  $file >$file_t
				cp $file_t $file
				rm $file_t -rf
				break
			fi
		fi
		#echo $n
	done


	end=0
	if [[ -f $file_t ]];then
		while read line
		do
	    end=$line
		done < $file_t
		rm $file_t -rf
	fi

	if [[ -f $file_n ]];then
		rm $file_n -rf
	fi

	echo "find end line is ""$end"
	begin=0
	if [ $end -gt 0 ];then
		cat $file | while read  line
		do
			echo "$line" | grep -qi "$var2""\>" >$file_n
			m=$?
			if [ $begin -gt 0 ];then
				n=n+1
				echo "$line" | grep "$substring" >$file_n
				if [ $? -eq 0 ];then
					#echo have
					line=${line//$substring/$replacement}
					#echo $line
				else
					substring_b=`tr '[a-z]' '[A-Z]' <<<"$substring"`
					replacement_b=`tr '[a-z]' '[A-Z]' <<<"$replacement"`
					line=${line//$substring_b/$replacement_b}
					#echo $line
				fi
				if [ $is_substring_2 -gt 0 ];then
					echo "$line" | grep "$substring_2" >$file_n
					if [ $? -eq 0 ];then
						line=${line//$substring_2/$replacement_2}
						#echo $substring$replacement
						#echo "my log aa "$line
					else
						substring_b=`tr '[a-z]' '[A-Z]' <<<"$substring_2"`
						replacement_b=`tr '[a-z]' '[A-Z]' <<<"$replacement_2"`
						line=${line//$substring_b/$replacement_b}
						#echo "my log bb "$line
					fi
				fi
				k=k+1
				if [ $is_add_tab -gt 0 ];then
					if [ $k -ge $is_add_tab ];then
						line_s="$line_s""\n\t$line"
					else
						line_s="$line_s""\n$line"
					fi
				else
					line_s="$line_s""\n$line"
				fi
				echo "$line" |grep -qi "$var1" >$file_n
				if [ $? -eq 0 ];then
					echo "begin to write to file"
					#echo $end
					#echo $line_s
					sed -e "$end""a\\$line_s"  $file >$file_t
					cp $file_t $file
					rm $file_t -rf
					break
				fi
			elif [ $m -eq 0 ];then
				n=n+1
				begin=$n
				echo "$line" | grep "$substring" >$file_n
				if [ $? -eq 0 ];then
					line=${line//$substring/$replacement}
					#echo log a ....
					echo $substring$replacement
					#echo $line
				else
					substring_b=`tr '[a-z]' '[A-Z]' <<<"$substring"`
					replacement_b=`tr '[a-z]' '[A-Z]' <<<"$replacement"`
					line=${line//$substring_b/$replacement_b}
					#echo log b....
					#echo $line
				fi

				if [ $is_substring_2 -gt 0 ];then
					echo "$line" | grep "$substring_2" >$file_n
					if [ $? -eq 0 ];then
						line=${line//$substring_2/$replacement_2}
						#echo $substring$replacement
						#echo "my log a "$line
					else
						substring_b=`tr '[a-z]' '[A-Z]' <<<"$substring_2"`
						replacement_b=`tr '[a-z]' '[A-Z]' <<<"$replacement_2"`
						line=${line//$substring_b/$replacement_b}
						#echo "my log b "$line
					fi
				fi

				if [ $k -gt $is_add_tab ];then
					line_s="\n\t$line"
				else
					line_s="\n$line"
				fi
				k=k+1
			else
				n=n+1
			fi
		done
	fi

	if [[ -f $file_n ]];then
		rm $file_n -rf
	fi
}

#用此函数,关键字不可以是最后一个串,否则被删掉一个字符,将出现错误
function add_to_file_e()
{
	file=$1
	cd $2
	substring=$3
	replacement=$4
	var2=$5
	var1=$6
	have_second_end_string=0
	only_add_one_line=0

	if [ $# -ge 7 ];then
		temp=$7
		if [ ${#temp} -gt 1 ];then
			#second_end_str所在的行必须紧跟着var1所在的行,
			echo "second_end_str="$7  #有第二结束字符串,第一结束字符串的后面是第二结束字符串,如连续存在第二结束字符串的行,以最后的第二结束字符串结束为准
			second_end_string=$7
			have_second_end_string=1
		fi
	fi
	is_substring_2=0  #is_substring_2用于是否需要替换第二种字符串
	substring_2=0
	if [ $# -ge 9 ];then
		#仅增加单行替换第二种字符串
		#echo "$8 $9"
		substring_2=$8
		replacement_2=$9
		is_substring_2=1
	fi

	declare -i n
	declare -i begin
	declare -i end
	declare -i m

	#if [ $# -lt 2 ]
	n=0
	begin=0
	file_t="$file""_bak"
	file_m="$file""_bak_bak"  #computer lines
	file_n="$file""_bak_b"    #避免屏幕输出
	file_s="$file""_bak_s"    #找到第二结束字符串
	file_o="$file""_bak_o"    #是否只增加一行
	is_one_line='skip_one_line'

	sed 's/.$//' $file >$file_m  #删除每行的最后一个字符,可能是"\",引起计算行错误

	cat $file_m | while read line >$file_n
	do
		#echo $line
		n=n+1
		#echo $n
		echo "$line" |grep -qi "$var2""\>" >$file_n   #字符串向后须全配比
		m=$?
		if [ $begin -gt 0 ];then
			#echo "$line"
			#echo $var1
			echo "$line" |grep -qi "$var1" >$file_n
			if [ $? -eq 0 ];then
				end=$n
				#echo $begin >$file_t
				echo $end >$file_t   #end变量在do...done循环里的值,仅在循环的内部启作用,故保存到一个临时文件
				echo "find"" end=""$end"
				if [ $have_second_end_string -eq 1 ];then
					have_second_end_string=2
					var1=$second_end_string
					echo have_second_end_string
					continue
				elif [ $have_second_end_string -eq 2 ];then
					echo "find second_end_string:"$var1" in $line"
					have_second_end_string=3  #continue,置标志3
					echo $have_second_end_string >$file_s
					continue
				elif [ $have_second_end_string -eq 3 ];then
					echo "find second_end_string:"$var1" in $line"
					continue   #continue,需找到最后一个
				fi
				break
			elif [ $have_second_end_string -eq 2 ];then  #没找到第二结束字符串,以第一结束字符串为准
				echo "not find second_end_string:"$var1
				have_second_end_string=0
				break
			elif [ $have_second_end_string -eq 3 ];then  #不再是第二结束字符串,break
				echo "find second_end_string:"$var1" and break"
				break
			fi
		elif [ $m -eq 0 ];then
			begin=$n
			echo "find"" begin=""$begin"
			echo "$var1" | grep "$is_one_line" >$file_n
			if [ $? -eq 0 ];then
				echo "xxx only add one line"
				end=$n
				echo $end >$file_t
				only_add_one_line=1
				echo $only_add_one_line >$file_o
				break
			fi
		fi
	done


	end=0
	if [[ -f $file_t ]];then
		while read line
		do
	    end=$line
		done < $file_t
		rm $file_t -rf
	fi
	if [[ -f $file_s ]];then
		while read line
		do
	    have_second_end_string=$line
		done < $file_s
		rm $file_s -rf
		if [ $have_second_end_string -eq 3 ];then
			var1=$second_end_string  #以第二结束字符串为准
		fi
	fi
	if [[ -f $file_o ]];then
		while read line
		do
	    only_add_one_line=$line
		done < $file_o
		rm $file_o -rf
	fi

	if [[ -f $file_m ]];then
		rm $file_m -rf
	fi

	if [[ -f $file_n ]];then
		rm $file_n -rf
	fi

	echo "find end line is ""$end"
	begin=0
	if [ $end -gt 0 ];then
		cat $file | while read line
		do
			echo "$line" | grep -qi "$var2""\>" >$file_n   #字符串向后须全配比
			m=$?
			if [ $begin -gt 0 ];then
				n=n+1
				echo "$line" | grep "$substring" >$file_n
				if [ $? -eq 0 ];then
					#echo have
					line=${line//$substring/$replacement}
				else
					substring_b=`tr '[a-z]' '[A-Z]' <<<"$substring"`
					replacement_b=`tr '[a-z]' '[A-Z]' <<<"$replacement"`
					line=${line//$substring_b/$replacement_b}
				fi
				line_s_b=$line_s #备份一下
				line_s="$line_s""\n\t$line"
				echo "$line" |grep -qi "$var1" >$file_n
				if [ $? -eq 0 ];then
					if [ $have_second_end_string -eq 3 ];then
					#继续,需找到最后配比的second_end_string
						have_second_end_string=4
						#echo $line
						continue
					elif [ $have_second_end_string -eq 4 ];then
						#echo $line
						continue
					fi
					echo "begin to write to file"
					#echo $end
					#echo $line_s
					sed -e "$end""a\\$line_s"  $file >$file_t
					cp $file_t $file
					rm $file_t -rf
					break
				elif [ $have_second_end_string -eq 4 ];then
					#echo $line
					echo "second,begin to write to file,end=$end"
					line_s=$line_s_b  #用上一个line_s
					#echo $line_s
					sed -e "$end""a\\$line_s"  $file >$file_t
					cp $file_t $file
					rm $file_t -rf
					break
				fi
			elif [ $m -eq 0 ];then
				n=n+1
				begin=$n
				echo "$line" | grep -i "$substring" >$file_n
				if [ $? -eq 0 ];then
					line=${line//$substring/$replacement}
					substring_b=`tr '[a-z]' '[A-Z]' <<<"$substring"`
					replacement_b=`tr '[a-z]' '[A-Z]' <<<"$replacement"`
					line=${line//$substring_b/$replacement_b}
				fi
				if [ $only_add_one_line = 1 ];then
					#only add one line
					echo xxzzz
					if [ $is_substring_2 -gt 0 ];then
						echo "$line" | grep -i "$substring_2" >$file_n
						if [ $? -eq 0 ];then
							line=${line//$substring_2/$replacement_2}
							substring_b=`tr '[a-z]' '[A-Z]' <<<"$substring_2"`
							replacement_b=`tr '[a-z]' '[A-Z]' <<<"$replacement_2"`
							line=${line//$substring_b/$replacement_b}
						fi
					fi
					sed -e "$end""a\\$line"  $file >$file_t
					cp $file_t $file
					rm $file_t -rf
					break
				fi
				line_s="\n$line"
			else
				n=n+1
			fi
		done
	fi

	if [[ -f $file_n ]];then
		rm $file_n -rf
	fi
}

#用此函数,关键字不可以是最后一个串,否则被删掉一个字符,将出现错误
function add_to_file_n()
{
	file=$1
	cd $2
	substring=$3
	replacement=$4
	var2=$5
	var1=$6
	have_second_end_string=0
	only_add_one_line=0

	if [ $# -ge 7 ];then
		temp=$7
		if [ ${#temp} -gt 1 ];then
			#second_end_str所在的行必须紧跟着var1所在的行,
			echo "second_end_str="$7  #有第二结束字符串,第一结束字符串的后面是第二结束字符串,如连续存在第二结束字符串的行,以最后的第二结束字符串结束为准
			second_end_string=$7
			have_second_end_string=1
		fi
	fi
	is_substring_2=0  #is_substring_2用于是否需要替换第二种字符串
	substring_2=0
	if [ $# -ge 9 ];then
		#仅增加单行替换第二种字符串
		#echo "$8 $9"
		substring_2=$8
		replacement_2=$9
		is_substring_2=1
	fi

	declare -i n
	declare -i begin
	declare -i end
	declare -i m

	#if [ $# -lt 2 ]
	n=0
	begin=0
	file_t="$file""_bak"
	file_m="$file""_bak_bak"  #computer lines
	file_n="$file""_bak_b"    #避免屏幕输出
	file_s="$file""_bak_s"    #找到第二结束字符串
	file_o="$file""_bak_o"    #是否只增加一行
	is_one_line='skip_one_line'

	cp $file $file_m  #删除每行的最后一个字符,可能是"\",引起计算行错误
	
	echo "add_to_file_n"
	echo $var2
	echo $var1
	
	cat $file_m | while read line >$file_n
	do
		#echo $line
		n=n+1
		#echo $n
		echo "$line" |grep -Eqi $var2 >$file_n   #字符串向后须全配比
		m=$?
		if [ $begin -gt 0 ];then
			echo "$line"
			echo $var1
			echo "$line" |grep -qi "$var1" >$file_n
			if [ $? -eq 0 ];then
				end=$n
				#echo $begin >$file_t
				echo $end >$file_t   #end变量在do...done循环里的值,仅在循环的内部启作用,故保存到一个临时文件
				echo "find"" end=""$end"
				if [ $have_second_end_string -eq 1 ];then
					have_second_end_string=2
					var1=$second_end_string
					echo have_second_end_string
					continue
				elif [ $have_second_end_string -eq 2 ];then
					echo "find second_end_string:"$var1" in $line"
					have_second_end_string=3  #continue,置标志3
					echo $have_second_end_string >$file_s
					continue
				elif [ $have_second_end_string -eq 3 ];then
					echo "find second_end_string:"$var1" in $line"
					continue   #continue,需找到最后一个
				fi
				break
			elif [ $have_second_end_string -eq 2 ];then  #没找到第二结束字符串,以第一结束字符串为准
				echo "not find second_end_string:"$var1
				have_second_end_string=0
				break
			elif [ $have_second_end_string -eq 3 ];then  #不再是第二结束字符串,break
				echo "find second_end_string:"$var1" and break"
				break
			fi
		elif [ $m -eq 0 ];then
			begin=$n
			echo "find"" begin=""$begin"
			echo "$var1" | grep "$is_one_line" >$file_n
			if [ $? -eq 0 ];then
				echo "xxx only add one line"
				end=$n
				echo $end >$file_t
				only_add_one_line=1
				echo $only_add_one_line >$file_o
				break
			fi
		fi
	done


	end=0
	if [[ -f $file_t ]];then
		while read line
		do
	    end=$line
		done < $file_t
		rm $file_t -rf
	fi
	if [[ -f $file_s ]];then
		while read line
		do
	    have_second_end_string=$line
		done < $file_s
		rm $file_s -rf
		if [ $have_second_end_string -eq 3 ];then
			var1=$second_end_string  #以第二结束字符串为准
		fi
	fi
	if [[ -f $file_o ]];then
		while read line
		do
	    only_add_one_line=$line
		done < $file_o
		rm $file_o -rf
	fi

	if [[ -f $file_m ]];then
		rm $file_m -rf
	fi

	if [[ -f $file_n ]];then
		rm $file_n -rf
	fi

	echo "find end line is ""$end"
	begin=0
	if [ $end -gt 0 ];then
		cat $file | while read line
		do
			echo "$line" | grep -Eqi "$var2" >$file_n   #字符串向后须全配比
			m=$?
			if [ $begin -gt 0 ];then
				n=n+1
				echo "$line" | grep "$substring" >$file_n
				if [ $? -eq 0 ];then
					#echo have
					line=${line//$substring/$replacement}
				else
					substring_b=`tr '[a-z]' '[A-Z]' <<<"$substring"`
					replacement_b=`tr '[a-z]' '[A-Z]' <<<"$replacement"`
					line=${line//$substring_b/$replacement_b}
				fi
				line_s_b=$line_s #备份一下
				line_s="$line_s""\n\t$line"
				echo "$line" |grep -qi "$var1" >$file_n
				if [ $? -eq 0 ];then
					if [ $have_second_end_string -eq 3 ];then
					#继续,需找到最后配比的second_end_string
						have_second_end_string=4
						#echo $line
						continue
					elif [ $have_second_end_string -eq 4 ];then
						#echo $line
						continue
					fi
					echo "begin to write to file"
					#echo $end
					#echo $line_s
					sed -e "$end""a\\$line_s"  $file >$file_t
					cp $file_t $file
					rm $file_t -rf
					break
				elif [ $have_second_end_string -eq 4 ];then
					#echo $line
					echo "second,begin to write to file,end=$end"
					line_s=$line_s_b  #用上一个line_s
					#echo $line_s
					sed -e "$end""a\\$line_s"  $file >$file_t
					cp $file_t $file
					rm $file_t -rf
					break
				fi
			elif [ $m -eq 0 ];then
				n=n+1
				begin=$n
				echo "$line" | grep -i "$substring" >$file_n
				if [ $? -eq 0 ];then
					line=${line//$substring/$replacement}
					substring_b=`tr '[a-z]' '[A-Z]' <<<"$substring"`
					replacement_b=`tr '[a-z]' '[A-Z]' <<<"$replacement"`
					line=${line//$substring_b/$replacement_b}
				fi
				if [ $only_add_one_line = 1 ];then
					#only add one line
					echo xxzzz
					if [ $is_substring_2 -gt 0 ];then
						echo "$line" | grep -i "$substring_2" >$file_n
						if [ $? -eq 0 ];then
							line=${line//$substring_2/$replacement_2}
							substring_b=`tr '[a-z]' '[A-Z]' <<<"$substring_2"`
							replacement_b=`tr '[a-z]' '[A-Z]' <<<"$replacement_2"`
							line=${line//$substring_b/$replacement_b}
						fi
					fi
					sed -e "$end""a\\$line"  $file >$file_t
					cp $file_t $file
					rm $file_t -rf
					break
				fi
				line_s="\n$line"
			else
				n=n+1
			fi
		done
	fi

	if [[ -f $file_n ]];then
		rm $file_n -rf
	fi
}

function board_for_kernel()
{
	USE_INPUT_BOARD_MACRO=0
	have_dt_board=1

	echo -e "\n\nconfigure kernel begin......"
	#---------产生*dts文件 begin-------
	cd $workdir
	cd "kernel/arch/arm64/boot/dts"
	#cheng.fang mark
        #file="sprd-""$PLATFORM""_""$KCONFIG_BOARD_NAME_R"".dts"
	file=$DTS_NAME_REF".dts"
	#echo $file
	if [  -f $file ];then
		TEMP_1="$BOARD_NAME_N"".dts"
		cp $file $TEMP_1
	fi
	#---------产生*dts文件 end-------
	
	#---------修改kernel/arch/arm/boot/dts/Makefile begin-------
	#echo $have_dt_board
	echo "modify kernel/arch/arm64/boot/dts/Makefile"
	cd $workdir
	file=Makefile
	replacement=$BOARD_NAME_N".dtb\n	"$DTS_NAME_REF".dtb"
	echo "replacement:"$replacement
	path="kernel/arch/arm64/boot/dts"
	echo $path
	end_string='skip_one_line'
	substring=$DTS_NAME_REF".dtb"
	begin_string=`tr '[a-z]' '[A-Z]' <<<"dtb-"`
	sed -i s/$substring/"$replacement"/ ./$path/Makefile
	sed -i s/$BOARD_NAME_N".dtb"/$BOARD_NAME_N".dtb \\\\"/ ./$path/Makefile
	#add_to_file_e $file $path $substring $replacement $begin_string $end_string
	#---------修改kernel/arch/arm/boot/dts/Makefile end-------

	#---------增加'|| defined(CONFIG_MACH_BOARD_NAME_N)' begin-------
	cd $workdir
	cd kernel/
	BOARD_NAME_R_b=`tr '[a-z]' '[A-Z]' <<<"$KCONFIG_BOARD_NAME_R"`
	BOARD_NAME_N_b=`tr '[a-z]' '[A-Z]' <<<"$BOARD_NAME_N"`
	substring="(CONFIG_MACH_$BOARD_NAME_R_b)"
	addstring=" || defined(CONFIG_MACH_$BOARD_NAME_N_b)"
	replacestring="$substring$addstring"
	echo -e $substring
	echo -e $addstring
	echo -e $replacestring
	#if [ `grep -Hrn  $substring ./` ];then
	#echo ok
	#fi
	if [ -z "`grep -Hrn --exclude-dir=".git" --include="*.c" --include="*.h" $substring ./`" ]
	then
		echo "NULL"
	else
		#sed -i "/$substring/s/$/$addstring/;" `grep $substring -rl --include="*.*" ./`
		#sed -i s/$TEMP1/$TEMP2/g `grep $TEMP1 -rl --include="*.*" ./`
		sed -i s/"$substring"/"$replacestring"/g `grep $substring -rl --include="*.*" ./`
		echo "===========================notice========================================"
		echo "===========================notice========================================"
		echo "maybe add \""$addstring"\" words!"
		echo "please check that if need!!!!!"
	fi

  #---------增加'|| (defined CONFIG_MACH_BOARD_NAME_N)' begin-------
	substring1="#ifdef\s\+CONFIG_MACH_$BOARD_NAME_R_b"
	addstring1='#ifdef\tCONFIG_MACH_'$BOARD_NAME_N_b'\r\n#include\t\"__board-'$BOARD_NAME_N.h'\"\t\r\n#endif\r\n\r\n'
	replacestring1=$addstring1'#ifdef\tCONFIG_MACH_'$BOARD_NAME_R_b
	echo -e $substring1
	echo -e $addstring1
	echo -e $replacestring1
	#if [ `grep -Hrn  $substring ./` ];then
	#echo ok
	#fi
	#if [ -z "`grep -Ei $substring1 ./include/soc/sprd/board.h`" ]
	#then
	#	echo "NULL"
	#else
		#sed -i "/$substring/s/$/$addstring/;" `grep $substring -rl --include="*.*" ./`
		#sed -i s/$TEMP1/$TEMP2/g `grep $TEMP1 -rl --include="*.*" ./`
		sed -i 's/'$substring1'/'$replacestring1'/' ./include/soc/sprd/board.h
		echo "===========================notice========================================"
		echo "===========================notice========================================"
		echo "maybe add \""$addstring1"\" words!"
		echo "please check that if need!!!!!"
	#fi
	#---------增加'|| defined(CONFIG_MACH_BOARD_NAME_N)' end-------

	echo "configure kernel end!"
}

function board_for_uboot()
{
	#echo $BOARD_NAME_N

	PATH_R="u-boot15/board/spreadtrum/""$UCONFIG_BOARD_NAME_R"
	PATH_N="u-boot15/board/spreadtrum/""$BOARD_NAME_N"

	echo -e "\n\nconfigure u-boot begin......"

	while [ 1 ]
	do
		if [ ! -d $PATH_R ];then
			echo "$PATH_R not exist,configure u-boot fail!"
			break
		fi

		if [  -d $PATH_N ];then
			echo "$PATH_N exist,will rm it"
			rm $PATH_N -rf
		fi
		mkdir $PATH_N
		cp $PATH_R/* $PATH_N/ -rf

		PATH_M="u-boot15/include/configs/"
		file_r="$PATH_M""$UCONFIG_BOARD_NAME_R"".h"
		file_n="$PATH_M""$BOARD_NAME_N"".h"

		if [ ! -f $file_r ];then
			echo "$file_r not exist,configure u-boot fail!"
			break
		fi

		cp $file_r $file_n

		BOARD_NAME_R_b=`tr '[a-z]' '[A-Z]' <<<"$UCONFIG_BOARD_NAME_R"`
		#echo $BOARD_NAME_R_b
		BOARD_NAME_N_b=`tr '[a-z]' '[A-Z]' <<<"$BOARD_NAME_N"`
		#sed -i s/$BOARD_NAME_R_b/$BOARD_NAME_N_b/g `grep $BOARD_NAME_N_b $file_n`
		echo $BOARD_NAME_N
		echo $PATH_M
		#sed -i s/$BOARD_NAME_R_b/$BOARD_NAME_N_b/g `grep $BOARD_NAME_R_b -rl --include="$UCONFIG_BOARD_NAME_N"".h" $PATH_M`
		cd $PATH_N
		mv "pinmap-"$UCONFIG_BOARD_NAME_R".c" "pinmap-"$BOARD_NAME_N".c"
		mv $UCONFIG_BOARD_NAME_R".c" $BOARD_NAME_N".c"
		sed -i s/$UCONFIG_BOARD_NAME_R/$BOARD_NAME_N/g ./*
		sed -i s/$BOARD_NAME_R_b/$BOARD_NAME_N_b/g ./*

	#---------修改boards.cfg文件 begin-------
	#	cd $workdir
	#	file=boards.cfg
	#	path="./u-boot64"
	#	substring="$UCONFIG_BOARD_NAME_R"
	#	replacement="$BOARD_NAME_N"
	#	begin_string='Active\s+arm\s+armv7\s+sc8830\s+spreadtrum\s+'$UCONFIG_BOARD_NAME_R
	#	end_string='-'
	#	add_to_file_n $file $path $substring $replacement $begin_string $end_string
	#---------修改文件 end-------
		cd $workdir
		cd "u-boot15/configs"

		#clone dt_defconfig
		TEMP_=$UCONFIG_NAME_REF"_defconfig"
		#echo "xxx"$TEMP_
		TEMP_1=$BOARD_NAME_N"_defconfig"
		if [  -f $TEMP_ ];then
			cp $TEMP_ $TEMP_1
		fi
		echo $BOARD_NAME_R_b
		sed -i s/$BOARD_NAME_R_b/$BOARD_NAME_N_b/g ./$TEMP_1

	#---------修改Makefile文件 begin-------
		cd $workdir
		file="Kconfig"
		substring="endchoice"
		begin_string=`tr '[a-z]' '[A-Z]' <<<"$BOARD_NAME_N"`
		begin_string='config TARGET_'$begin_string
		replacement=$begin_string"\n\tbool \"Spreadtrum "$BOARD_NAME_N"\"\n\tselect ARM64\nendchoice"
		path="./u-boot15/board/spreadtrum"
		sed -i s/$substring/"$replacement"/g $path/$file
		substring1="endif"
		replacement1="source\ \"board\/spreadtrum\/"$BOARD_NAME_N"\/Kconfig\"\nendif"
		echo $replacement1
		sed -i s/$substring1/"$replacement1"/g $path/$file
	#---------修改文件 end-------


	break
	done

	echo "configure u-boot OK!"
}

function board_for_chipram()
{
	DEBUG_S=debug
	if [ $# -gt 0 ];then
		echo "$1" | grep "$DEBUG_S"
		if [ $? -eq 0 ];then
			PLATFORM=scx15
			BOARD_NAME_R=sp7715ea
			BOARD_NAME_N=sp7715eaopenphone
			workdir=$PWD
		fi
	fi

	#echo $BOARD_NAME_N

	echo -e "\n\nconfigure chipram begin......"

	while [ 1 ]
	do
		PATH_ROOT="chipram"
		if [ ! -d $PATH_ROOT ];then
			echo "$PATH_ROOT not exist,configure chipram fail!"
			break
		fi

		PATH_M="chipram/include/configs/"
		file_r="$PATH_M""$CCONFIG_NAME_REF"".h"
		file_n="$PATH_M""$BOARD_NAME_N"".h"

		if [ ! -f $file_r ];then
			echo "$file_r not exist,configure chipram fail!"
			break
		fi

		cp $file_r $file_n

		BOARD_NAME_R_b=`tr '[a-z]' '[A-Z]' <<<"$CCONFIG_BOARD_NAME_R"`
		#echo $BOARD_NAME_R_b
		BOARD_NAME_N_b=`tr '[a-z]' '[A-Z]' <<<"$BOARD_NAME_N"`
		#sed -i s/$BOARD_NAME_R_b/$BOARD_NAME_N_b/g `grep $BOARD_NAME_N_b $file_n`
		#echo $BOARD_NAME_N
		#echo $PATH_M
		sed -i s/$BOARD_NAME_R_b/$BOARD_NAME_N_b/g `grep $BOARD_NAME_R_b -rl --include="$BOARD_NAME_N"".h" $PATH_M`


	#---------修改Makefile文件 begin-------
		cd $workdir
		file=Makefile
		substring="$CCONFIG_BOARD_NAME_R"
		begin_string=`tr '[a-z]' '[A-Z]' <<<"$CCONFIG_BOARD_NAME_R"`
		begin_string="$begin_string""_config"
		replacement="$BOARD_NAME_N"
		path="./chipram"
		end_string='armv7'
		s_e_end_string='(obj)include'
		add_to_file_e $file $path $substring $replacement $begin_string $end_string $s_e_end_string
	#---------修改文件 end-------


	#---------增加'|| defined(CONFIG_BOARD_NAME_N)' begin-------
		cd $workdir
		cd chipram/
		BOARD_NAME_R_b=`tr '[a-z]' '[A-Z]' <<<"$CCONFIG_BOARD_NAME_R"`
		BOARD_NAME_N_b=`tr '[a-z]' '[A-Z]' <<<"$BOARD_NAME_N"`
		substring="(CONFIG_$BOARD_NAME_R_b)"
		addstring=" || defined(CONFIG_$BOARD_NAME_N_b)"
		replacestring=$substring$addstring
		#echo -e $substring
		#echo -e $addstring
		if [ -z "`grep -Hrn --exclude-dir=".git" --include="*.c" --include="*.h" $substring ./`" ]
		then
			echo "NULL"
		else
			#sed -i "/$substring/s/$/$addstring/;" `grep $substring -rl --include="*.*" ./`
			sed -i s/"$substring"/"$replacestring"/g `grep $substring -rl --include="*.*" ./`
			echo "===========================notice========================================"
			echo "===========================notice========================================"
			echo "maybe add \"|| defined(CONFIG_$BOARD_NAME_N_b)\" words!"
			echo "please check that if need!!!!!"
		fi
	#---------增加'|| defined(CONFIG_BOARD_NAME_N)' end-------

	break
	done

	echo "configure chipram OK!"
}

function do_recursive_rename()
{
	for file in `ls $1`
	do
		if [ -d $1"/"$file ]
		then
			do_recursive_rename $1"/"$file
		else
			#echo $filename
			newname=`echo $1"/"$file | sed -n "s/$BOARD_NAME_R/$BOARD_NAME_N/p"`
			#echo "n="$newname
			mv $1"/"$file $newname
		fi
	done
	return 0
}

function board_for_device()
{
	DEBUG_S=debug
	if [ $# -gt 0 ];then
		echo "$1" | grep "$DEBUG_S"
		if [ $? -eq 0 ];then
			PLATFORM=scx15
			BOARD_NAME_R=sp7715ea
			BOARD_NAME_N=sp7715eaopenphone
			workdir=$PWD
		fi
	fi

	export PATH_R="device/sprd/$PLATFORM/$BOARD_NAME_R"

	#echo $PATH_R

	if [ ! -d $PATH_R ];then
		echo "reference board not exist,please check againxxx!"
		exit 0
	fi

	echo "configure device begin......"

	PATH_N="device/sprd/$PLATFORM/$BOARD_NAME_N"
	if  [ -d $PATH_N ];then
		rm $PATH_N -rf
	fi

	mkdir $PATH_N

	cp $PATH_R/* $PATH_N/ -rf

	cd $PATH_N

	
	mv $PROJECT_REF".mk" $PROJECT_NEW".mk"
#change DTS
	sed -i 's/TARGET_DTB[[:space:]?]:=[[:space:]?].*/DTS_DEFCONFIG := '$BOARD_NAME_N'/g' $PROJECT_NEW".mk"
#change Kernel
	sed -i 's/KERNEL_DEFCONFIG[[:space:]?]:=[[:space:]?].*/KERNEL_DEFCONFIG := '$BOARD_NAME_N'_dt_defconfig/g' $PROJECT_NEW".mk"
#change uboot
	sed -i 's/UBOOT_DEFCONFIG[[:space:]?]:=[[:space:]?].*/UBOOT_DEFCONFIG := '$BOARD_NAME_N'/g' $PROJECT_NEW".mk"
#change chipram
	sed -i 's/CHIPRAM_DEFCONFIG[[:space:]?]:=[[:space:]?].*/CHIPRAM_DEFCONFIG := '$BOARD_NAME_N'/g' $PROJECT_NEW".mk"

	do_recursive_rename "."
	mv "module_"$DTS_NAME_REF "module_sprd-"$PLATFORM"_"$BOARD_NAME_N

	#for filename in *$BOARD_NAME_R*
	#do
	#echo $filename
	#newname=`echo $filename | sed -n "s/$BOARD_NAME_R/$BOARD_NAME_N/p"`
	#echo "n="$newname
	#mv $filename $newname
	#done


	#---------替换board名关键字begin----------------
	sed -i s/$PROJECT_REF/$PROJECT_NEW/g `grep $PROJECT_REF -rl --include="*.*" ./`
	sed -i s/$BOARD_NAME_R/$BOARD_NAME_N/g `grep $BOARD_NAME_R -rl --include="*.*" ./`

	TEMP=$BOARD_NAME_R
	TEMP1=`tr '[a-z]' '[A-Z]' <<<"$TEMP"`

	TEMP=$BOARD_NAME_N
	TEMP2=`tr '[a-z]' '[A-Z]' <<<"$TEMP"`

	sed -i s/$TEMP1/$TEMP2/g `grep $TEMP1 -rl --include="*.*" ./`
	#---------替换board名关键字end--------------------

	sed -i /$BOARD_NAME_N/d AndroidProducts.mk
        echo "PRODUCT_MAKEFILES += \$(LOCAL_DIR)/"$PROJECT_NEW".mk" >>AndroidProducts.mk

	#---------删除vendorsetup.sh AndroidProducts中其他衍生的board-------
	sed -i '/add_lunch_combo/d' vendorsetup.sh
	echo "add_lunch_combo $PROJECT_NEW-userdebug" >>vendorsetup.sh

	#---------删除AndroidProducts最后的\字符begin-------
	clear_file vendorsetup.sh .
	TEMP=`tail -1 $file`
	TEMP1='\'
	TEMP2=${#TEMP}-1
	TEMP3=${TEMP:$TEMP2:1}

	if [[ $TEMP3 == '\' ]];then
		#echo ''\' is del'
		sed -i '$s/.$//' $file
		TEMP2=${#TEMP}-2
		TEMP3=${TEMP:$TEMP2:1}
		if [[ $TEMP3 == ' ' ]];then
		sed -i '$s/.$//' $file
		fi
	fi
	#---------删除AndroidProducts最后的\字符end-------

	#---------增加vendor/sprd/open-source/res/productinfo目录下的ini文件begin-------
	cd $workdir
	cd "vendor/sprd/open-source/res/productinfo"

	TEMP1=$PLATFORM"_""$BOARD_NAME_R""plus""_connectivity_configure.ini"
	TEMP2=$PLATFORM"_""$BOARD_NAME_N""plus""_connectivity_configure.ini"
	if [ -f $TEMP1 ];then
		cp $TEMP1 $TEMP2
	fi
	TEMP1=$PLATFORM"_""$BOARD_NAME_R""base""_connectivity_configure.ini"
	TEMP2=$PLATFORM"_""$BOARD_NAME_N""base""_connectivity_configure.ini"
	if [ -f $TEMP1 ];then
		cp $TEMP1 $TEMP2
	fi

	#---------增加vendor/sprd/open-source/res/productinfo目录下的ini文件end-------
	echo "configure device end!"
}

function print_help()
{
  echo "-r refernce_board_platform"
  echo "-u chipram_uboot_config"
  echo "-k kenerl_defconfig_name"
  echo "-p refernce_project"
  echo "-n new_board_name"
  echo "-d dts_name"
  echo "-o new_project_name"
}

#程序的入口 is here
#cd ../../../../..
workdir=$PWD
echo $workdir
declare -i g_count

if [ ! -d "./kernel" ];then
	echo "board_clone.sh maybe is not in correct dir,please put it to android top dir!"
	exit 0
fi

if [ ! -d "./u-boot15" ];then
	echo "board_clone.sh maybe is not in correct dir,please put it to android top dir!!"
	exit 0
fi

if [ ! -d "./device/sprd" ];then
	echo "board_clone.sh maybe is not in correct dir,please put it to android top dir!!!"
	exit 0
fi

CONFIG_NAME_REF=""

while getopts "r:n:d:k:p:o:u:" OPT;
do
 case "$OPT" in
   "r")
	BOARD_REF=$OPTARG
	;;
   "u")
	CONFIG_NAME_REF=$OPTARG
	;;
   "p")
	PROJECT_REF=$OPTARG
	;;
   "d")
	DTS_NAME_REF=$OPTARG
	;;
   "k")
	KCONFIG_NAME_REF=$OPTARG
	;;
   "n")
	BOARD_NEW=$OPTARG
        ;;
   "o")
	PROJECT_NEW=$OPTARG
        ;;
   "*")
      echo "Usage:`boardmaker.sh $0`[options] filename";;
 esac
done

#parameter processing
#BOARD_REF=$1
echo "$BOARD_REF"

#BOARD_NEW=$2
echo "$BOARD_NEW"

PLATFORM=`echo "$BOARD_REF"|awk -F '_' '{print $1}'`
BOARD_NAME_R=${BOARD_REF#$PLATFORM'_'}
echo "------------------------------------------"
echo "Reference Platform: $PLATFORM"
echo "Reference Board	: $BOARD_NAME_R"
echo "------------------------------------------"

BOARD_NAME_N=${BOARD_NEW#$PLATFORM'_'}
echo "New Board name	: $BOARD_NAME_N"
echo "------------------------------------------"

#需转换成小写,如此输入大小写都可
BOARD_NAME_R=`tr '[A-Z]' '[a-z]' <<<"$BOARD_NAME_R"`
BOARD_NAME_N=`tr '[A-Z]' '[a-z]' <<<"$BOARD_NAME_N"`

export PATH_R="device/sprd/$PLATFORM/$BOARD_NAME_R"

echo $PATH_R

if [ ! -d $PATH_R ];then
	echo "$PATH_R not exist!"
	echo "reference board not exist,please check again!"
	exit 0
fi

#解析工程mk
CCONFIG_NAME_REF=`grep "CHIPRAM_DEFCONFIG[[:space:]?]:=[[:space:]?]" $PATH_R/$PROJECT_REF".mk"`
CCONFIG_NAME_REF=${CCONFIG_NAME_REF#"CHIPRAM_DEFCONFIG := "}
echo "CCONFIG_NAME_REF:"$CCONFIG_NAME_REF

UCONFIG_NAME_REF=`grep "UBOOT_DEFCONFIG[[:space:]?]:=[[:space:]?]" $PATH_R/$PROJECT_REF".mk"`
UCONFIG_NAME_REF=${UCONFIG_NAME_REF#"UBOOT_DEFCONFIG := "}
echo "UCONFIG_NAME_REF:"$UCONFIG_NAME_REF

KCONFIG_NAME_REF=`grep "KERNEL_DEFCONFIG[[:space:]?]:=[[:space:]?]" $PATH_R/$PROJECT_REF".mk"`
KCONFIG_NAME_REF=${KCONFIG_NAME_REF#"KERNEL_DEFCONFIG := "}
echo "KCONFIG_NAME_REF:"$KCONFIG_NAME_REF

DTS_NAME_REF=`grep "TARGET_DTB[[:space:]?]:=[[:space:]?]" $PATH_R/$PROJECT_REF".mk"`
DTS_NAME_REF=${DTS_NAME_REF#"TARGET_DTB := "}
echo "DTS_NAME_REF:"$DTS_NAME_REF

#因命名规则不统一建立选择表兼容旧的命名格式,设置默认
CCONFIG_BOARD_NAME_R=$CCONFIG_NAME_REF
echo "CCONFIG_BOARD_NAME_R:"$CCONFIG_BOARD_NAME_R

UCONFIG_BOARD_NAME_R=$UCONFIG_NAME_REF
echo "UCONFIG_BOARD_NAME_R:"$UCONFIG_BOARD_NAME_R

KCONFIG_BOARD_NAME_R=${KCONFIG_NAME_REF%?dt_defconfig}
echo $KCONFIG_BOARD_NAME_R
#CONFIG_BOARD_NAME_R=`echo "$BOARD_NAME_R"|awk -F '_' '{print $1}'`
#echo $CONFIG_BOARD_NAME_R
#case $CONFIG_BOARD_NAME_R in
#	sp7731g)
#		CONFIG_BOARD_NAME_R="sp7731gea"
#		echo $CONFIG_BOARD_NAME_R
#esac

#因命名规则不统一建立选择表设置默认REF工程

cd $workdir
#board_for_device

cd $workdir
#board_for_kernel

cd $workdir
board_for_uboot

cd $workdir
#board_for_chipram
