# debug_trace.gdb - Find corrupt block in select_block
# Strategy: Break at select_block, check instrs[0].op for each block
# If op >= 50 (max valid is 49/OP_SYSCALL), it's corrupt

set pagination off
set logging file /tmp/mach_debug_trace.log
set logging overwrite on
set logging enabled on

# select_block args: RDI=ret_ptr, RSI=ctx, RDX=block
break *0x53661b
commands
  silent
  # block ptr is in RDX
  set $blk = $rdx
  # IRBlock layout: label(str=8), instrs(*Instr=8), instr_count(i32=4), instr_cap(i32=4)
  set $label = *(char**)$blk
  set $instrs = *(void**)($blk + 8)
  set $icount = *(int*)($blk + 16)
  set $icap = *(int*)($blk + 20)

  # Check if instrs is non-null and has instructions
  if $instrs != 0 && $icount > 0
    # Check first instr's op (IROp is a tagged union, tag byte is first)
    set $op0 = *(unsigned char*)$instrs
    if $op0 >= 50
      printf "CORRUPT BLOCK at %p: label=%s instrs=%p count=%d cap=%d op0=%d\n", $blk, $label, $instrs, $icount, $icap, $op0
      # Dump first 64 bytes of instrs array
      printf "instrs[0] first 64 bytes:\n"
      x/8gx $instrs
      printf "instrs[0] next 64 bytes (offset 64):\n"
      x/8gx $instrs+64
      printf "instrs[0] next 64 bytes (offset 128):\n"
      x/8gx $instrs+128
      # Also check what's 192 bytes BEFORE instrs (in case of shift)
      printf "Memory 192 bytes BEFORE instrs:\n"
      x/8gx $instrs-192
      printf "Memory 384 bytes BEFORE instrs:\n"
      x/8gx $instrs-384
      # Check block address region
      printf "Block struct full dump:\n"
      x/4gx $blk
      # Stop here for manual inspection
      bt 5
    else
      continue
    end
  else
    continue
  end
end

run build . -o /tmp/mach_s3
