struct task_struct
{
    /*
     * prio: The process's priority. It is a combination of the static priority,
     * nice value, and other factors that affect scheduling.
     */
    int prio;

    /*
     * static_prio: The static priority of the task, which is determined by the
     * nice value. It does not change during the life of the process unless the
     * nice value is explicitly changed.
     */
    int static_prio;

    /*
     * normal_prio: The normal priority of the task, calculated based on its
     * static priority and other factors. It is the priority used when the
     * process is scheduled normally (not in real-time).
     */
    int normal_prio;

    /*
     * rt_priority: The real-time priority of the task. This field is only used
     * if the task is a real-time task, which means it is scheduled according
     * to real-time policies such as SCHED_FIFO or SCHED_RR.
     */
    unsigned int rt_priority;

    /*
     * sched_class: A pointer to the scheduling class that this task belongs to.
     * The scheduling class determines the scheduling policy and algorithms
     * used for the task. Examples include the real-time, fair, and deadline
     * scheduling classes.
     */
    const struct sched_class *sched_class;

    /*
     * se: The scheduling entity representing the task in the Completely
     * Fair Scheduler (CFS). It contains all the data needed to manage the
     * task's position in the CFS scheduling algorithm, including time
     * accounting and scheduling statistics.
     */
    struct sched_entity se;

    /*
     * rt: The real-time scheduling entity, used if the task is a real-time task.
     * It contains all the data needed to manage the task's position in
     * the real-time scheduling class, such as SCHED_FIFO or SCHED_RR.
     */
    struct sched_rt_entity rt;

#ifdef CONFIG_CGROUP_SCHED
    /*
     * sched_task_group: A pointer to the scheduling group that this task
     * belongs to. Task groups are used for cgroup-based scheduling, allowing
     * tasks to be grouped together for resource management and scheduling
     * purposes.
     */
    struct task_group *sched_task_group;
#endif

    /*
     * dl: The scheduling entity for deadline scheduling, used if the task
     * is scheduled under the SCHED_DEADLINE policy. It contains the deadline,
     * runtime, and period parameters needed for managing the task's execution
     * under this policy.
     */
    struct sched_dl_entity dl;
};

