struct sched_entity
{
    /*
     * load: Represents the load weight of this scheduling entity.
     * It is used by the Completely Fair Scheduler (CFS) to determine
     * how much CPU time this entity should receive relative to other
     * entities. The load weight is influenced by factors like the
     * priority of the task.
     */
    struct load_weight load;

    /*
     * run_node: An rb_node used to place this scheduling entity
     * in the red-black tree, which is the core data structure
     * used by the CFS to manage runnable entities. The red-black
     * tree helps maintain an ordered list of tasks based on their
     * virtual runtime (vruntime).
     */
    struct rb_node run_node;

    /*
     * group_node: A list_head used to link this scheduling entity
     * into a group of scheduling entities when group scheduling
     * is enabled. This allows the scheduler to manage groups of
     * tasks collectively, rather than individually.
     */
    struct list_head group_node;

    /*
     * on_rq: Indicates whether this scheduling entity is currently
     * on the runqueue (i.e., it is runnable). A non-zero value means
     * the entity is on the runqueue, while a zero value means it is not.
     */
    unsigned int on_rq;

    /*
     * exec_start: The timestamp when the task started execution.
     * It is used to calculate how much time the task has spent
     * on the CPU.
     */
    u64 exec_start;

    /*
     * sum_exec_runtime: The total time this scheduling entity
     * has spent on the CPU so far. This value is used by the
     * scheduler to determine how much CPU time the entity has
     * already consumed, which influences future scheduling decisions.
     */
    u64 sum_exec_runtime;

    /*
     * vruntime: The virtual runtime of the scheduling entity.
     * It is the key value used in the CFS's red-black tree
     * to determine the order of tasks. The task with the
     * smallest vruntime is the next one to be scheduled.
     */
    u64 vruntime;

    /*
     * prev_sum_exec_runtime: The sum of the task's execution
     * time before the last time it was scheduled. This value
     * is used for various accounting purposes.
     */
    u64 prev_sum_exec_runtime;

    /*
     * nr_migrations: The number of times this scheduling entity
     * has been migrated from one CPU to another. High migration
     * counts can indicate potential performance issues due to
     * cache locality loss.
     */
    u64 nr_migrations;

    /*
     * statistics: A struct that contains various statistics
     * related to the scheduling entity, such as the number of
     * times it has been scheduled, the time spent waiting,
     * and other metrics that the scheduler uses for decision-making.
     */
    struct sched_statistics statistics;

#ifdef CONFIG_FAIR_GROUP_SCHED
    /*
     * depth: The depth of the scheduling entity in the group
     * scheduling hierarchy. Group scheduling allows tasks to
     * be organized into hierarchical groups, and the depth
     * indicates how deeply nested this entity is within that
     * hierarchy.
     */
    int depth;

    /*
     * parent: A pointer to the parent scheduling entity in the
     * group scheduling hierarchy. This allows the scheduler to
     * traverse up the hierarchy when necessary.
     */
    struct sched_entity *parent;

    /*
     * cfs_rq: A pointer to the runqueue on which this scheduling
     * entity is (or will be) queued. The runqueue is where all
     * runnable tasks/entities are placed by the CFS.
     */
    struct cfs_rq *cfs_rq;

    /*
     * my_q: A pointer to the runqueue "owned" by this entity or
     * group. In the case of a group, this would be the runqueue
     * for the tasks within that group.
     */
    struct cfs_rq *my_q;

    /*
     * runnable_weight: A cached value representing the number
     * of runnable tasks in the runqueue owned by this entity.
     * This is used to quickly access the load of the group
     * without having to traverse the entire runqueue.
     */
    unsigned long runnable_weight;
#endif

#ifdef CONFIG_SMP
    /*
     * avg: The per-entity load average tracking, which is used
     * in Symmetric Multiprocessing (SMP) systems to balance the
     * load across CPUs. This field is placed in a separate cache
     * line to avoid conflicts with other read-mostly fields,
     * enhancing cache efficiency.
     */
    struct sched_avg avg;
#endif
};
