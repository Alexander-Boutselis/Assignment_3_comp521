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
// Structure: sort_params
// Description:
//   Holds parameters passed to each sorting or merging thread.
//===========================================================
struct sort_params {
    int size;     // number of elements in this sublist
    int *data;    // pointer to the array (or sublist) to sort
};


//===========================================================
// Thread references
// Description:
//   These hold the task_struct pointers returned by kthread_run().
//   They allow us to manage (stop or check) the threads later.
//===========================================================
static struct task_struct *sort_thread1 = NULL;
static struct task_struct *sort_thread2 = NULL;
static struct task_struct *merge_thread = NULL;


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
//   Called automatically when the module is loaded.
//   Initializes all resources, sets up thread data,
//   and starts the sorting and merging threads.
//===========================================================
static int __init proc_init(void)
{
    printk(KERN_INFO "[INIT] MergeSort module loaded.\n");

    // ------------------------------------------------------
    // Step 1: Validate input parameters
    // ------------------------------------------------------
    // Check if my_size is valid (greater than zero)
    // and ensure the number of elements matches expectation.
    // If invalid, print an error and return -EINVAL to fail load.
    //
    if (my_size <= 0) {
        printk(KERN_ERR "[INIT] Invalid size parameter.\n");
        return -EINVAL;
    }

    // ------------------------------------------------------
    // Step 2: Allocate memory for working copy of the array
    // ------------------------------------------------------
    // Use kmalloc() to create a dynamic array of integers
    // (so we don't modify the module parameter directly).
    //
    int *work_array = kmalloc(my_size * sizeof(int), GFP_KERNEL);
    if (!work_array) {
        printk(KERN_ERR "[INIT] Memory allocation failed.\n");
        return -ENOMEM;
    }

    // ------------------------------------------------------
    // Step 3: Copy data from my_data[] into the working array
    // ------------------------------------------------------
    // Use a for-loop to copy each element.
    //
    for (int i = 0; i < my_size; i++){
        work_array[i] = data[i];
    }

    // ------------------------------------------------------
    // Step 4: Split array into two halves
    // ------------------------------------------------------
    // Determine the midpoint (half = my_size / 2).
    int half = my_size/2;

    // Prepare two struct sort_params variables:
    //   left_params → points to first half of array
    //   right_params → points to second half
    struct sort_params left_params;
	struct sort_params right_params;

    // Each struct stores:
    //   - pointer to sublist
    //   - number of elements in that sublist
    left_params.size = mid;
	left_params.data = work_array;

	right_params.size = my_size - mid;
	right_params.data = work_array + mid;

    // ------------------------------------------------------
    // Step 5: Create sorting threads
    // ------------------------------------------------------
    // Launch two kernel threads using kthread_run():
    sort_thread1 = kthread_run(sorting_thread_fn, &left_params, "sort_thread1");
    sort_thread2 = kthread_run(sorting_thread_fn, &right_params, "sort_thread2");
    //
    // Always check if kthread_run() returns NULL (failure).
    if (sort_thread1 == NULL)
    {
    	printk(KERN_ERR "[INIT] Thread1 Sort Failure.\n");
    	kfree(work_array);
        return PTR_ERR(sort_thread1);
    }
    if (sort_thread2 == NULL)
    {
    	printk(KERN_ERR "[INIT] Thread2 Sort Failure.\n");
    	kfree(work_array);
        return PTR_ERR(sort_thread2);
    }

    // ------------------------------------------------------
    // Step 6: Create merging thread
    // ------------------------------------------------------
    // Launch the merge thread AFTER starting sorting threads.
    // This thread will merge results from both halves.
    //   merge_thread = kthread_run(merging_thread_fn, NULL, "merge_thread");

    // ------------------------------------------------------
    // Step 7: Print initial data to kernel log
    // ------------------------------------------------------
    // Loop through input and print for debugging.
    //
    // Example:
    // printk(KERN_INFO "[INIT] Input array: ");
    // for (i = 0; i < my_size; i++)
    //     printk(KERN_CONT "%d ", work_array[i]);
    // printk(KERN_CONT "\n");

    // ------------------------------------------------------
    // Step 8: Return success
    // ------------------------------------------------------
    // Return 0 to indicate successful initialization.
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
