/*
 * The difference between nr_running and h_nr_running is:
 * nr_running:   present how many entity would take part in the sharing
 *               the cpu power of that cfs_rq
 * h_nr_running: present how many tasks in current cfs runqueue
 */

/* CFS-related fields in a runqueue */
struct cfs_rq
{
    struct load_weight load;
    unsigned int nr_running;
    unsigned int h_nr_running;      /* SCHED_{NORMAL,BATCH,IDLE} */
    unsigned int idle_h_nr_running; /* SCHED_IDLE */

    u64 exec_clock;
    u64 min_vruntime;
#ifndef CONFIG_64BIT
    u64 min_vruntime_copy;
#endif

    struct rb_root_cached tasks_timeline;

    /*
     * 'curr' points to currently running entity on this cfs_rq.
     * It is set to NULL otherwise (i.e when none are currently running).
     */
    struct sched_entity *curr;
    struct sched_entity *next;
    struct sched_entity *last;
    struct sched_entity *skip;

#ifdef CONFIG_SCHED_DEBUG
    unsigned int nr_spread_over;
#endif

#ifdef CONFIG_SMP
    /*
     * CFS load tracking
     */
    struct sched_avg avg;
#ifndef CONFIG_64BIT
    u64 load_last_update_time_copy;
#endif
    struct
    {
        raw_spinlock_t lock ____cacheline_aligned;
        int nr;
        unsigned long load_avg;
        unsigned long util_avg;
        unsigned long runnable_avg;
    } removed;

#ifdef CONFIG_FAIR_GROUP_SCHED
    unsigned long tg_load_avg_contrib;
    long propagate;
    long prop_runnable_sum;

    /*
     *   h_load = weight * f(tg)
     *
     * Where f(tg) is the recursive weight fraction assigned to
     * this group.
     */
    unsigned long h_load;
    u64 last_h_load_update;
    struct sched_entity *h_load_next;
#endif /* CONFIG_FAIR_GROUP_SCHED */
#endif /* CONFIG_SMP */

#ifdef CONFIG_FAIR_GROUP_SCHED
    struct rq *rq; /* CPU runqueue to which this cfs_rq is attached */

    /*
     * leaf cfs_rqs are those that hold tasks (lowest schedulable entity in
     * a hierarchy). Non-leaf lrqs hold other higher schedulable entities
     * (like users, containers etc.)
     *
     * leaf_cfs_rq_list ties together list of leaf cfs_rq's in a CPU.
     * This list is used during load balance.
     */
    int on_list;
    struct list_head leaf_cfs_rq_list;
    struct task_group *tg; /* group that "owns" this runqueue */

#ifdef CONFIG_CFS_BANDWIDTH
    int runtime_enabled;
    s64 runtime_remaining;

    u64 throttled_clock;
    u64 throttled_clock_pelt;
    u64 throttled_clock_pelt_time;
    int throttled;
    int throttle_count;
    struct list_head throttled_list;
#endif /* CONFIG_CFS_BANDWIDTH */
#endif /* CONFIG_FAIR_GROUP_SCHED */
};