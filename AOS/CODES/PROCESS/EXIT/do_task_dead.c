void __noreturn do_task_dead(void)
{
    /* Causes final put_task_struct in finish_task_switch(): */
    set_special_state(TASK_DEAD);

    /* Tell freezer to ignore us: */
    current->flags |= PF_NOFREEZE;

    __schedule(false);
    BUG();

    /* Avoid "noreturn function does return" - but don't continue if BUG() is a NOP: */
    for (;;)
        cpu_relax();
}
