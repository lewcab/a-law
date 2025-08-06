connect -url tcp:127.0.0.1:3121
source G:/440_w_board_files/440_w_board_files.sdk/design_1_wrapper_hw_platform_0/ps7_init.tcl
targets -set -nocase -filter {name =~"APU*" && jtag_cable_name =~ "Digilent Cora Z7 - 7007S 210370BD769BA"} -index 0
loadhw -hw G:/440_w_board_files/440_w_board_files.sdk/design_1_wrapper_hw_platform_0/system.hdf -mem-ranges [list {0x40000000 0xbfffffff}]
configparams force-mem-access 1
targets -set -nocase -filter {name =~"APU*" && jtag_cable_name =~ "Digilent Cora Z7 - 7007S 210370BD769BA"} -index 0
stop
ps7_init
ps7_post_config
targets -set -nocase -filter {name =~ "ARM*#0" && jtag_cable_name =~ "Digilent Cora Z7 - 7007S 210370BD769BA"} -index 0
rst -processor
targets -set -nocase -filter {name =~ "ARM*#0" && jtag_cable_name =~ "Digilent Cora Z7 - 7007S 210370BD769BA"} -index 0
dow G:/440_w_board_files/440_w_board_files.sdk/aLaw/Debug/aLaw.elf
configparams force-mem-access 0
bpadd -addr &main
