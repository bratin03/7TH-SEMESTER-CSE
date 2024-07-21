struct pid *alloc_pid(struct pid_namespace *ns, pid_t *set_tid,
                      size_t set_tid_size)
{
    struct pid *pid;
    enum pid_type type;
    int i, nr;
    struct pid_namespace *tmp;
    struct upid *upid;
    int retval = -ENOMEM;

    /*
     * set_tid_size contains the size of the set_tid array. Starting at
     * the most nested currently active PID namespace it tells alloc_pid()
     * which PID to set for a process in that most nested PID namespace
     * up to set_tid_size PID namespaces. It does not have to set the PID
     * for a process in all nested PID namespaces but set_tid_size must
     * never be greater than the current ns->level + 1.
     */
    if (set_tid_size > ns->level + 1)
        return ERR_PTR(-EINVAL);

    pid = kmem_cache_alloc(ns->pid_cachep, GFP_KERNEL);
    if (!pid)
        return ERR_PTR(retval);

    tmp = ns;
    pid->level = ns->level;

    for (i = ns->level; i >= 0; i--)
    {
        int tid = 0;

        if (set_tid_size)
        {
            tid = set_tid[ns->level - i];

            retval = -EINVAL;
            if (tid < 1 || tid >= pid_max)
                goto out_free;
            /*
             * Also fail if a PID != 1 is requested and
             * no PID 1 exists.
             */
            if (tid != 1 && !tmp->child_reaper)
                goto out_free;
            retval = -EPERM;
            if (!checkpoint_restore_ns_capable(tmp->user_ns))
                goto out_free;
            set_tid_size--;
        }

        idr_preload(GFP_KERNEL);
        spin_lock_irq(&pidmap_lock);

        if (tid)
        {
            nr = idr_alloc(&tmp->idr, NULL, tid,
                           tid + 1, GFP_ATOMIC);
            /*
             * If ENOSPC is returned it means that the PID is
             * alreay in use. Return EEXIST in that case.
             */
            if (nr == -ENOSPC)
                nr = -EEXIST;
        }
        else
        {
            int pid_min = 1;
            /*
             * init really needs pid 1, but after reaching the
             * maximum wrap back to RESERVED_PIDS
             */
            if (idr_get_cursor(&tmp->idr) > RESERVED_PIDS)
                pid_min = RESERVED_PIDS;

            /*
             * Store a null pointer so find_pid_ns does not find
             * a partially initialized PID (see below).
             */
            nr = idr_alloc_cyclic(&tmp->idr, NULL, pid_min,
                                  pid_max, GFP_ATOMIC);
        }
        spin_unlock_irq(&pidmap_lock);
        idr_preload_end();

        if (nr < 0)
        {
            retval = (nr == -ENOSPC) ? -EAGAIN : nr;
            goto out_free;
        }

        pid->numbers[i].nr = nr;
        pid->numbers[i].ns = tmp;
        tmp = tmp->parent;
    }

    /*
     * ENOMEM is not the most obvious choice especially for the case
     * where the child subreaper has already exited and the pid
     * namespace denies the creation of any new processes. But ENOMEM
     * is what we have exposed to userspace for a long time and it is
     * documented behavior for pid namespaces. So we can't easily
     * change it even if there were an error code better suited.
     */
    retval = -ENOMEM;

    get_pid_ns(ns);
    refcount_set(&pid->count, 1);
    spin_lock_init(&pid->lock);
    for (type = 0; type < PIDTYPE_MAX; ++type)
        INIT_HLIST_HEAD(&pid->tasks[type]);

    init_waitqueue_head(&pid->wait_pidfd);
    INIT_HLIST_HEAD(&pid->inodes);

    upid = pid->numbers + ns->level;
    spin_lock_irq(&pidmap_lock);
    if (!(ns->pid_allocated & PIDNS_ADDING))
        goto out_unlock;
    for (; upid >= pid->numbers; --upid)
    {
        /* Make the PID visible to find_pid_ns. */
        idr_replace(&upid->ns->idr, pid, upid->nr);
        upid->ns->pid_allocated++;
    }
    spin_unlock_irq(&pidmap_lock);

    return pid;

out_unlock:
    spin_unlock_irq(&pidmap_lock);
    put_pid_ns(ns);

out_free:
    spin_lock_irq(&pidmap_lock);
    while (++i <= ns->level)
    {
        upid = pid->numbers + i;
        idr_remove(&upid->ns->idr, upid->nr);
    }

    /* On failure to allocate the first pid, reset the state */
    if (ns->pid_allocated == PIDNS_ADDING)
        idr_set_cursor(&ns->idr, 0);

    spin_unlock_irq(&pidmap_lock);

    kmem_cache_free(ns->pid_cachep, pid);
    return ERR_PTR(retval);
}
