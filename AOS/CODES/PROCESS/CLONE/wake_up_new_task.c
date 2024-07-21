/*
 * wake_up_new_task - wake up a newly created task for the first time.
 *
 * This function will do some initial scheduler statistics housekeeping
 * that must be done for every newly created context, then puts the task
 * on the runqueue and wakes it.
 */
void wake_up_new_task(struct task_struct *p)
{
    struct rq_flags rf;
    struct rq *rq;

    raw_spin_lock_irqsave(&p->pi_lock, rf.flags);
    p->state = TASK_RUNNING;
#ifdef CONFIG_SMP
    /*
     * Fork balancing, do it here and not earlier because:
     *  - cpus_ptr can change in the fork path
     *  - any previously selected CPU might disappear through hotplug
     *
     * Use __set_task_cpu() to avoid calling sched_class::migrate_task_rq,
     * as we're not fully set-up yet.
     */
    p->recent_used_cpu = task_cpu(p);
    rseq_migrate(p);
    __set_task_cpu(p, select_task_rq(p, task_cpu(p), SD_BALANCE_FORK, 0));
#endif
    rq = __task_rq_lock(p, &rf);
    update_rq_clock(rq);
    post_init_entity_util_avg(p);

    activate_task(rq, p, ENQUEUE_NOCLOCK);
    trace_sched_wakeup_new(p);
    check_preempt_curr(rq, p, WF_FORK);
#ifdef CONFIG_SMP
    if (p->sched_class->task_woken)
    {
        /*
         * Nothing relies on rq->lock after this, so its fine to
         * drop it.
         */
        rq_unpin_lock(rq, &rf);
        p->sched_class->task_woken(rq, p);
        rq_repin_lock(rq, &rf);
    }
#endif
    task_rq_unlock(rq, p, &rf);
}