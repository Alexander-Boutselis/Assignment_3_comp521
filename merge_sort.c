//merge_sort.c

//===========================================================
// COMP 521/L - Assignment 3: Multithreaded Merge Sort Module
// File: mergesort.c
// Description:
//   A Linux kernel module that sorts an integer array using
//   multiple threads (two sorting threads + one merging thread).
//===========================================================

#include <linux/module.h>       // Needed for all kernel modules
#include <linux/kernel.h>       // Needed for KERN_INFO
#include <linux/slab.h>         // For kmalloc/kfree
#include <linux/moduleparam.h>  // For module parameters
#include <linux/kthread.h>      // For kernel threads
#include <linux/delay.h>        // For msleep or similar
#include <linux/mutex.h>        // For synchronization


//===========================================================
// Function Prototypes
//===========================================================
static void merge(int *left, int left_size, int *right, int right_size, int *result);
static int sorting_thread_fn(void *args);
static int merging_thread_fn(void *args);
static int __init proc_init(void);
static void __exit proc_exit(void);

//===========================================================
// merge()
// Description:
//   Merge two sorted lists into one sorted list.
//===========================================================
static void merge(int *left, int left_size, int *right, int right_size, int *result)
{
    // TODO: implement merge operation for two sorted arrays
}

//===========================================================
// sorting_thread_fn()
// Description:
//   Thread function that recursively sorts a sublist.
//===========================================================
static int sorting_thread_fn(void *args)
{
    // TODO: implement sorting logic (split, sort, merge)
    return 0;
}

//===========================================================
// merging_thread_fn()
// Description:
//   Thread function that merges two sorted halves.
//===========================================================
static int merging_thread_fn(void *args)
{
    // TODO: implement merging of sorted halves
    return 0;
}

//===========================================================
// proc_init()
// Description:
//   Initializes module, sets up threads, and starts sorting.
//===========================================================
static int __init proc_init(void)
{
    // TODO: copy input parameters, create threads, print initial data
    return 0;
}

//===========================================================
// proc_exit()
// Description:
//   Cleans up threads and memory, unloads module.
//===========================================================
static void __exit proc_exit(void)
{
    // TODO: stop threads and free any allocated resources
}

//===========================================================
// Register module entry/exit points
//===========================================================
module_init(proc_init);
module_exit(proc_exit);


//===========================================================
// Module metadata
//===========================================================
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alexander Boutselis");
MODULE_DESCRIPTION("Multithreaded Merge Sort Kernel Module");
