void exit_signals(struct task_struct *tsk)
{
    int group_stop = 0;
    sigset_t unblocked;

    /*
     * @tsk is about to have PF_EXITING set - lock out users which
     * expect stable threadgroup.
     */
    cgroup_threadgroup_change_begin(tsk);

    if (thread_group_empty(tsk) || signal_group_exit(tsk->signal))
    {
        tsk->flags |= PF_EXITING;
        cgroup_threadgroup_change_end(tsk);
        return;
    }

    spin_lock_irq(&tsk->sighand->siglock);
    /*
     * From now this task is not visible for group-wide signals,
     * see wants_signal(), do_signal_stop().
     */
    tsk->flags |= PF_EXITING;

    cgroup_threadgroup_change_end(tsk);

    if (!task_sigpending(tsk))
        goto out;

    unblocked = tsk->blocked;
    signotset(&unblocked);
    retarget_shared_pending(tsk, &unblocked);

    if (unlikely(tsk->jobctl & JOBCTL_STOP_PENDING) &&
        task_participate_group_stop(tsk))
        group_stop = CLD_STOPPED;
out:
    spin_unlock_irq(&tsk->sighand->siglock);

    /*
     * If group stop has completed, deliver the notification.  This
     * should always go to the real parent of the group leader.
     */
    if (unlikely(group_stop))
    {
        read_lock(&tasklist_lock);
        do_notify_parent_cldstop(tsk, false, group_stop);
        read_unlock(&tasklist_lock);
    }
}
