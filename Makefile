# compilation flags: warnings, force standard gnu11, optimization, stop out of memory access
CFLAGS += -Wall -Werror -std=gnu11 -O2 # -fsanitize=address
# linking flags link math library 
LDFLAGS += -lm 

# to intercept memory leaks, run: 
# 	valgrind ./filename 
#
# to find out where they happened add the flags, run: 
# 	--leak-checks=full --track-origins=yes
#
# to find out what part of the code is taking the most time with valgrind, check recording at 1:25
# same for space complexity

# to find out the computation time, run: 
# 	time ./filename
#
# to have more information like max resident bytes, number of page faults
# 	/usr/bin/time ./simple

# complete guide: https://giuse-boccia.notion.site/Using-C-and-WSL-in-VS-Code-a936ba0342af4c06ab40f3dd6de51942