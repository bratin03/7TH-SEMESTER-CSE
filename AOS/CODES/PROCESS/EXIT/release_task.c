void release_task(struct task_struct *p)
{
    struct task_struct *leader;
    struct pid *thread_pid;
    int zap_leader;
repeat:
    /* don't need to get the RCU readlock here - the process is dead and
     * can't be modifying its own credentials. But shut RCU-lockdep up */
    rcu_read_lock();
    atomic_dec(&__task_cred(p)->user->processes);
    rcu_read_unlock();

    cgroup_release(p);

    write_lock_irq(&tasklist_lock);
    ptrace_release_task(p);
    thread_pid = get_pid(p->thread_pid);
    __exit_signal(p);

    /*
     * If we are the last non-leader member of the thread
     * group, and the leader is zombie, then notify the
     * group leader's parent process. (if it wants notification.)
     */
    zap_leader = 0;
    leader = p->group_leader;
    if (leader != p && thread_group_empty(leader) && leader->exit_state == EXIT_ZOMBIE)
    {
        /*
         * If we were the last child thread and the leader has
         * exited already, and the leader's parent ignores SIGCHLD,
         * then we are the one who should release the leader.
         */
        zap_leader = do_notify_parent(leader, leader->exit_signal);
        if (zap_leader)
            leader->exit_state = EXIT_DEAD;
    }

    write_unlock_irq(&tasklist_lock);
    seccomp_filter_release(p);
    proc_flush_pid(thread_pid);
    /////////////////////////////////// PUT PID ///////////////////////////
    put_pid(thread_pid);
    //////////////////////////////////////////////////////////////////////
    release_thread(p);
    put_task_struct_rcu_user(p);

    p = leader;
    if (unlikely(zap_leader))
        goto repeat;
}
