        .text
        .set    noreorder
# Atomic compare and swap. Atomicaly reads unsigned value at addr, compares it
# with compare value and if matches replaces value at addr with swap.
# returns 1 if successfuly swapped value 0 if not.
# extern prototype
# unsigned CAS(unsigned * addr, unsigned compare, unsigned swap)
        .globl  CAS
CAS:
        ll      $8, 0($4)
        bne     $8, $5, exit
        or      $2, $0, $0
        sc      $6, 0($4)
        or      $2, $6, $0
exit:   
        j       $31
        nop

