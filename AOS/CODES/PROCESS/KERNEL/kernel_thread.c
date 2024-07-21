/*
 * Create a kernel thread.
 */
pid_t kernel_thread(int (*fn)(void *), void *arg, unsigned long flags)
{
    struct kernel_clone_args args = {
        .flags = ((lower_32_bits(flags) | CLONE_VM |
                   CLONE_UNTRACED) &
                  ~CSIGNAL),
        .exit_signal = (lower_32_bits(flags) & CSIGNAL),
        .stack = (unsigned long)fn,
        .stack_size = (unsigned long)arg,
    };

    return kernel_clone(&args);
}