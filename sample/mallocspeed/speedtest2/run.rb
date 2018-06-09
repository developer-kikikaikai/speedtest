#!//usr/bin/ruby
require 'bigdecimal'
require 'bigdecimal/util'
require 'json'

#NUM_LIST=[50, 100, 500, 10000];
CONNECTION=2000
NUM_LIST=[256*CONNECTION];
CALL_NUM=5;

PLIST=["normal_run", "own_run", "hhs_run", "tcmalloc_run"];

result_total_hash={}
PLIST.each{ |pname|
	NUM_LIST.each{|num|
		result_hash={}
		for i in 1..CALL_NUM
			result_time=[]
			#separate each line
			key=""
			`./#{pname} #{num}`.each_line {|line|
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
				if result_time.length() == 2 then
					#diff time
					diff = (result_time[1] - result_time[0])
					result_time.push(diff)
					result_hash[key][:val].push(result_time)
					result_hash[key][:total]+=diff
					result_time=[]
				end
			}
		end
	
		result_total_hash["#{pname} #{num}"] = result_hash
	}
}

puts("**Call #{CALL_NUM} times, input is #{NUM_LIST[0]}**")
puts("")
puts("|Action|Total time|")
puts("|:---|:---|")

result_total_hash.each{|num, result_hash|
	result_hash.each{|key, result|
		puts("|#{key[5..key.length()]}|#{result[:total]}|")
	}
#	puts("detail:")
#	puts JSON.pretty_generate(result_hash)
}
