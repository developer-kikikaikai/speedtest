#!//usr/bin/ruby
require 'bigdecimal'
require 'bigdecimal/util'
require 'json'

#NUM_LIST=[50, 100, 500, 10000];
NUM_LIST=[50, 100, 500, 1024, 10000];
CALL_NUM=10;

PNAME="mallocspeed"

result_total_hash={}
NUM_LIST.each{|num|
	result_hash={}
	for i in 1..CALL_NUM
		result_time=[]
		#separate each line
		key=""
		`./#{PNAME} #{num}`.each_line {|line|
			time_and_word=line.split(",")
			if time_and_word[1].start_with?("CALL:") then
				key=time_and_word[1].chomp
				if ! result_hash.has_key?(key) then
					result_hash[key]={}
					result_hash[key][:val]=[]
					result_hash[key][:total]=0.to_d
				end
			end
			result_time.push(time_and_word[0].to_d)
			if result_time.length() == 3 then
				#diff time
				diff = (result_time[2] - result_time[0])
				result_time.push(diff)
				result_hash[key][:val].push(result_time)
				result_hash[key][:total]+=diff
				result_time=[]
			end
		}
	end

	result_total_hash[num] = result_hash
}

result_total_hash.each{|num, result_hash|
	puts("**Call #{PNAME} #{CALL_NUM} times, input is #{num}**")
	puts("")
	puts("|Action|Total time|")
	puts("|:---|:---|")
	result_hash.each{|key, result|
		puts("|#{key[5..key.length()]}|#{result[:total]}|")
	}
	puts("")
#	puts("detail:")
#	puts JSON.pretty_generate(result_hash)
}
