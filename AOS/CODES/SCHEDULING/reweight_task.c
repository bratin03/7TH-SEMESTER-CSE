void reweight_task(struct task_struct *p, int prio)
{
    struct sched_entity *se = &p->se;
    struct cfs_rq *cfs_rq = cfs_rq_of(se);
    struct load_weight *load = &se->load;
    unsigned long weight = scale_load(sched_prio_to_weight[prio]);

    reweight_entity(cfs_rq, se, weight);
    load->inv_weight = sched_prio_to_wmult[prio];
}

static void reweight_entity(struct cfs_rq *cfs_rq, struct sched_entity *se,
                            unsigned long weight)
{
    if (se->on_rq)
    {
        /* commit outstanding execution time */
        if (cfs_rq->curr == se)
            update_curr(cfs_rq);
        update_load_sub(&cfs_rq->load, se->load.weight);
    }
    dequeue_load_avg(cfs_rq, se);

    update_load_set(&se->load, weight);

#ifdef CONFIG_SMP
    do
    {
        u32 divider = get_pelt_divider(&se->avg);

        se->avg.load_avg = div_u64(se_weight(se) * se->avg.load_sum, divider);
    } while (0);
#endif

    enqueue_load_avg(cfs_rq, se);
    if (se->on_rq)
        update_load_add(&cfs_rq->load, se->load.weight);
}