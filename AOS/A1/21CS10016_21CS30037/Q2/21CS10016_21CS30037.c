/*
 * CS60038: Advances in Operating Systems Design
 * Assignment 1, Part B: Loadable Kernel Module(LKM) for a Set
 * Members:
 * > 21CS30037 - Datta Ksheeraj
 * > 21CS10016 - Bratin Mondal
 *
 * Department of Computer Science and Engineering,
 * Indian Institute of Technology Kharagpur
 */

// Header files
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/rbtree.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ksheeraj and Bratin");
MODULE_DESCRIPTION("LKM for a Set with Max Capacity");
MODULE_VERSION("1.0");

#define FILE_NAME "partb_21CS10016_21CS30037" /** < Name of the proc file >*/
#define MAX_SIZE 1024                         /** < Maximum size of the buffer >*/

static DEFINE_MUTEX(procFileMutex); /** < Mutex for the proc file >*/

/**
 * Enum to represent the state of a process
 *
 * FILE_OPEN: File is open but set is not initialized
 * SET_READY: File is open and set is initialized, ready for reads and writes
 */
enum procStatus
{
    FILE_OPEN,
    SET_READY
};

/**
 * Struct to represent a set
 *
 * elements: Array of elements in the set
 * capacity: Maximum capacity of the set
 * size: Number of elements in the set
 */
struct set
{
    int *elements;
    int capacity;
    int size;
};

/**
 * Struct to represent a process node
 *
 * node: Red-Black tree node
 * pid: Process ID
 * processSet: Pointer to the set associated with the process
 * state: State of the process
 */
struct processNode
{
    struct rb_node node;
    pid_t pid;
    struct set *processSet;
    enum procStatus state;
};

static struct proc_dir_entry *proc_file;      /**< Pointer to the proc file >*/
static char procfs_buffer[MAX_SIZE];          /**< Buffer to store data >*/
static size_t procfs_buffer_size = 0;         /**< Size of the buffer >*/
static struct rb_root process_root = RB_ROOT; /**< Root of the process nodes >*/

/**
 * set_init: Initialize a set
 *
 * @param capacity: Maximum capacity of the set
 * @return Pointer to the set
 */
static struct set *set_init(int capacity)
{
    struct set *s = kmalloc(sizeof(struct set), GFP_KERNEL); // Allocate memory for the set
    if (!s)
    {
        printk(KERN_ALERT "E: Memory allocation for set failed for capacity %d | PID: %d\n", capacity, current->pid);
        return NULL;
    }

    s->elements = kmalloc(capacity * sizeof(int), GFP_KERNEL); // Allocate memory for the elements
    if (!s->elements)
    {
        printk(KERN_ALERT "E: Memory allocation for set elements failed for capacity %d | PID: %d\n", capacity, current->pid);
        kfree(s);
        return NULL;
    }

    s->capacity = capacity; // Set the capacity
    s->size = 0;            // Initialize the size to 0

    return s; // Return the set
}

/**
 * set_delete: Delete a set
 *
 * @param s: Pointer to the set
 * @return void
 */
static void set_delete(struct set *s)
{
    if (s) // If the set exists
    {
        if (s->elements)
        {
            kfree(s->elements); // Free the memory allocated for the elements
        }
        kfree(s); // Free the memory allocated for the set
    }
}

/**
 * set_contains: Check if an element is present in the set
 *
 * @param s: Pointer to the set
 * @param val: Value to check
 * @return 1 if the element is present, 0 otherwise
 */
static int set_contains(struct set *s, int val)
{
    int i;
    for (i = 0; i < s->size; i++)
    {
        if (s->elements[i] == val)
        {
            return 1;
        }
    }
    return 0;
}

/**
 * set_insert: Insert an element into the set
 *
 * @param s: Pointer to the set
 * @param val: Value to insert
 * @return 0 on success, -EACCES if the set is full.
 */
static int set_insert(struct set *s, int val)
{
    int i;
    if (s->size >= s->capacity) // Check if the set is full
    {
        printk(KERN_ALERT "E: Set is full for capacity %d | PID: %d\n", s->capacity, current->pid);
        return -EACCES;
    }

    if (set_contains(s, val)) // Check if the element already exists in the set
    {
        return 0; // Return 0 if the element is already present (No need to insert)
    }

    // Insert the element in sorted order
    i = s->size - 1;
    while (i >= 0 && s->elements[i] > val)
    {
        s->elements[i + 1] = s->elements[i];
        i--;
    }
    s->elements[i + 1] = val;
    s->size++; // Increment the size of the set
    return 0;
}

/**
 * set_read: Read the elements of the set
 *
 * @param s: Pointer to the set
 * @param buffer: Buffer to store the elements
 * @param length: Length of the buffer
 * @return Size of the set in bytes
 */
static ssize_t set_read(struct set *s, char __user *buffer, size_t length)
{
    size_t bytes_to_copy = min(length, (size_t)(s->size * sizeof(int))); // Get the number of bytes to copy, minimum of buffer length and set size in bytes
    bytes_to_copy = (bytes_to_copy / sizeof(int)) * sizeof(int);         // Round down to the nearest multiple of 4 bytes
    if (copy_to_user(buffer, s->elements, bytes_to_copy))
    {
        return -EACCES;
    }
    return s->size * sizeof(int); // Return the size of the set in bytes even if the buffer is not sufficient to copy all elements. So that the user can know the size of the set and allocate a buffer accordingly.
}

/**
 * process_find: Find a process node
 *
 * @param pid: Process ID
 * @return Pointer to the process node if found, NULL otherwise
 */
static struct processNode *process_find(pid_t pid)
{
    struct rb_node *node = process_root.rb_node; // Start from the root node

    while (node) // Traverse the tree
    {
        struct processNode *data = container_of(node, struct processNode, node); // Get the process node

        if (pid < data->pid) // Move to the left child
        {
            node = node->rb_left;
        }
        else if (pid > data->pid) // Move to the right child
        {
            node = node->rb_right;
        }
        else // Process node found
        {
            return data;
        }
    }
    return NULL; // Process node not found
}

/**
 * processInsert: Insert a process node
 *
 * @param pid: Process ID
 * @return Pointer to the process node
 */
static struct processNode *processInsert(pid_t pid)
{
    struct processNode *node = kmalloc(sizeof(struct processNode), GFP_KERNEL); // Allocate memory for the process node
    struct rb_node **new, *parent;                                              // New node and parent node

    if (!node)
    {
        printk(KERN_ALERT "E: Memory allocation for process node failed | PID: %d\n", pid);
        return NULL;
    }

    node->pid = pid;         // Set the process ID
    node->processSet = NULL; // Set the set to NULL
    node->state = FILE_OPEN; // Set the state to FILE_OPEN

    // Insert the process node in the red-black tree
    new = &(process_root.rb_node); // Start from the root node
    parent = NULL;                 // Initialize the new node and parent node
    while (*new)                   // Traverse the tree
    {
        struct processNode *this = container_of(*new, struct processNode, node); // Get the process node
        parent = *new;                                                           // Set the parent node
        if (pid < this->pid)                                                     // Move to the left child
        {
            new = &((*new)->rb_left);
        }
        else if (pid > this->pid) // Move to the right child
        {
            new = &((*new)->rb_right);
        }
        else // Process node already exists
        {
            kfree(node);
            return NULL;
        }
    }
    rb_link_node(&node->node, parent, new);      // Link the process node
    rb_insert_color(&node->node, &process_root); // Insert the process node
    return node;                                 // Return the process node
}

/**
 * processDelete: Delete a process node
 *
 * @param pid: Process ID
 * @return 0 on success, -EACCES if the process node is not found
 */
static int processDelete(pid_t pid)
{
    struct processNode *node = process_find(pid); // Find the process node
    if (!node)
    {
        printk(KERN_ALERT "E: Process node not found | PID: %d\n", pid);
        return -EACCES;
    }

    rb_erase(&node->node, &process_root); // Erase the process node

    if (node->processSet)
    {
        set_delete(node->processSet); // Delete the set associated with the process
    }
    kfree(node); // Free the memory allocated for the process node

    return 0; // Return the status
}

/**
 * helperWrite: Handle write operations on the proc file
 *
 * @param node: Pointer to the process node
 * @return Number of bytes written
 */
static ssize_t helperWrite(struct processNode *node)
{
    size_t capacity;
    int value, ret_val;

    if (node->state == FILE_OPEN) // If the set is not initialized yet
    {
        if (procfs_buffer_size != 1) // Check if the capacity buffer size is 1 byte
        {
            printk(KERN_ALERT "E: Capacity buffer size must be 1 byte | PID: %d\n", node->pid);
            return -EINVAL;
        }

        capacity = (size_t)procfs_buffer[0];
        if (capacity < 1 || capacity > 100) // Check if the capacity is between 1 and 100
        {
            printk(KERN_ALERT "E: Capacity must be between 1 and 100 | PID: %d\n", node->pid);
            return -EINVAL;
        }

        node->processSet = set_init(capacity); // Initialize the set
        if (!node->processSet)
        {
            printk(KERN_ALERT "E: Could not initialize set with capacity %lu | PID: %d\n", capacity, node->pid);
            return -ENOMEM;
        }

        node->state = SET_READY; // Set the state to SET_READY
        printk(KERN_INFO "I: Set initialized with capacity %lu | PID: %d\n", capacity, node->pid);
    }
    else if (node->state == SET_READY) // If the set is initialized
    {
        if (procfs_buffer_size != sizeof(int)) // Check if the value buffer size is 4 bytes
        {
            printk(KERN_ALERT "E: Value buffer size must be 4 bytes | PID: %d\n", node->pid);
            return -EINVAL;
        }

        value = *((int *)procfs_buffer);

        ret_val = set_insert(node->processSet, value); // Insert the value into the set
        if (ret_val < 0)
        {
            printk(KERN_ALERT "E: Could not insert value %d into set | PID: %d\n", value, node->pid);
            return -EACCES;
        }

        printk(KERN_INFO "I: Value %d inserted in set | PID: %d\n", value, node->pid);
    }

    return procfs_buffer_size; // Return the number of bytes written
}

/**
 * process_file_write: Write operation on the proc file
 *
 * @param filep: Pointer to the file
 * @param buffer: Buffer to write
 * @param length: Length of the buffer
 * @param offset: Offset
 * @return Number of bytes written
 */
static ssize_t process_file_write(struct file *filep, const char __user *buffer, size_t length, loff_t *offset)
{
    pid_t pid;
    int ret_val;
    struct processNode *node;

    pid = current->pid;
    printk(KERN_INFO "I: Writing to the file | PID: %d\n", pid);
    mutex_lock(&procFileMutex); // Lock the mutex

    ret_val = 0;

    node = process_find(pid); // Find the process node
    if (!node)
    {
        printk(KERN_ALERT "E: No process node found | PID: %d\n", pid);
        ret_val = -EACCES;
    }
    else
    {
        if (buffer == NULL || length == 0) // Check if the buffer is empty
        {
            printk(KERN_ALERT "E: No data to write | PID: %d\n", pid);
            ret_val = -EINVAL;
        }
        else
        {
            procfs_buffer_size = min(length, (size_t)MAX_SIZE); // Set the buffer size

            if (copy_from_user(procfs_buffer, buffer, procfs_buffer_size)) // Copy data from user space
            {
                printk(KERN_ALERT "E: Could not copy data from user space | PID: %d\n", pid);
                ret_val = -EFAULT;
            }
            else
            {
                ret_val = helperWrite(node); // Handle the write operation
            }
        }
    }

    mutex_unlock(&procFileMutex); // Unlock the mutex

    return ret_val; // Return the number of bytes written
}

/**
 * process_file_read: Read operation on the proc file
 *
 * @param filep: Pointer to the file
 * @param buffer: Buffer to read
 * @param length: Length of the buffer
 * @param offset: Offset
 * @return Number of bytes read
 */
static ssize_t process_file_read(struct file *filep, char __user *buffer, size_t length, loff_t *offset)
{
    pid_t pid;
    ssize_t ret_val;
    struct processNode *node;

    pid = current->pid;
    printk(KERN_INFO "I: Reading from the file | PID: %d\n", pid);
    mutex_lock(&procFileMutex); // Lock the mutex

    ret_val = 0;

    node = process_find(pid); // Find the process node
    if (!node)
    {
        printk(KERN_ALERT "E: No process node found | PID: %d\n", pid);
        ret_val = -EACCES;
    }
    else
    {
        if (node->state == FILE_OPEN) // Check if the set is not initialized yet
        {
            printk(KERN_ALERT "E: Set not initialized | PID: %d\n", pid);
            ret_val = -EACCES;
        }
        else if (!node->processSet) // Check if the set is NULL
        {
            printk(KERN_ALERT "E: Set not initialized | PID: %d\n", pid);
            ret_val = -EACCES;
        }
        else
        {
            ret_val = set_read(node->processSet, buffer, length); // Read the elements of the set
            if (ret_val < 0)
            {
                printk(KERN_ALERT "E: Could not read data from set | PID: %d\n", pid);
            }
        }
    }

    mutex_unlock(&procFileMutex); // Unlock the mutex

    return ret_val; // Return the number of bytes read
}

/**
 * process_file_open: Open operation on the proc file
 *
 * @param inode: Pointer to the inode
 * @param file: Pointer to the file
 * @return 0 on success, -EACCES if the process already has the file open, -ENOMEM if memory allocation fails
 */
static int process_file_open(struct inode *inode, struct file *file)
{
    pid_t pid;
    int ret_val;
    struct processNode *node;

    pid = current->pid;
    printk(KERN_INFO "I: Opening the file | PID: %d\n", pid);
    mutex_lock(&procFileMutex); // Lock the mutex

    ret_val = 0;

    node = process_find(pid); // Find the process node
    if (node)                 // Check if the process already has the file open
    {
        printk(KERN_ALERT "E: Already have the process file open | PID: %d\n", pid);
        ret_val = -EACCES;
    }
    else
    {
        node = processInsert(pid); // Insert the process node
        if (!node)
        {
            printk(KERN_ALERT "E: Could not allocate memory | PID: %d\n", pid);
            ret_val = -ENOMEM;
        }
        else
        {
            printk(KERN_INFO "I: Process file opened | PID: %d\n", pid);
        }
    }

    mutex_unlock(&procFileMutex); // Unlock the mutex

    return ret_val; // Return the status
}

/**
 * process_file_release: Release operation on the proc file
 *
 * @param inode: Pointer to the inode
 * @param file: Pointer to the file
 * @return 0 on success, -EACCES if the process does not have the file open
 */
static int process_file_release(struct inode *inode, struct file *file)
{
    pid_t pid = current->pid;
    int ret_val;
    struct processNode *node;

    printk(KERN_INFO "I: Closing the file | PID: %d\n", pid);
    mutex_lock(&procFileMutex); // Lock the mutex

    ret_val = 0;

    node = process_find(pid); // Find the process node
    if (!node)
    {
        printk(KERN_ALERT "E: Process node not found | PID: %d\n", pid);
        ret_val = -EACCES;
    }
    else
    {
        ret_val = processDelete(pid); // Delete the process node
        printk(KERN_INFO "I: Process file closed | PID: %d\n", pid);
    }

    mutex_unlock(&procFileMutex); // Unlock the mutex

    return ret_val; // Return the status
}

/**
 * proc_fops: File operations for the proc file
 *
 * proc_open: Open operation on the proc file
 * SET_READY: Read operation on the proc file
 * proc_write: Write operation on the proc file
 * proc_release: Release operation on the proc file
 */
static const struct proc_ops proc_fops = {
    .proc_open = process_file_open,
    .proc_read = process_file_read,
    .proc_write = process_file_write,
    .proc_release = process_file_release,
};

/**
 * LKM_Init: Module initialization function
 *
 * @return 0 on success
 */
static int __init LKM_Init(void)
{

    printk(KERN_INFO "I: LKM for partb_21CS10016_21CS30037 loaded\n");

    proc_file = proc_create(FILE_NAME, 0666, NULL, &proc_fops); // Create the proc file
    if (!proc_file)
    {
        printk(KERN_ALERT "E: Could not create process file\n");
        return -ENOENT;
    }
    mutex_init(&procFileMutex);                           // Initialize the mutex
    printk(KERN_INFO "I: /proc/%s created\n", FILE_NAME); // printk to the kernel log file
    return 0;
}

/**
 * process_root_delete: Delete the process nodes
 *
 * @return void
 */
static void process_root_delete(void)
{
    struct rb_node *node;
    struct processNode *data;
    printk(KERN_INFO "I: Deleting process nodes\n");

    // Traverse the red-black tree and delete the process nodes
    for (node = rb_first(&process_root); node; node = rb_next(node))
    {
        data = container_of(node, struct processNode, node);
        rb_erase(&data->node, &process_root); // Erase the process node from the tree
        if (data->processSet)
        {
            set_delete(data->processSet); // Delete the set associated with the process
        }
        kfree(data); // Free the memory allocated for the process node
    }
    printk(KERN_INFO "I: Process nodes deleted\n"); // printk to the kernel log file
}

/**
 * LKM_Exit: Module exit function
 *
 * @return void
 */
static void __exit LKM_Exit(void)
{
    process_root_delete();                               // Delete the process nodes
    remove_proc_entry(FILE_NAME, NULL);                  // Remove the proc file
    mutex_destroy(&procFileMutex);                       // Destroy the mutex
    printk(KERN_INFO "I: LKM for partb_set unloaded\n"); // printk to the kernel log file
}

module_init(LKM_Init); // Register the module initialization function
module_exit(LKM_Exit); // Register the module exit function