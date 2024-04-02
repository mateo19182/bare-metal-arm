.syntax unified
.cpu cortex-m0plus
.text
.globl divide
.type divide, %function

divide:
    // Input:
    // r0 = dividend
    // r1 = divisor
    // Output:
    // r0 = quotient

    // Initialize quotient to 0
    movs r2, #0

start_loop:
    // If dividend < divisor, division is done
    cmp r0, r1
    blt end_division

    // Subtract divisor from dividend
    subs r0, r0, r1

    // Increment quotient
    adds r2, r2, #1

    // Loop back
    b start_loop

end_division:
    // Move result to r0
    mov r0, r2

    // Return from function
    bx lr
