//===========================================================
// COMP 521/L - Assignment 3: Multithreaded Merge Sort Module
// File: mergesort.c
// Description:
//   A Linux kernel module that sorts an integer array using
//   multiple threads (two sorting threads + one merging thread).
//===========================================================

#include <linux/module.h>       
#include <linux/kernel.h>       
#include <linux/slab.h>         
#include <linux/moduleparam.h>  
#include <linux/kthread.h>      
#include <linux/delay.h>        
#include <linux/mutex.h>        
#include <linux/completion.h>   

/*
Example usage
  sudo insmod mergesort.ko my_size=8 my_data=7,3,9,1,4,8,2,6
  dmesg | tail -n 200
  sudo rmmod mergesort
*/

// -------- Module parameters --------
static int my_size;                         // number of elements to sort
module_param(my_size, int, 0444);
MODULE_PARM_DESC(my_size, "Number of elements in my_data to sort");

static int my_data[1024];                   // raw input elements (max 1024)
static int my_data_count;                   // how many ints were provided
module_param_array(my_data, int, &my_data_count, 0444);
MODULE_PARM_DESC(my_data, "Comma-separated list of integers to sort");

// -------- Types --------
struct sort_params {
    int *ptr_sort_array;                    // pointer to the sub-array to sort
    int  sort_array_size;                   // length of the sub-array
    struct completion *done;                // completion to signal when done
};

struct merge_params {
    int *ptr_merged_array;                  // output array for merged result
    int *ptr_left;  int left_array_size;    // left, already sorted
    int *ptr_right; int right_array_size;   // right, already sorted
    struct completion *wait_left;           // wait for left sorter
    struct completion *wait_right;          // wait for right sorter
    struct completion *done;                // signal when merge finishes
};

// -------- Globals --------
static int *work_array;                     // copy of input (unsorted)
static int *left_array, *right_array;       // halves after split
static int left_size, right_size;

static int *ptr_final_sorted_array;         // final sorted output buffer

static struct task_struct *left_thread;
static struct task_struct *right_thread;
static struct task_struct *merge_thread;

// Completions for sync
static DECLARE_COMPLETION(left_done);
static DECLARE_COMPLETION(right_done);
static DECLARE_COMPLETION(merge_done);

// -------- Prototypes --------
static int __init mergesort_init(void);
static void __exit mergesort_exit(void);

static void merge(int *ptr_merged_array,
                  int *ptr_left,  int left_array_size,
                  int *ptr_right, int right_array_size);
static void split(void);
static void sort(int *ptr, int array_size);

static int sorting_thread_fn(void *data);
static int merging_thread_fn(void *data);

// -------- Utilities --------
//static void print_array(const char *tag, const int *a, int n)
//{
//    int i;
//    printk(KERN_INFO "%s [", tag);
//    for (i = 0; i < n; ++i) {
//        printk(KERN_INFO "%s%d%s",
//               " ", a[i], (i == n - 1) ? " " : ",");
//    }
//    printk(KERN_INFO "]\n");
//}


static void print_array(const char *prefix, const int *a, int n)
{
    int i;
    pr_info("%s", prefix);   // start (no newline)
    pr_cont(" [");
    for (i = 0; i < n; ++i) {
        if (i) pr_cont(", ");
        pr_cont("%d", a[i]);
    }
    pr_cont("]\n");
}



// -------- Implementations --------
static void merge(int *ptr_merged_array,
                  int *ptr_left,  int left_array_size,
                  int *ptr_right, int right_array_size)
{
    int i = 0, j = 0, k = 0;
    int t;

    /* Print: Merging: [ ... ] and [ ... ] */
    pr_info("Merging: [ ");
    for (t = 0; t < left_array_size; ++t) {
        pr_cont("%d", ptr_left[t]);
        if (t < left_array_size - 1)
            pr_cont(" ");
    }
    pr_cont(" ] and [ ");
    for (t = 0; t < right_array_size; ++t) {
        pr_cont("%d", ptr_right[t]);
        if (t < right_array_size - 1)
            pr_cont(" ");
    }
    pr_cont(" ]\n");

    /* Merge two sorted arrays into ptr_merged_array */
    while (i < left_array_size && j < right_array_size) {
        if (ptr_left[i] <= ptr_right[j])
            ptr_merged_array[k++] = ptr_left[i++];
        else
            ptr_merged_array[k++] = ptr_right[j++];
    }
    while (i < left_array_size)
        ptr_merged_array[k++] = ptr_left[i++];
    while (j < right_array_size)
        ptr_merged_array[k++] = ptr_right[j++];
}


static void split(void)
{
    // Split work_array into two halves and allocate left/right buffers
    left_size  = my_size / 2;
    right_size = my_size - left_size;

    left_array  = kmalloc_array(left_size, sizeof(int), GFP_KERNEL);
    right_array = kmalloc_array(right_size, sizeof(int), GFP_KERNEL);
    if (!left_array || !right_array) {
        printk(KERN_ERR "[SPLIT] Allocation failed for sub-arrays.\n");
        return;
    }

    // Copy halves
    memcpy(left_array,  work_array,               left_size  * sizeof(int));
    memcpy(right_array, work_array + left_size,   right_size * sizeof(int));
    printk(KERN_INFO "[SPLIT] left_size=%d right_size=%d\n", left_size, right_size);
}

// Recursive mergesort on a single array buffer
static void mergesort_rec(int *arr, int n, int *tmp)
{
    int mid, i, j, k;
    if (n <= 1) return;

    mid = n / 2;
    mergesort_rec(arr, mid, tmp);
    mergesort_rec(arr + mid, n - mid, tmp);

    // Merge arr[0..mid-1] and arr[mid..n-1] into tmp[0..n-1]
    i = 0; j = mid; k = 0;
    while (i < mid && j < n)
        tmp[k++] = (arr[i] <= arr[j]) ? arr[i++] : arr[j++];
    while (i < mid)
        tmp[k++] = arr[i++];
    while (j < n)
        tmp[k++] = arr[j++];

    // Copy back
    memcpy(arr, tmp, n * sizeof(int));
}

static void sort(int *ptr, int array_size)
{
    int *tmp;
    if (!ptr || array_size <= 1) return;
    tmp = kmalloc_array(array_size, sizeof(int), GFP_KERNEL);
    if (!tmp) {
        printk(KERN_ERR "[SORT] Temp allocation failed. Using insertion sort fallback.\n");
        // Tiny safe fallback to avoid failing silently
        for (int i = 1; i < array_size; ++i) {
            int key = ptr[i], m = i - 1;
            while (m >= 0 && ptr[m] > key) { ptr[m+1] = ptr[m]; m--; }
            ptr[m+1] = key;
        }
        return;
    }
    mergesort_rec(ptr, array_size, tmp);
    kfree(tmp);
}

// ---- Thread functions ----
static int sorting_thread_fn(void *data)
{
    struct sort_params *p = (struct sort_params *)data;
    if (!p || !p->ptr_sort_array || p->sort_array_size <= 0) {
        printk(KERN_ERR "[THREAD:sort] Invalid params.\n");
        if (p && p->done) complete(p->done);
        return -EINVAL;
    }
    printk(KERN_INFO "[THREAD:sort] Sorting %d elements...\n", p->sort_array_size);
    sort(p->ptr_sort_array, p->sort_array_size);
    if (p->done) complete(p->done);
    return 0;
}

static int merging_thread_fn(void *data)
{
    struct merge_params *m = (struct merge_params *)data;
    if (!m || !m->ptr_merged_array || !m->ptr_left || !m->ptr_right) {
        printk(KERN_ERR "[THREAD:merge] Invalid params.\n");
        if (m && m->done) complete(m->done);
        return -EINVAL;
    }
    // Wait for the two sorting threads to complete
    if (m->wait_left)  wait_for_completion(m->wait_left);
    if (m->wait_right) wait_for_completion(m->wait_right);

    printk(KERN_INFO "[THREAD:merge] Merging L=%d, R=%d...\n",
           m->left_array_size, m->right_array_size);

    merge(m->ptr_merged_array,
          m->ptr_left,  m->left_array_size,
          m->ptr_right, m->right_array_size);

    if (m->done) complete(m->done);
    return 0;
}

// -------- Module init/exit --------
static int __init mergesort_init(void)
{
    int i;
    struct sort_params left_params = {
        .ptr_sort_array = NULL,
        .sort_array_size = 0,
        .done = &left_done,
    };
    struct sort_params right_params = {
        .ptr_sort_array = NULL,
        .sort_array_size = 0,
        .done = &right_done,
    };
    struct merge_params mparams = {
        .ptr_merged_array = NULL,
        .ptr_left = NULL, .left_array_size = 0,
        .ptr_right = NULL, .right_array_size = 0,
        .wait_left = &left_done,
        .wait_right = &right_done,
        .done = &merge_done,
    };

    printk(KERN_INFO "[INIT] MergeSort module loaded. my_size=%d my_data_count=%d\n",
           my_size, my_data_count);

    // Validate input
    if (my_size <= 0 || my_size > 1024) {
        printk(KERN_ERR "[INIT] Invalid size parameter: %d\n", my_size);
        return -EINVAL;
    }
    if (my_data_count < my_size) {
        printk(KERN_ERR "[INIT] Provided my_data (%d) smaller than my_size (%d).\n",
               my_data_count, my_size);
        return -EINVAL;
    }

    // Allocate and copy input into work_array
    work_array = kmalloc_array(my_size, sizeof(int), GFP_KERNEL);
    if (!work_array) {
        printk(KERN_ERR "[INIT] Memory allocation failed for work_array.\n");
        return -ENOMEM;
    }
    for (i = 0; i < my_size; i++)
        work_array[i] = my_data[i];

    printk(KERN_INFO "[INPUT] Original array:");
    print_array("[INPUT]", work_array, my_size);

    // Split input into two independent buffers
    split();
    if (!left_array || !right_array) {
        printk(KERN_ERR "[INIT] split() failed, aborting.\n");
        kfree(work_array);
        work_array = NULL;
        return -ENOMEM;
    }

    // Prepare final output buffer
    ptr_final_sorted_array = kmalloc_array(my_size, sizeof(int), GFP_KERNEL);
    if (!ptr_final_sorted_array) {
        printk(KERN_ERR "[INIT] Allocation failed for final array.\n");
        kfree(left_array);  left_array = NULL;
        kfree(right_array); right_array = NULL;
        kfree(work_array);  work_array = NULL;
        return -ENOMEM;
    }

    // Fill thread parameter blocks
    reinit_completion(&left_done);
    reinit_completion(&right_done);
    reinit_completion(&merge_done);

    left_params.ptr_sort_array  = left_array;
    left_params.sort_array_size = left_size;

    right_params.ptr_sort_array  = right_array;
    right_params.sort_array_size = right_size;

    mparams.ptr_merged_array = ptr_final_sorted_array;
    mparams.ptr_left = left_array;   mparams.left_array_size  = left_size;
    mparams.ptr_right = right_array; mparams.right_array_size = right_size;

    // Launch two sorting threads
    left_thread  = kthread_run(sorting_thread_fn,  &left_params,  "msort_left");
    right_thread = kthread_run(sorting_thread_fn,  &right_params, "msort_right");
    if (IS_ERR(left_thread) || IS_ERR(right_thread)) {
        printk(KERN_ERR "[INIT] Failed to start sort threads.\n");
        if (!IS_ERR_OR_NULL(left_thread))  kthread_stop(left_thread);
        if (!IS_ERR_OR_NULL(right_thread)) kthread_stop(right_thread);
        kfree(ptr_final_sorted_array);
        kfree(left_array); kfree(right_array); kfree(work_array);
        ptr_final_sorted_array = NULL; left_array = right_array = work_array = NULL;
        return -ECHILD;
    }

    // Launch merging thread (waits on completions internally)
    merge_thread = kthread_run(merging_thread_fn, &mparams, "msort_merge");
    if (IS_ERR(merge_thread)) {
        printk(KERN_ERR "[INIT] Failed to start merge thread.\n");
        // Stop sort threads and bail out
        if (left_thread)  kthread_stop(left_thread);
        if (right_thread) kthread_stop(right_thread);
        kfree(ptr_final_sorted_array);
        kfree(left_array); kfree(right_array); kfree(work_array);
        ptr_final_sorted_array = NULL; left_array = right_array = work_array = NULL;
        return -ECHILD;
    }

    // Wait for merge to complete
    wait_for_completion(&merge_done);

    printk(KERN_INFO "[OUTPUT] Sorted array:");
    print_array("[OUTPUT]", ptr_final_sorted_array, my_size);

    return 0;
}

static void __exit mergesort_exit(void)
{
    printk(KERN_INFO "[EXIT] Exiting MergeSort. Cleaning up...\n");

    // Threads should be finished by now (merge_done waited). But in case
    // the module gets removed unexpectedly early, try to stop threads.
    if (!IS_ERR_OR_NULL(left_thread))  kthread_stop(left_thread),  left_thread = NULL;
    if (!IS_ERR_OR_NULL(right_thread)) kthread_stop(right_thread), right_thread = NULL;
    if (!IS_ERR_OR_NULL(merge_thread)) kthread_stop(merge_thread), merge_thread = NULL;

    kfree(ptr_final_sorted_array); ptr_final_sorted_array = NULL;
    kfree(left_array);             left_array = NULL;
    kfree(right_array);            right_array = NULL;
    kfree(work_array);             work_array = NULL;

    printk(KERN_INFO "[EXIT] MergeSort module removed.\n");
}

module_init(mergesort_init);
module_exit(mergesort_exit);

//===========================================================
// Module metadata
//===========================================================
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alexander Boutselis");
MODULE_DESCRIPTION("Multithreaded Merge Sort Kernel Module");