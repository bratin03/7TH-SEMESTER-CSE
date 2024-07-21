struct sched_class
{
#ifdef CONFIG_UCLAMP_TASK
    /* 
     * uclamp_enabled: Indicates whether utilization clamping is enabled 
     * for tasks. Utilization clamping is a feature that allows the 
     * scheduler to limit the minimum and maximum CPU utilization a task 
     * can claim. This can be used to control power consumption or 
     * enforce QoS constraints.
     */
    int uclamp_enabled;
#endif

    /* 
     * enqueue_task: A function pointer to enqueue a task in the runqueue.
     * This function adds a task to the appropriate place in the runqueue, 
     * depending on its priority and other scheduling policies.
     */
    void (*enqueue_task)(struct rq *rq, struct task_struct *p, int flags);

    /* 
     * dequeue_task: A function pointer to dequeue a task from the runqueue.
     * This function removes a task from the runqueue when it is no longer 
     * runnable, such as when it is going to sleep or exiting.
     */
    void (*dequeue_task)(struct rq *rq, struct task_struct *p, int flags);

    /* 
     * yield_task: A function pointer that handles task yielding. When a 
     * task calls yield(), this function is invoked to move the task to the 
     * end of the runqueue, allowing other tasks to run.
     */
    void (*yield_task)(struct rq *rq);

    /* 
     * yield_to_task: A function pointer that handles yielding to a specific task.
     * This function allows one task to yield its CPU time directly to another 
     * specified task. It returns true if successful, false otherwise.
     */
    bool (*yield_to_task)(struct rq *rq, struct task_struct *p);

    /* 
     * check_preempt_curr: A function pointer that checks whether the current 
     * task should be preempted by another task. This is invoked when a new 
     * task becomes runnable, and it decides whether the new task should 
     * preempt the currently running task.
     */
    void (*check_preempt_curr)(struct rq *rq, struct task_struct *p, int flags);

    /* 
     * pick_next_task: A function pointer that selects the next task to run 
     * on the CPU. This is the core of the scheduling decision-making process, 
     * where the scheduler chooses the most appropriate task to run next based 
     * on the scheduling policy.
     */
    struct task_struct *(*pick_next_task)(struct rq *rq);

    /* 
     * put_prev_task: A function pointer that prepares the current task to be 
     * replaced by another task. This function is called before switching to 
     * the next task and is responsible for saving the state of the current task.
     */
    void (*put_prev_task)(struct rq *rq, struct task_struct *p);

    /* 
     * set_next_task: A function pointer that sets up the next task to be 
     * executed on the CPU. This function is called after pick_next_task() 
     * and prepares the selected task to run.
     */
    void (*set_next_task)(struct rq *rq, struct task_struct *p, bool first);

#ifdef CONFIG_SMP
    /* 
     * balance: A function pointer that performs load balancing across CPUs.
     * In Symmetric Multiprocessing (SMP) systems, this function is used to 
     * redistribute tasks between CPUs to ensure that all CPUs have an 
     * appropriate load.
     */
    int (*balance)(struct rq *rq, struct task_struct *prev, struct rq_flags *rf);

    /* 
     * select_task_rq: A function pointer that selects the most suitable CPU 
     * for a task. This is particularly important in SMP systems, where tasks 
     * may need to be moved to different CPUs for better load balancing.
     */
    int (*select_task_rq)(struct task_struct *p, int task_cpu, int sd_flag, int flags);

    /* 
     * migrate_task_rq: A function pointer that migrates a task from one CPU 
     * to another. This function is called when a task needs to be moved to 
     * a different CPU, often due to load balancing.
     */
    void (*migrate_task_rq)(struct task_struct *p, int new_cpu);

    /* 
     * task_woken: A function pointer that is called when a task is woken up. 
     * This function handles the necessary steps to requeue the task into the 
     * runqueue and make it runnable again.
     */
    void (*task_woken)(struct rq *this_rq, struct task_struct *task);

    /* 
     * set_cpus_allowed: A function pointer that sets the CPUs a task is allowed 
     * to run on. This is used to enforce CPU affinity for tasks, restricting them 
     * to specific CPUs.
     */
    void (*set_cpus_allowed)(struct task_struct *p, const struct cpumask *newmask);

    /* 
     * rq_online: A function pointer that is called when a runqueue (CPU) 
     * comes online. This function handles the necessary setup when a CPU 
     * becomes available for scheduling tasks.
     */
    void (*rq_online)(struct rq *rq);

    /* 
     * rq_offline: A function pointer that is called when a runqueue (CPU) 
     * goes offline. This function handles the necessary cleanup when a CPU 
     * is taken out of the scheduling pool.
     */
    void (*rq_offline)(struct rq *rq);
#endif

    /* 
     * task_tick: A function pointer that is called for each timer tick 
     * while a task is running. This function handles time-based scheduling 
     * decisions, such as time slicing or updating the task's runtime.
     */
    void (*task_tick)(struct rq *rq, struct task_struct *p, int queued);

    /* 
     * task_fork: A function pointer that is called when a task is forked. 
     * This function handles the necessary setup for the new child task.
     */
    void (*task_fork)(struct task_struct *p);

    /* 
     * task_dead: A function pointer that is called when a task exits and 
     * is no longer active. This function handles the cleanup and removal 
     * of the task from the scheduler.
     */
    void (*task_dead)(struct task_struct *p);

    /*
     * switched_from: A function pointer that is called when a task is 
     * switched out from a CPU. This function can drop the runqueue's lock, 
     * but the pair of switched_from and switched_to is serialized by 
     * the task's pi_lock.
     */
    void (*switched_from)(struct rq *this_rq, struct task_struct *task);

    /* 
     * switched_to: A function pointer that is called when a task is switched 
     * to a CPU. This function handles the setup needed for the task to begin 
     * execution on the new CPU.
     */
    void (*switched_to)(struct rq *this_rq, struct task_struct *task);

    /* 
     * prio_changed: A function pointer that is called when a task's priority 
     * is changed. This function allows the scheduler to react to changes in 
     * task priority, potentially reordering tasks in the runqueue.
     */
    void (*prio_changed)(struct rq *this_rq, struct task_struct *task, int oldprio);

    /* 
     * get_rr_interval: A function pointer that returns the time slice 
     * interval for a task in the Round-Robin (RR) scheduling policy. 
     * This determines how long a task will run before being preempted.
     */
    unsigned int (*get_rr_interval)(struct rq *rq, struct task_struct *task);

    /* 
     * update_curr: A function pointer that updates the scheduler's state 
     * for the currently running task. This is typically called during each 
     * timer tick to update runtime statistics and enforce scheduling policies.
     */
    void (*update_curr)(struct rq *rq);

#define TASK_SET_GROUP 0
#define TASK_MOVE_GROUP 1

#ifdef CONFIG_FAIR_GROUP_SCHED
    /* 
     * task_change_group: A function pointer that is called when a task 
     * changes its scheduling group. This is used in group scheduling 
     * to move a task from one group to another, which may affect its 
     * scheduling behavior.
     */
    void (*task_change_group)(struct task_struct *p, int type);
#endif
} __aligned(STRUCT_ALIGNMENT); /* STRUCT_ALIGN(), vmlinux.lds.h */

/**
 * Fair scheduling class
 */

/*
 * All the scheduling class methods:
 */
const struct sched_class fair_sched_class
	__section("__fair_sched_class") = {
	.enqueue_task		= enqueue_task_fair,
	.dequeue_task		= dequeue_task_fair,
	.yield_task		= yield_task_fair,
	.yield_to_task		= yield_to_task_fair,

	.check_preempt_curr	= check_preempt_wakeup,

	.pick_next_task		= __pick_next_task_fair,
	.put_prev_task		= put_prev_task_fair,
	.set_next_task          = set_next_task_fair,

#ifdef CONFIG_SMP
	.balance		= balance_fair,
	.select_task_rq		= select_task_rq_fair,
	.migrate_task_rq	= migrate_task_rq_fair,

	.rq_online		= rq_online_fair,
	.rq_offline		= rq_offline_fair,

	.task_dead		= task_dead_fair,
	.set_cpus_allowed	= set_cpus_allowed_common,
#endif

	.task_tick		= task_tick_fair,
	.task_fork		= task_fork_fair,

	.prio_changed		= prio_changed_fair,
	.switched_from		= switched_from_fair,
	.switched_to		= switched_to_fair,

	.get_rr_interval	= get_rr_interval_fair,

	.update_curr		= update_curr_fair,

#ifdef CONFIG_FAIR_GROUP_SCHED
	.task_change_group	= task_change_group_fair,
#endif

#ifdef CONFIG_UCLAMP_TASK
	.uclamp_enabled		= 1,
#endif
};

/*
 * The order of the sched class addresses are important, as they are
 * used to determine the order of the priority of each sched class in
 * relation to each other.
 */
#define SCHED_DATA				\
	STRUCT_ALIGN();				\
	__begin_sched_classes = .;		\
	*(__idle_sched_class)			\
	*(__fair_sched_class)			\
	*(__rt_sched_class)			\
	*(__dl_sched_class)			\
	*(__stop_sched_class)			\
	__end_sched_classes = .;