/*
 * Perform scheduler related setup for a newly forked process p.
 * p is forked by current.
 *
 * __sched_fork() is basic setup used by init_idle() too:
 */
static void __sched_fork(unsigned long clone_flags, struct task_struct *p)
{
    p->on_rq = 0;

    p->se.on_rq = 0;
    p->se.exec_start = 0;
    p->se.sum_exec_runtime = 0;
    p->se.prev_sum_exec_runtime = 0;
    p->se.nr_migrations = 0;
    p->se.vruntime = 0;
    INIT_LIST_HEAD(&p->se.group_node);

#ifdef CONFIG_FAIR_GROUP_SCHED
    p->se.cfs_rq = NULL;
#endif

#ifdef CONFIG_SCHEDSTATS
    /* Even if schedstat is disabled, there should not be garbage */
    memset(&p->se.statistics, 0, sizeof(p->se.statistics));
#endif

    RB_CLEAR_NODE(&p->dl.rb_node);
    init_dl_task_timer(&p->dl);
    init_dl_inactive_task_timer(&p->dl);
    __dl_clear_params(p);

    INIT_LIST_HEAD(&p->rt.run_list);
    p->rt.timeout = 0;
    p->rt.time_slice = sched_rr_timeslice;
    p->rt.on_rq = 0;
    p->rt.on_list = 0;

#ifdef CONFIG_PREEMPT_NOTIFIERS
    INIT_HLIST_HEAD(&p->preempt_notifiers);
#endif

#ifdef CONFIG_COMPACTION
    p->capture_control = NULL;
#endif
    init_numa_balancing(clone_flags, p);
#ifdef CONFIG_SMP
    p->wake_entry.u_flags = CSD_TYPE_TTWU;
#endif
}

/*
 * fork()/clone()-time setup:
 */
int sched_fork(unsigned long clone_flags, struct task_struct *p)
{
    __sched_fork(clone_flags, p);
    /*
     * We mark the process as NEW here. This guarantees that
     * nobody will actually run it, and a signal or other external
     * event cannot wake it up and insert it on the runqueue either.
     */
    p->state = TASK_NEW;

    /*
     * Make sure we do not leak PI boosting priority to the child.
     */
    p->prio = current->normal_prio;

    uclamp_fork(p);

    /*
     * Revert to default priority/policy on fork if requested.
     */
    if (unlikely(p->sched_reset_on_fork))
    {
        if (task_has_dl_policy(p) || task_has_rt_policy(p))
        {
            p->policy = SCHED_NORMAL;
            p->static_prio = NICE_TO_PRIO(0);
            p->rt_priority = 0;
        }
        else if (PRIO_TO_NICE(p->static_prio) < 0)
            p->static_prio = NICE_TO_PRIO(0);

        p->prio = p->normal_prio = p->static_prio;
        set_load_weight(p);

        /*
         * We don't need the reset flag anymore after the fork. It has
         * fulfilled its duty:
         */
        p->sched_reset_on_fork = 0;
    }

    if (dl_prio(p->prio))
        return -EAGAIN;
    else if (rt_prio(p->prio))
        p->sched_class = &rt_sched_class;
    else
        p->sched_class = &fair_sched_class;

    init_entity_runnable_average(&p->se);

#ifdef CONFIG_SCHED_INFO
    if (likely(sched_info_on()))
        memset(&p->sched_info, 0, sizeof(p->sched_info));
#endif
#if defined(CONFIG_SMP)
    p->on_cpu = 0;
#endif
    init_task_preempt_count(p);
#ifdef CONFIG_SMP
    plist_node_init(&p->pushable_tasks, MAX_PRIO);
    RB_CLEAR_NODE(&p->pushable_dl_tasks);
#endif
    return 0;
}