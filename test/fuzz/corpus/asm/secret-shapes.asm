mov rax, {secret_local}
mul rax, rbx
shl rax, cl
div rbx
cmp rax, rbx
jne 1f
1:
