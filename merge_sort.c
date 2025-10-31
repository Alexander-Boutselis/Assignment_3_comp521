//===========================================================
// Multithreaded Merge Sort Kernel Module (simplified)
//===========================================================

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/moduleparam.h>
#include <linux/kthread.h>
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/string.h>

static int my_size;
module_param(my_size, int, 0444);
MODULE_PARM_DESC(my_size, "Number of elements to sort");

static int my_data[1024];
static int my_data_count;
module_param_array(my_data, int, &my_data_count, 0444);
MODULE_PARM_DESC(my_data, "Comma-separated list of integers to sort");

// -------------------- Globals --------------------
static int *work_array;
static int *left_array, *right_array;
static int left_size, right_size;
static int *final_sorted;

static struct task_struct *left_thread;
static struct task_struct *right_thread;
static struct task_struct *merge_thread;

static DECLARE_COMPLETION(left_done);
static DECLARE_COMPLETION(right_done);
static DECLARE_COMPLETION(merge_done);

// -------------------- Small helpers --------------------
static void print_array_line(const char *label, const int *a, int n)
{
    int i;
    pr_info("%s [ ", label);
    for (i = 0; i < n; ++i) {
        pr_cont("%d", a[i]);
        if (i < n - 1) pr_cont(" ");
    }
    pr_cont(" ]\n");
}

// -------------------- Core merge (with logging) --------------------
static void merge(int *dst, int *L, int nL, int *R, int nR)
{
    int i = 0, j = 0, k = 0, t;

    pr_info("Merging: [ ");
    for (t = 0; t < nL; ++t) {
        pr_cont("%d", L[t]);
        if (t < nL - 1) pr_cont(" ");
    }
    pr_cont(" ] and [ ");
    for (t = 0; t < nR; ++t) {
        pr_cont("%d", R[t]);
        if (t < nR - 1) pr_cont(" ");
    }
    pr_cont(" ]\n");

    while (i < nL && j < nR)
        dst[k++] = (L[i] <= R[j]) ? L[i++] : R[j++];
    while (i < nL) dst[k++] = L[i++];
    while (j < nR) dst[k++] = R[j++];
}

// -------------------- Local mergesort for each half --------------------
static void mergesort_rec(int *arr, int n, int *tmp)
{
    int mid, i, j, k;
    if (n <= 1) return;

    mid = n / 2;
    mergesort_rec(arr, mid, tmp);
    mergesort_rec(arr + mid, n - mid, tmp);

    i = 0; j = mid; k = 0;
    while (i < mid && j < n) tmp[k++] = (arr[i] <= arr[j]) ? arr[i++] : arr[j++];
    while (i < mid) tmp[k++] = arr[i++];
    while (j < n)   tmp[k++] = arr[j++];
    memcpy(arr, tmp, n * sizeof(int));
}

static void sort(int *ptr, int n)
{
    int *tmp;
    if (!ptr || n <= 1) return;
    tmp = kmalloc_array(n, sizeof(int), GFP_KERNEL);
    if (!tmp) return;
    mergesort_rec(ptr, n, tmp);
    kfree(tmp);
}

// -------------------- Threads --------------------
struct sort_params { int *p; int n; struct completion *done; };
struct merge_params {
    int *dst, *L, *R; int nL, nR;
    struct completion *waitL, *waitR, *done;
};

static int sorting_thread_fn(void *data)
{
    struct sort_params *sp = data;
    sort(sp->p, sp->n);
    if (sp->done) complete(sp->done);
    return 0;
}

static int merging_thread_fn(void *data)
{
    struct merge_params *mp = data;

    if (mp->waitL)  wait_for_completion(mp->waitL);
    if (mp->waitR)  wait_for_completion(mp->waitR);

    merge(mp->dst, mp->L, mp->nL, mp->R, mp->nR);

    if (mp->done) complete(mp->done);
    return 0;
}

// -------------------- Split --------------------
static void split(void)
{
    left_size  = my_size / 2;
    right_size = my_size - left_size;

    left_array  = kmalloc_array(left_size, sizeof(int), GFP_KERNEL);
    right_array = kmalloc_array(right_size, sizeof(int), GFP_KERNEL);

    if (!left_array || !right_array)
        return;

    memcpy(left_array,  work_array,             left_size  * sizeof(int));
    memcpy(right_array, work_array + left_size, right_size * sizeof(int));
}

// -------------------- Module init/exit --------------------
static int __init mergesort_init(void)
{
    int i;
    struct sort_params lsp = {0}, rsp = {0};
    struct merge_params mp  = {0};

    if (my_size <= 0 || my_size > 1024 || my_data_count < my_size)
        return -EINVAL;

    work_array = kmalloc_array(my_size, sizeof(int), GFP_KERNEL);
    if (!work_array) return -ENOMEM;
    for (i = 0; i < my_size; ++i) work_array[i] = my_data[i];

    pr_info("Size of list: %d\n", my_size);
    print_array_line("Original list:", work_array, my_size);

    split();
    if (!left_array || !right_array) { kfree(work_array); return -ENOMEM; }

    final_sorted = kmalloc_array(my_size, sizeof(int), GFP_KERNEL);
    if (!final_sorted) { kfree(left_array); kfree(right_array); kfree(work_array); return -ENOMEM; }

    reinit_completion(&left_done);
    reinit_completion(&right_done);
    reinit_completion(&merge_done);

    lsp.p = left_array;   lsp.n = left_size;   lsp.done = &left_done;
    rsp.p = right_array;  rsp.n = right_size;  rsp.done = &right_done;

    mp.dst = final_sorted;
    mp.L = left_array;  mp.nL = left_size;
    mp.R = right_array; mp.nR = right_size;
    mp.waitL = &left_done; mp.waitR = &right_done; mp.done = &merge_done;

    left_thread  = kthread_run(sorting_thread_fn, &lsp, "msort_left");
    right_thread = kthread_run(sorting_thread_fn, &rsp, "msort_right");
    if (IS_ERR(left_thread) || IS_ERR(right_thread)) {
        if (!IS_ERR_OR_NULL(left_thread))  kthread_stop(left_thread);
        if (!IS_ERR_OR_NULL(right_thread)) kthread_stop(right_thread);
        kfree(final_sorted); kfree(left_array); kfree(right_array); kfree(work_array);
        final_sorted = NULL; left_array = right_array = work_array = NULL;
        return -ECHILD;
    }

    merge_thread = kthread_run(merging_thread_fn, &mp, "msort_merge");
    if (IS_ERR(merge_thread)) {
        if (!IS_ERR_OR_NULL(left_thread))  kthread_stop(left_thread);
        if (!IS_ERR_OR_NULL(right_thread)) kthread_stop(right_thread);
        kfree(final_sorted); kfree(left_array); kfree(right_array); kfree(work_array);
        final_sorted = NULL; left_array = right_array = work_array = NULL;
        return -ECHILD;
    }

    /* Wait for full pipeline to finish so stack params are safe to discard. */
    wait_for_completion(&merge_done);

    /* Threads have finished; make exit() a pure free. */
    left_thread = right_thread = merge_thread = NULL;

    print_array_line("Sorted list:", final_sorted, my_size);
    return 0;
}

static void __exit mergesort_exit(void)
{
    /* Nothing to stop; we nulled thread pointers after completion. */
    kfree(final_sorted);   final_sorted = NULL;
    kfree(left_array);     left_array = NULL;
    kfree(right_array);    right_array = NULL;
    kfree(work_array);     work_array = NULL;
    pr_info("mergesort: module removed.\n");
}

module_init(mergesort_init);
module_exit(mergesort_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alexander Boutselis");
MODULE_DESCRIPTION("Multithreaded Merge Sort Kernel Module");
