static struct task_struct *dup_task_struct(struct task_struct *orig, int node)
{
    struct task_struct *tsk;
    unsigned long *stack;
    struct vm_struct *stack_vm_area __maybe_unused;
    int err;

    if (node == NUMA_NO_NODE)
        node = tsk_fork_get_node(orig);
    tsk = alloc_task_struct_node(node);
    if (!tsk)
        return NULL;

    stack = alloc_thread_stack_node(tsk, node);
    if (!stack)
        goto free_tsk;

    if (memcg_charge_kernel_stack(tsk))
        goto free_stack;

    stack_vm_area = task_stack_vm_area(tsk);

    err = arch_dup_task_struct(tsk, orig);

    /*
     * arch_dup_task_struct() clobbers the stack-related fields.  Make
     * sure they're properly initialized before using any stack-related
     * functions again.
     */
    tsk->stack = stack;
#ifdef CONFIG_VMAP_STACK
    tsk->stack_vm_area = stack_vm_area;
#endif
#ifdef CONFIG_THREAD_INFO_IN_TASK
    refcount_set(&tsk->stack_refcount, 1);
#endif

    if (err)
        goto free_stack;

    err = scs_prepare(tsk, node);
    if (err)
        goto free_stack;

#ifdef CONFIG_SECCOMP
    /*
     * We must handle setting up seccomp filters once we're under
     * the sighand lock in case orig has changed between now and
     * then. Until then, filter must be NULL to avoid messing up
     * the usage counts on the error path calling free_task.
     */
    tsk->seccomp.filter = NULL;
#endif

    setup_thread_stack(tsk, orig);
    clear_user_return_notifier(tsk);
    clear_tsk_need_resched(tsk);
    set_task_stack_end_magic(tsk);

#ifdef CONFIG_STACKPROTECTOR
    tsk->stack_canary = get_random_canary();
#endif
    if (orig->cpus_ptr == &orig->cpus_mask)
        tsk->cpus_ptr = &tsk->cpus_mask;

    /*
     * One for the user space visible state that goes away when reaped.
     * One for the scheduler.
     */
    refcount_set(&tsk->rcu_users, 2);
    /* One for the rcu users */
    refcount_set(&tsk->usage, 1);
#ifdef CONFIG_BLK_DEV_IO_TRACE
    tsk->btrace_seq = 0;
#endif
    tsk->splice_pipe = NULL;
    tsk->task_frag.page = NULL;
    tsk->wake_q.next = NULL;
    tsk->pf_io_worker = NULL;

    account_kernel_stack(tsk, 1);

    kcov_task_init(tsk);

#ifdef CONFIG_FAULT_INJECTION
    tsk->fail_nth = 0;
#endif

#ifdef CONFIG_BLK_CGROUP
    tsk->throttle_queue = NULL;
    tsk->use_memdelay = 0;
#endif

#ifdef CONFIG_MEMCG
    tsk->active_memcg = NULL;
#endif
    return tsk;

free_stack:
    free_thread_stack(tsk);
free_tsk:
    free_task_struct(tsk);
    return NULL;
}