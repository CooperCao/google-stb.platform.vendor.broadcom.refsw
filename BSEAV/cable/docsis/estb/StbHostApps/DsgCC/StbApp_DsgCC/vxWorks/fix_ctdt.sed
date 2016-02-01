/long 0/i\
__asm__ ("    .long _GLOBAL_$I$__stdstrbufs_o");\
__asm__ ("    .long _GLOBAL_$I$__stlinst_o");\
__asm__ ("    .long _GLOBAL_$I$__streambuf_o");
/_dtors:/a\
__asm__ ("    .long _GLOBAL_$D$__streambuf_o");\
__asm__ ("    .long _GLOBAL_$D$__stlinst_o");\
__asm__ ("    .long _GLOBAL_$D$__stdstrbufs_o");

