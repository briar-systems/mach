# debug_trace2.gdb - Deep investigation of corrupt block
# We know: block=0x7fffe9776b18, instrs=0x7fffe97773b8
# Goal: Find what wrote to the instrs memory, and trace the block's lifecycle

set pagination off
set debuginfod enabled off
set logging file /tmp/mach_debug_trace2.log
set logging overwrite on
set logging enabled on

# Strategy 1: Break at allocator_resize[Instr] (0x540a11) to track
# ALL Instr array allocations. Log (old_ptr, old_cap, new_cap, returned_ptr).
# allocator_resize args: RDI=alloc, RSI=old_ptr, RDX=old_count, RCX=new_count

# Strategy 2: Break at emit_instr (0x53de60) to track which blocks get instructions
# emit_instr args: RDI=alloc, RSI=block, RDX=op

# Let's focus on tracking the instrs pointer for blocks that get the pointer
# that later becomes 0x7fffe97773b8

# Track allocator_resize[Instr] to see all Instr array allocations
break *0x540a11
commands
  silent
  set $old_ptr = $rsi
  set $old_count = $rdx
  set $new_count = $rcx
  printf "RESIZE_INSTR: alloc=%p old_ptr=%p old_count=%lu new_count=%lu\n", $rdi, $old_ptr, $old_count, $new_count
  continue
end

# Also track the return from allocator_resize[Instr] to get the new pointer
# Find the ret instruction
break *0x540a11
disable 1

# Instead, let's use a finish approach. Let's track emit_instr instead
# to catch the block that gets instrs=0x7fffe97773b8
# emit_instr: RDI=alloc, RSI=block, RDX=op
break *0x53de60
commands
  silent
  set $eblk = $rsi
  # Check block's instrs pointer
  set $einstrs = *(void**)($eblk + 8)
  set $eicount = *(int*)($eblk + 16)
  set $eicap = *(int*)($eblk + 20)
  # Log if this block will need a resize (count >= cap) - that's when instrs ptr changes
  if $eicount >= $eicap
    set $elabel = *(char**)$eblk
    printf "EMIT_RESIZE: block=%p label=%s instrs=%p count=%d cap=%d op=%d\n", $eblk, $elabel, $einstrs, $eicount, $eicap, (int)$rdx
  end
  # Also check if instrs is or becomes near our target 0x7fffe97773b8
  # We can't know the result yet, but let's track blocks near this address
  if $eblk > 0x7fffe9776000 && $eblk < 0x7fffe9778000
    set $elabel2 = *(char**)$eblk
    printf "EMIT_NEAR_TARGET: block=%p label=%s instrs=%p count=%d cap=%d op=%d\n", $eblk, $elabel2, $einstrs, $eicount, $eicap, (int)$rdx
  end
  continue
end

# Break at select_block to catch the corrupt one
break *0x53661b
commands
  silent
  set $blk = $rdx
  set $instrs = *(void**)($blk + 8)
  set $icount = *(int*)($blk + 16)
  if $instrs != 0 && $icount > 0
    set $op0 = *(unsigned char*)$instrs
    if $op0 >= 50
      set $label = *(char**)$blk
      printf "\n=== CORRUPT BLOCK FOUND ===\n"
      printf "block=%p label=%s instrs=%p count=%d cap=%d op0=%d\n", $blk, $label, $instrs, $icount, *(int*)($blk+20), $op0

      # Dump all 11 "instructions" at this location
      printf "\nAll 11 instrs (each 192=0xC0 bytes, showing first 8 bytes = op field):\n"
      set $j = 0
      while $j < 11
        set $addr = (unsigned char*)$instrs + $j * 192
        printf "  instrs[%d] @ %p: op=%d (0x%02x), bytes: ", $j, $addr, *(unsigned char*)$addr, *(unsigned char*)$addr
        x/1gx $addr
        set $j = $j + 1
      end

      # Check if the instrs memory looks like valid Instr data but shifted
      # A valid Instr starts with an op (0-49), then padding, then IRValue fields
      # Let's scan nearby memory for valid-looking Instr sequences
      printf "\nScanning for valid Instr pattern near instrs ptr:\n"
      set $scan = (unsigned char*)$instrs - 384
      set $end = (unsigned char*)$instrs + 384
      while $scan < $end
        set $sop = *(unsigned char*)$scan
        if $sop <= 49
          # Could be a valid op - check if next bytes look like padding
          set $b1 = *(unsigned char*)($scan+1)
          set $b2 = *(unsigned char*)($scan+2)
          set $b3 = *(unsigned char*)($scan+3)
          if $b1 == 0 && $b2 == 0 && $b3 == 0
            printf "  Possible valid Instr at %p: op=%d\n", $scan, $sop
          end
        end
        set $scan = $scan + 1
      end

      printf "\nFull memory dump around instrs ptr (768 bytes centered):\n"
      x/96gx ((unsigned char*)$instrs - 384)

      bt 8
    else
      continue
    end
  else
    continue
  end
end

run build . -o /tmp/mach_s3
