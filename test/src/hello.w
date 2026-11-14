!include "sys/stdlib.wh"
!include "sys/stdio.wh"
!include "sys/files.wh"
!include "sys/stdint.wh"
!include "sys/string.wh"

fn none entry(i32 argc, ch point arr argv){
    write(stdout_fileno, argv[0], strlen(argv[0]));
}