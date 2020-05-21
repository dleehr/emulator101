emulator101
===========

Notes

**Calling Conventions**

When you are writing assembly language that only calls your own code, you can decide how the subroutines are going to talk to each other. For example, how do I return to the caller when the subroutine is finished? One way would be to put the return address into a specific register. Another way might be to make the return address be on the top of the stack. A lot of times this decision is dictated by the processor's support. The 8080 has a CALL instruction that puts the function return address on the stack. You'd probably want to use that instruction on the 8080 to implement subroutine calls.

Here's another decision. Is saving the registers the responsibility of the caller or the subroutine? In the example above, the caller saves the registers. But what if there are 32 registers? Saving and restoring 32 registers when only a few may be used by the subroutine would be a waste.

A compromise might be a mixed approach. Let's say you make a policy where a subroutine can use registers r10-r32 without saving their contents, but can't destroy r1-r9. In a situation like this, the caller knows:

- When the function returns, the contents of r1-r9 are exactly what they were
- I can't depend on the contents of r10-r32
- If I need the value in r10-r32 after the subroutine call, I need to save it somewhere before calling it

Similarly, each subroutine knows:

- I can destroy r10-r32
- If I want to use r1-r9, I need to save the contents and restore them prior to returning to the caller

**ABI**

On most modern platforms, these policies are made by engineers and published in a document called the ABI (Application Binary Interface). Using this document, compiler writers know how to compile a code that can call code compiled by other compilers. If you want to write assembly language that can function in an environment like that, you have to know the ABI and write your code to conform to it.

Knowing the ABI can also aid you in debugging code when you don't have the source. The ABI will define the location of the parameters to functions, so when you stop at some arbitrary subroutine you can look in those locations to see what is being passed around.


