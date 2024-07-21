void __noreturn do_exit(long code)
{
    struct task_struct *tsk = current;
    int group_dead;

    /*
     * We can get here from a kernel oops, sometimes with preemption off.
     * Start by checking for critical errors.
     * Then fix up important state like USER_DS and preemption.
     * Then do everything else.
     */

    WARN_ON(blk_needs_flush_plug(tsk));

    if (unlikely(in_interrupt()))
        panic("Aiee, killing interrupt handler!");
    if (unlikely(!tsk->pid))
        panic("Attempted to kill the idle task!");

    /*
     * If do_exit is called because this processes oopsed, it's possible
     * that get_fs() was left as KERNEL_DS, so reset it to USER_DS before
     * continuing. Amongst other possible reasons, this is to prevent
     * mm_release()->clear_child_tid() from writing to a user-controlled
     * kernel address.
     */
    force_uaccess_begin();

    if (unlikely(in_atomic()))
    {
        pr_info("note: %s[%d] exited with preempt_count %d\n",
                current->comm, task_pid_nr(current),
                preempt_count());
        preempt_count_set(PREEMPT_ENABLED);
    }

    profile_task_exit(tsk);
    kcov_task_exit(tsk);

    ptrace_event(PTRACE_EVENT_EXIT, code);

    validate_creds_for_do_exit(tsk);

    /*
     * We're taking recursive faults here in do_exit. Safest is to just
     * leave this task alone and wait for reboot.
     */
    if (unlikely(tsk->flags & PF_EXITING))
    {
        pr_alert("Fixing recursive fault but reboot is needed!\n");
        futex_exit_recursive(tsk);
        set_current_state(TASK_UNINTERRUPTIBLE);
        schedule();
    }

    io_uring_files_cancel();
    /////////////////////////////////// EXIT SIGNALS ///////////////////////////
    exit_signals(tsk); /* sets PF_EXITING */
    //////////////////////////////////////////////////////////////////////

    /* sync mm's RSS info before statistics gathering */
    if (tsk->mm)
        sync_mm_rss(tsk->mm);
    acct_update_integrals(tsk);
    group_dead = atomic_dec_and_test(&tsk->signal->live);
    if (group_dead)
    {
        /*
         * If the last thread of global init has exited, panic
         * immediately to get a useable coredump.
         */
        if (unlikely(is_global_init(tsk)))
            panic("Attempted to kill init! exitcode=0x%08x\n",
                  tsk->signal->group_exit_code ?: (int)code);

#ifdef CONFIG_POSIX_TIMERS
        hrtimer_cancel(&tsk->signal->real_timer);
        exit_itimers(tsk);
#endif
        if (tsk->mm)
            setmax_mm_hiwater_rss(&tsk->signal->maxrss, tsk->mm);
    }
    acct_collect(code, group_dead);
    if (group_dead)
        tty_audit_exit();
    audit_free(tsk);

    /////////////////////////////// EXIT CODE ///////////////////////////
    tsk->exit_code = code; // SET EXIT CODE
    //////////////////////////////////////////////////////////////////////
    taskstats_exit(tsk, group_dead);

    exit_mm();

    if (group_dead)
        acct_process();
    trace_sched_process_exit(tsk);

    exit_sem(tsk);
    exit_shm(tsk);
    exit_files(tsk);
    exit_fs(tsk);
    if (group_dead)
        disassociate_ctty(1);
    exit_task_namespaces(tsk);
    exit_task_work(tsk);
    exit_thread(tsk);

    /*
     * Flush inherited counters to the parent - before the parent
     * gets woken up by child-exit notifications.
     *
     * because of cgroup mode, must be called before cgroup_exit()
     */
    perf_event_exit_task(tsk);

    sched_autogroup_exit_task(tsk);
    cgroup_exit(tsk);

    /*
     * FIXME: do that only when needed, using sched_exit tracepoint
     */
    flush_ptrace_hw_breakpoint(tsk);

    exit_tasks_rcu_start();
    exit_notify(tsk, group_dead);
    proc_exit_connector(tsk);
    mpol_put_task_policy(tsk);
#ifdef CONFIG_FUTEX
    if (unlikely(current->pi_state_cache))
        kfree(current->pi_state_cache);
#endif
    /*
     * Make sure we are holding no locks:
     */
    debug_check_no_locks_held();

    if (tsk->io_context)
        exit_io_context(tsk);

    if (tsk->splice_pipe)
        free_pipe_info(tsk->splice_pipe);

    if (tsk->task_frag.page)
        put_page(tsk->task_frag.page);

    validate_creds_for_do_exit(tsk);

    check_stack_usage();
    preempt_disable();
    if (tsk->nr_dirtied)
        __this_cpu_add(dirty_throttle_leaks, tsk->nr_dirtied);
    exit_rcu();
    exit_tasks_rcu_finish();

    lockdep_free_task(tsk);
    do_task_dead();
}