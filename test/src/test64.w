!bits 64!
!os linux!
!arch amd!

fn none exit(i8 code){
    !ifbits 64!
        _asm "mov rax, 60";
        _asm "mov rdi, [rbp - 8]";
        _asm "syscall";
    !endif!
    
    !ifbits 32!
        _asm "mov eax, 1";
        _asm "mov ebx, [ebp - 4]";
        _asm "int 0x80";
    !endif!
}

fn none main(){
    i8 code = 42;
    exit(code);
}