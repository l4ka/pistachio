all:
	@if [ ! -d include ]; then ../../tools/mkdirhier include; fi
	@grep -h "^ *DECLARE_CMD *(" `find ../../kernel/kdb ../../kernel/include/kdb  -name "*.cc" -o -name "*.h"`| perl -pe 's/^ *DECLARE_CMD *\(([^,]+).*/static cmd_ret_t \1(cmd_group_t*);/' >> include/kdb_class_helper.h
	@umask 000
	@rm -rf html
	@doxygen
