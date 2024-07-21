static inline struct pid *get_pid(struct pid *pid)
{
    if (pid)
        refcount_inc(&pid->count);
    return pid;
}
