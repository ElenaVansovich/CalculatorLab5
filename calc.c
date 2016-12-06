#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("Calculator Module");
MODULE_AUTHOR("Elena Vansovich");
MODULE_VERSION("1");

#define FIRST_PROC "first"
#define SECOND_PROC "second"
#define OPERAND_PROC "operand"
#define RESULT_PROC "result"

#define MAX_SIZE 20

static char first_buf[MAX_SIZE], second_buf[MAX_SIZE], operand_buf[MAX_SIZE];

static unsigned long first_buf_size = 0, second_buf_size = 0, operand_buf_size = 0;

static const int first_ind = 1, second_ind = 2, operand_ind = 3;

static struct proc_dir_entry *first_file, *second_file, *operand_file;


int string_to_int(char *str, int n) 
{
	int len = 0;
	char *s = str;
	int k = 1;
	int res = 0;
	int i = 0;
	int is_negative = 0;

	if (*s == '-') {
		is_negative = 1;
		s++;
		n--;
	}
	while (*s >= '0' && *s <= '9' && len < n) {
		len ++;
		s ++;
	}
	--s;
	for (i = 0; i < len; i++, s--, k *= 10) {
		res += k * (*s - '0');
	}

	if (is_negative) {
		res *= -1;
	}
	return res;
}

int len_of_int(int num)
{
	int len = 0;
	int k = 1;

	if (num == 0) {
		return 1;
	}
	if (num < 0) {
		len++;
		num *= -1;
	}
	while (num / k > 0) {
		len++;
		k *= 10;
	}
	return len;
}

static ssize_t write_proc(struct file *f, const char __user *buf, size_t count, loff_t *data)
{
	char *proc_buf;
	unsigned long* size_buf;
	const unsigned char *name = f->f_path.dentry->d_name.name;


	if (strcmp(name, FIRST_PROC) == 0) {
		printk(KERN_INFO "write_proc first\n");
		proc_buf = first_buf;
		size_buf = &first_buf_size;
	}
	else if (strcmp(name, SECOND_PROC) == 0) {
		printk(KERN_INFO "write_proc second\n");
		proc_buf = second_buf;
		size_buf = &second_buf_size;
	}
	else if (strcmp(name, OPERAND_PROC) == 0) {
		printk(KERN_INFO "write_proc operand\n");
		proc_buf = operand_buf;
		size_buf = &operand_buf_size;
	}
	else {
		return 0;
	}

	*size_buf = count;
	if (*size_buf > MAX_SIZE) {
		*size_buf = MAX_SIZE;
	}

	if (copy_from_user(proc_buf, buf, *size_buf)) {
		return -EFAULT;
	}
	
	return *size_buf;
}

static ssize_t read_dev(struct file * f, char * buf, size_t count, loff_t *ppos)
{
	char result[MAX_SIZE];
	int len = 0;
	int first = string_to_int(first_buf, first_buf_size);
	int second = string_to_int(second_buf, second_buf_size);
	int flag = 1;
	int res = 0;

	if (operand_buf_size == 0) {
		flag = 0;
	}
	else {
		switch (operand_buf[0]) {
			case '-':
				res = first - second;
				break;
			case '+':
				res = first + second;
				break;
			case '*':
				res = first * second;
				break;
			case '/':
				if (second == 0) {
					flag = 0;
					break;
				}
				res = first / second;
				break;
			default:
				flag = 0;
		}
	}
	if (!flag) {
		memcpy(result, "Error!", 6);
		len = 6;
	} 
	else {
		sprintf(result, "%d", res);
		len = len_of_int(res);
	}
	if (count < len) {
		return -EINVAL;
	}
	if (*ppos != 0) {
		return 0;
	}
	if (count < len) {
		return -EINVAL;
	}
	if (*ppos != 0) {
		return 0;
	}
	if (copy_to_user(buf, result, len)) {
		return -EINVAL;
	}
	*ppos = len;
	return len;
}

static struct file_operations proc_file_op = {
	.owner = THIS_MODULE,
	.write = write_proc,
};

static const struct file_operations dev_file_op = {
	.owner = THIS_MODULE,
	.read = read_dev,
};

static struct miscdevice result_dev = {
	MISC_DYNAMIC_MINOR,
	RESULT_PROC,
	&dev_file_op
};


static int calc_init(void)
{		
	printk(KERN_INFO "Calculator module started working\n");

	first_file = proc_create_data(FIRST_PROC, 766, NULL, &proc_file_op, (void*) &first_ind);
	if (first_file == NULL) {
		printk(KERN_ERR "can't create first proc");
		remove_proc_entry(FIRST_PROC, NULL);
		return -ENOMEM;
	}

	second_file = proc_create_data(SECOND_PROC, 766, NULL, &proc_file_op, (void*) &second_ind);
	if (second_file == NULL) {
		printk(KERN_ERR "can't create second proc");
		remove_proc_entry(FIRST_PROC, NULL);
		remove_proc_entry(SECOND_PROC, NULL);
		return -ENOMEM;
	}

	operand_file = proc_create_data(OPERAND_PROC, 766, NULL, &proc_file_op, (void*) &operand_ind);
	if (operand_file == NULL) {
		printk(KERN_ERR "can't create operand proc");
		remove_proc_entry(FIRST_PROC, NULL);
		remove_proc_entry(SECOND_PROC, NULL);
		remove_proc_entry(OPERAND_PROC, NULL);
		return -ENOMEM;
	}

	if (misc_register(&result_dev)) {
		printk(KERN_ERR "unable to register result misc device\n");
		remove_proc_entry(FIRST_PROC, NULL);
		remove_proc_entry(SECOND_PROC, NULL);
		remove_proc_entry(OPERAND_PROC, NULL);
		return -ENOMEM;
	}
	return 0;
}

static void calc_exit(void)
{
	printk(KERN_INFO "Calculator module stopped working\n");
	remove_proc_entry(FIRST_PROC, NULL);
	remove_proc_entry(SECOND_PROC, NULL);
	remove_proc_entry(OPERAND_PROC, NULL);
	misc_deregister(&result_dev);
}


module_init(calc_init);
module_exit(calc_exit);
