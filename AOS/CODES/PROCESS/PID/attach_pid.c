/*
 * attach_pid() must be called with the tasklist_lock write-held.
 */
void attach_pid(struct task_struct *task, enum pid_type type)
{
    struct pid *pid = *task_pid_ptr(task, type);
    hlist_add_head_rcu(&task->pid_links[type], &pid->tasks[type]);
}

static void __change_pid(struct task_struct *task, enum pid_type type,
                         struct pid *new)
{
    struct pid **pid_ptr = task_pid_ptr(task, type);
    struct pid *pid;
    int tmp;

    pid = *pid_ptr;

    hlist_del_rcu(&task->pid_links[type]);
    *pid_ptr = new;

    for (tmp = PIDTYPE_MAX; --tmp >= 0;)
        if (pid_has_task(pid, tmp))
            return;

    free_pid(pid);
}