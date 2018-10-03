#include <linux/module.h>    // included for all kernel modules
#include <linux/kernel.h>    // included for KERN_INFO
#include <linux/init.h>      // included for __init and __exit macros
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/mount.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/signal.h>
#include <linux/pid.h>

#include <asm/segment.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define DEVICE_NAME "project2"
#define MAJOR_NUM 199

#define MB_SHIFT 20
#define KB_SHIFT 10
#define SHMEM_SIZE (128 << MB_SHIFT)

#define BASEPORT 0x700

static struct class *my_class;
static dev_t devt;
unsigned long phys_shared_mem;
unsigned int irq_line;
int a;
int pid;

static irqreturn_t project2_interrupt_handler(int irq, void *dev)
{
	printk(KERN_INFO "project2 interrup handler\n");
	kill_pid(find_vpid(pid), SIGUSR1, 1);

	return IRQ_HANDLED;
}

static int project2_open(struct inode *inode, struct file *file)
{
	pid = (int) task_pid_vnr(current);
/*	
	printk("The process id is %d\n", (int) task_pid_nr(current));
	printk("The process vid is %d\n", (int) task_pid_vnr(current));
	printk("The process name is %s\n", current->comm);
*/	
	return 0;
}

static long project2_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	printk(KERN_INFO "ioctl, cmd: %u\n", cmd);
	
	return 0;
}

static int project2_mmap(struct file *filep, struct vm_area_struct *vma)
{
	unsigned long start = (unsigned long)vma->vm_start; 
	unsigned long size = (unsigned long)(vma->vm_end-vma->vm_start);
	unsigned long pfn;

	/* if userspace tries to mmap beyond end of our buffer, fail */ 
	if (size > SHMEM_SIZE)
		return -EINVAL;
	
	pfn = (phys_shared_mem >> 12);

	while (size > 0) 
	{
		if (remap_pfn_range(vma, start, pfn, PAGE_SIZE, PAGE_SHARED))
			return -EAGAIN;

		start+= PAGE_SIZE;
		pfn++;
		size-= PAGE_SIZE;
	}

	return 0;
}

struct file_operations fops = {
	.open			= project2_open, 
	.mmap			= project2_mmap,
	.unlocked_ioctl = project2_ioctl,
};

static int __init project2_init(void)
{
	int ret;

	printk(KERN_INFO "Init module\n");
	register_chrdev(MAJOR_NUM, DEVICE_NAME, &fops);
	devt = MKDEV(MAJOR_NUM,0);
	my_class = class_create(THIS_MODULE, DEVICE_NAME);
    device_create(my_class, NULL, devt, NULL, DEVICE_NAME);
	
	/* get phys_shared_mem */
	phys_shared_mem = (unsigned long)inl(BASEPORT);
					
	/* get irq line */
	irq_line = inl(BASEPORT + 0x100);

	printk(KERN_INFO "phys_shread_mem: %lu, irq line: %u\n",phys_shared_mem,irq_line); 

	/* register interrupt handler */


	ret = request_irq(irq_line, project2_interrupt_handler, IRQF_SHARED, "project2", &a);
	if (ret)
		printk(KERN_INFO "request_irq fails, ret:%d\n", ret);

	return 0;
}
module_init(project2_init);

static void __exit project2_exit(void)
{
	/* Unregister the device */
	unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
	device_destroy(my_class, devt);
	class_destroy(my_class);
	free_irq(irq_line, &a);
    printk(KERN_INFO "Cleaning up module.\n");
}
module_exit(project2_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mincheol Sung <mincheol@vt.edu>");
MODULE_DESCRIPTION("Project2");
