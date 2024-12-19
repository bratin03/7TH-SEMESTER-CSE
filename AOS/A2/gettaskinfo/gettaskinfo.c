/*
 * CS60038: Advances in Operating Systems Design
 * Assignment 2
 * Members:
 * > 21CS30037 - Datta Ksheeraj
 * > 21CS10016 - Bratin Mondal
 *
 * Department of Computer Science and Engineering,
 * Indian Institute of Technology Kharagpur
 */

#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/rcupdate.h>

/**
 * gettaskinfo - A system call to retrieve information about a task.
 * @pid: The process ID of the task to retrieve information for.
 * @buffer: A pointer to a user-space buffer where the information will be copied.
 *
 * This system call retrieves information about a specific task identified by
 * the provided PID (Process ID). The information includes the task's state,
 * start time, and priority. It formats this information into a string and
 * copies it to the user-space buffer provided by the caller.
 *
 * Returns:
 * * 0 on success.
 * * -ESRCH if no task with the given PID is found.
 * * -EFAULT if there is a failure in copying data to user-space.
 */
SYSCALL_DEFINE2(gettaskinfo, pid_t, pid, char __user *, buffer)
{
    struct task_struct *task; ///< Pointer to the task structure for the given PID.
    char info[256];           ///< Buffer to hold the formatted task information.
    int len;                  ///< Length of the formatted string.

    printk(KERN_INFO "gettaskinfo: Called with PID = %d\n", pid);

    // Acquire RCU read lock to safely read task information.
    rcu_read_lock();

    // Find the task structure by PID.
    task = pid_task(find_vpid(pid), PIDTYPE_PID);
    task = rcu_dereference(task);
    if (!task)
    {
        printk(KERN_ERR "gettaskinfo: No task found with PID = %d\n", pid);
        rcu_read_unlock(); // Release the lock before returning.
        return -ESRCH;     // Return error code for no such process.
    }

    // Format the task information into the 'info' buffer (space separated)
    len = snprintf(info, sizeof(info), "%ld %llu %d\n",
                   task->state, task->start_time, task->normal_prio);

    // Release RCU read lock before copying data to user space.
    rcu_read_unlock();

    printk(KERN_INFO "gettaskinfo: Info formatted: %s\n", info);

    // Check if the user buffer is accessible and writable before copying data.
    if (!access_ok(buffer, len))
    {
        printk(KERN_ERR "gettaskinfo: User buffer is not accessible\n");
        return -EFAULT;
    }

    // Copy the formatted information from kernel space to user space.
    if (copy_to_user(buffer, info, len))
    {
        printk(KERN_ERR "gettaskinfo: Failed to copy info to user buffer\n");
        return -EFAULT; // Return error code for failed copy.
    }

    printk(KERN_INFO "gettaskinfo: Successfully copied info to user buffer\n");

    return 0; // Return success code.
}
