//merge_sort.c

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

/*
Example Input
    max_sort_size=8 \
    modules_to_sort=7,3,9,1,4,8,2,6
*/


// -------- Module parameters --------

static int my_size;
module_param(my_size, int, 0444);

static int my_data[1024];
static int my_data_count;
module_param_array(my_data, int, &my_data_count, 0444);

// -------- Types --------
struct sort_params {
    int *ptr_sort_array;
    int sort_array_size = my_size;
};

struct merge_params {
    int *ptr_merged_array;
    int *ptr_left;  int left_array_size;
    int *ptr_right; int right_array_size;
};

static int *work_array;

// -------- Globals --------
static struct task_struct *left_thread;
static struct task_struct *right_thread;
static struct task_struct *merge_thread;

static int *ptr_final_sorted_array;

// -------- Prototypes --------
static int __init mergesort_init(void);
static void __exit mergesort_exit(void);

// -------- Implementations (empty) --------

static void merge(int *ptr_merged_array, int *ptr_left, int left_array_size, int *ptr_right, int right_array_size)
{
    printk(KERN_INFO "[MERGE] Running Merge Function.\n");
    
    //Merge the left and right sides into ptr_sort_array
}

static void split(void)
{
    printk(KERN_INFO "[SPLIT] Running Split Function.\n");

    //Split work_array in half and assign it to the left and right arrays
    

}

static void sort(int *ptr, int array_size)
{
    printk(KERN_INFO "[SORT] Running sort Function.\n");

    //Sort the given array

}


static int __init mergesort_init(void)
{
    /* TODO: validate params, allocate buffers, spawn threads, join, print */
    printk(KERN_INFO "[INIT] MergeSort module loaded.\n");

    //Validate Input
    if (my_size <= 0) {
        printk(KERN_ERR "[INIT] Invalid size parameter.\n");
        return -EINVAL;
    }

    //Allocate Array size for work_array
    work_array = kmalloc(my_size * sizeof(int), GFP_KERNEL);
    if (!work_array) {
        printk(KERN_ERR "[INIT] Memory allocation failed.\n");
        return -ENOMEM;
    }

    //Copy my_data array into work_array
    for (int i = 0; i < my_size; i++){
        work_array[i] = my_data[i];
    }

    //Split array in half
    split();

    //Create thread for left side of array
    //Create thread for right side of array

    //Run both threads to sort their array

    //Wait for threads to end

    //Run merge

    //Output the Array as seen in the assignmnet

    return 0;
}

static void __exit mergesort_exit(void)
{
    printk(KERN_INFO "[EXIT] Exiting MergeSort \n");

    /* TODO: stop threads if needed, free buffers */
    pr_info("[mergesort] exit\n");
}

module_init(mergesort_init);
module_exit(mergesort_exit);


//===========================================================
// Module metadata
//===========================================================
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alexander Boutselis");
MODULE_DESCRIPTION("Multithreaded Merge Sort Kernel Module");