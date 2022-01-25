#include <linux/string.h>
#include <linux/init.h> // For module init and exit
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h> // For fops
#include <linux/uaccess.h>
#include <linux/kthread.h> //kernel threads
#include <linux/slab.h>
//#include <string.h> // Can't use it!

static int account[100];

#define DEVICE_NAME "iut_device"
MODULE_LICENSE("GPL");
#define MAX 256
int arr[100];
spinlock_t iutlock;
// FILE OPERATIONS
static int iut_open(struct inode *, struct file *);
static int iut_release(struct inode *, struct file *);
static ssize_t iut_read(struct file *, char *, size_t, loff_t *);
static ssize_t iut_write(struct file *, const char *, size_t, loff_t *);

static struct file_operations fops = {
    .open = iut_open,
    .read = iut_read,
    .write = iut_write,
    .release = iut_release,
};

// Why "static"? --> To bound it to the current file.
static int major; // Device major number. Driver reacts to this major number.

// Event --> LOAD
static int __init iut_init(void)
{
    major = register_chrdev(0, DEVICE_NAME, &fops); // 0: dynamically assign a major number ||| name is displayed in /proc/devices ||| fops.
    if (major < 0)
    {
        printk(KERN_ALERT "iut_device load failed.\n");
        return major;
    }

    int i = 0;
    for (i = 0; i < 100; i++)
    {
        account[i] = 2000000;
    }

    printk(KERN_INFO "iut_device module loaded: %d\n", major);
    spin_lock_init(&iutlock);
    return 0;
}

// Event --> UNLOAD
static void __exit iut_exit(void)
{
    unregister_chrdev(major, DEVICE_NAME);
    printk(KERN_INFO "iut_device module unloaded.\n");
}

// Event --> OPEN
static int iut_open(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "iut_device opened.\n");
    return 0;
}

// Event --> CLOSE
static int iut_release(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "iut_device closed.\n");
    return 0;
}

// Event --> READ
static ssize_t iut_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
    char str[3024] = "";
    char buff[50];
    int i = 0;
    for (i = 0; i < 100; i++)
    {
        sprintf(buff, "%d,", account[i]);
        strcat(str, buff);
    }

    int errors = 0;
    errors = copy_to_user(buffer, str, strlen(str));
    return errors == 0 ? strlen(str) : -EFAULT;
}

static ssize_t iut_write(struct file *filep, const char *buffer, size_t length, loff_t *offset)
{
    char message[MAX] = ""; ///< Memory for the string that is passed from userspace
    //printk(KERN_INFO "Entered to the write\n");
    if (length > MAX)
        return -EINVAL;

    if (copy_from_user(message, buffer, length) != 0)
        return -EFAULT;

    spin_lock(&iutlock);

    if (strcmp(message, "r") == 0)
    {
        if (strcmp(message, "r") == 0)
        {
            printk(KERN_INFO "Received %s characters from the user\n", message);
        }
    }
    else
    {
        int counter = 0;
        char *found;
        // string = strdup(command);
        char type[2];
        char *copy_string = (char *)kzalloc(sizeof(message) * sizeof(char), GFP_KERNEL);
        strcpy(copy_string, message);
        int from;
        int to;
        int amount;
        //printk(KERN_INFO "entering to fasd\n");
        if (strstr(copy_string, ",") == NULL)
        {
            printk(KERN_ALERT "command Not Found !!! \n");
            return 0;
        }
        while ((found = strsep(&copy_string, ",")) != NULL)
        {
            if (counter == 0)
            {
                strcpy(type, found);
            }
            else if (counter == 1)
            {
                if (strcmp(found, "-") == 0)
                {
                    counter++;
                    continue;
                }
                kstrtoint(found, 10, &from);
            }
            else if (counter == 2)
            {
                if (strcmp(found, "-") == 0)
                {
                    counter++;
                    continue;
                }
                kstrtoint(found, 10, &to);
            }
            else if (counter == 3)
            {
                kstrtoint(found, 10, &amount);
            }
            counter++;
        }
        //printk(KERN_ALERT "counter(%d) \n", counter);
        if (counter != 4)
        {
            printk(KERN_ALERT "command Not Found !!! \n");
        }
        else
        {

            if (strcmp(type, "e") == 0)
            {
                if (from > 100 || from < 0 || to > 100 || to < 0)
                {
                    printk(KERN_ALERT "ID out of Range\n");
                }
                else if (account[from] < amount)
                {
                    printk(KERN_ALERT "account (%d) Don't Have enough  Money\n", from);
                }
                else
                {
                    account[from] -= amount;
                    account[to] += amount;
                }
            }
            else if (strcmp(type, "b") == 0)
            {

                if (from > 100 || from < 0)
                {
                    printk(KERN_ALERT "ID out of Range\n");
                }
                else if (account[from] < amount)
                {
                    printk(KERN_ALERT "account (%d) Don't Have enough  Money\n", from);
                }
                else
                {

                    account[from] -= amount;
                }
            }
            else if (strcmp(type, "v") == 0)
            {
                if (to > 100 || to < 0)
                {
                    printk(KERN_ALERT "ID out of Range\n");
                }
                else
                {
                    account[to] += amount;
                }
            }
            else
            {

                printk(KERN_ALERT "command Not Found !!! \n");
            }
        }
    }
    spin_unlock(&iutlock);

    return 0;
}

// Registering load and unload functions.
module_init(iut_init);
module_exit(iut_exit);
