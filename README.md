# Assignment_3_comp521

# COMP 521/L — Advanced Operating Systems and Lab
## Assignment #3 — Linux Kernel Module for Multithreaded Sorting Application

### **Objective**
Create a Linux kernel module that performs a **multithreaded merge sort** on a list of integers.

---

## 🧠 Overview

You’ll write a kernel module (`mergesort.c`) that:
- Divides a list of integers into two equal parts  
- Sorts each sublist using two separate sorting threads  
- Merges the results into one sorted list using a merging thread  

All operations will occur **inside the kernel**, so testing must be done safely in a **Linux virtual machine**.

---

## 🧩 Environment Setup

**Run everything inside a Linux VM** (VirtualBox, VMware, etc.) to avoid damaging your host OS.

You will use:
- The **Linux kernel headers**  
- A **Makefile** for compilation  
- The `insmod`, `rmmod`, and `dmesg` commands for kernel module management

---

## 📁 Files Required

| File | Purpose |
|------|----------|
| `mergesort.c` | Main kernel module source file |
| `Makefile` | Used to build the kernel object module |

---

## 🧱 mergesort.c — File Structure

Your source file must include the following components:

### 1. **Include Statements**
```c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/moduleparam.h>
```

### 2. **Module Parameters**
You must define:
- `my_size`: integer value for list size  
- `my_data`: dynamic array of integers to be sorted  

Example:
```c
static int my_size;
module_param(my_size, int, 0);
static int my_data[128];
module_param_array(my_data, int, &my_size, 0);
```

---

### 3. **Data Structure**
Define a struct to hold sorting parameters:
```c
struct sort_params {
    int size;
    int *data;
};
```

---

### 4. **Thread Functions**

#### 🧮 Sorting Thread
- Recursively divides the list into halves  
- Calls merge operations on the results  

#### 🔗 Merging Thread
- Combines two sorted sublists into a single sorted list using a merge algorithm  

---

### 5. **Helper Functions**
- `merge()` → Performs merge operation between two sublists  
- `sorting_thread()` → Handles recursive sorting logic  
- `merging_thread()` → Handles the final merge operation  

---

### 6. **Initialization and Exit Functions**

#### `proc_init`
- Copies module parameters into a `sort_params` structure  
- Creates threads for sorting and merging  
- Prints the results to the kernel message buffer  

#### `proc_exit`
- Cleans up memory and exits the module safely  

Example template:
```c
static int __init proc_init(void) {
    printk(KERN_INFO "MergeSort module loaded.\n");
    // Copy parameters, create threads, perform sorting/merging
    return 0;
}

static void __exit proc_exit(void) {
    printk(KERN_INFO "MergeSort module unloaded.\n");
}
```

Register your init and exit functions:
```c
module_init(proc_init);
module_exit(proc_exit);
```

---

## ⚙️ Compilation and Execution Steps

### 1. **Compile the Module**
```bash
make
```
If there are errors, fix them and recompile.

---

### 2. **Insert the Module**
```bash
sudo insmod mergesort.ko my_size=8 my_data=3,5,2,9,1,6,4,8
```

---

### 3. **View Output**
```bash
dmesg
```
To clear old messages before testing:
```bash
sudo dmesg -C
```

---

### 4. **Remove the Module**
```bash
sudo rmmod mergesort
```

---

### 5. **Clean Build Files**
```bash
make clean
```

---

## 🧾 Example Test Run

1. **Compile the module**
   ```bash
   make
   ```
2. **Insert and execute**
   ```bash
   sudo insmod mergesort.ko my_size=10 my_data=9,7,3,5,2,6,1,8,4,0
   ```
3. **Check sorted output**
   ```bash
   dmesg
   ```
4. **Remove module and clear logs**
   ```bash
   sudo rmmod mergesort
   sudo dmesg -C
   ```

---

## 🧹 Expected Output

The kernel log (`dmesg`) should print messages like:
```
[INFO] MergeSort module loaded.
[INFO] Original data: 9, 7, 3, 5, 2, 6, 1, 8, 4, 0
[INFO] Sorted data: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9
[INFO] MergeSort module unloaded.
```

---

## 🧠 Summary

This assignment demonstrates:
- Thread synchronization in the kernel  
- Recursive sorting inside kernel space  
- Safe module loading/unloading procedures  