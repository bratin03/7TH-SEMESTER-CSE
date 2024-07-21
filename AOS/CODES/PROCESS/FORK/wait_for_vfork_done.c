static int wait_for_vfork_done(struct task_struct *child,
                               struct completion *vfork)
{
    int killed;

    freezer_do_not_count();
    cgroup_enter_frozen();
    killed = wait_for_completion_killable(vfork);
    cgroup_leave_frozen(false);
    freezer_count();

    if (killed)
    {
        task_lock(child);
        child->vfork_done = NULL;
        task_unlock(child);
    }

    put_task_struct(child);
    return killed;
}