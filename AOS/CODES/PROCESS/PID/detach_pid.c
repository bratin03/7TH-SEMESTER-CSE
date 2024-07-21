void detach_pid(struct task_struct *task, enum pid_type type)
{
	__change_pid(task, type, NULL);
}