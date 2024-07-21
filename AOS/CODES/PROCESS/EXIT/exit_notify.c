/*
 * Send signals to all our closest relatives so that they know
 * to properly mourn us..
 */
static void exit_notify(struct task_struct *tsk, int group_dead)
{
    bool autoreap;
    struct task_struct *p, *n;
    LIST_HEAD(dead);

    write_lock_irq(&tasklist_lock);
    forget_original_parent(tsk, &dead);

    if (group_dead)
        kill_orphaned_pgrp(tsk->group_leader, NULL);

    tsk->exit_state = EXIT_ZOMBIE;
    if (unlikely(tsk->ptrace))
    {
        int sig = thread_group_leader(tsk) &&
                          thread_group_empty(tsk) &&
                          !ptrace_reparented(tsk)
                      ? tsk->exit_signal
                      : SIGCHLD;
        autoreap = do_notify_parent(tsk, sig);
    }
    else if (thread_group_leader(tsk))
    {
        autoreap = thread_group_empty(tsk) &&
                   do_notify_parent(tsk, tsk->exit_signal);
    }
    else
    {
        autoreap = true;
    }

    if (autoreap)
    {
        tsk->exit_state = EXIT_DEAD;
        list_add(&tsk->ptrace_entry, &dead);
    }

    /* mt-exec, de_thread() is waiting for group leader */
    if (unlikely(tsk->signal->notify_count < 0))
        wake_up_process(tsk->signal->group_exit_task);
    write_unlock_irq(&tasklist_lock);

    list_for_each_entry_safe(p, n, &dead, ptrace_entry)
    {
        list_del_init(&p->ptrace_entry);
        /////////////////////////////////// RELEASE TASK ///////////////////////////
        release_task(p); // RELEASE TASK
        ////////////////////////////////////////////////////////////////////////////
    }
}
