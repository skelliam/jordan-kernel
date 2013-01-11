/* drivers/misc/smooth_ui.c
 *
 * Copyright 2012  Ezekeel
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *
 * Based on the FSync_control by Ezekeel, changed by Blechdose for Motorola Defy
 */

#include <linux/init.h>
#include <linux/device.h>
#include <linux/miscdevice.h>

#define SMOOTHUI_VERSION 1

static bool smooth_ui_enabled = true;

bool smooth_ui(void)
{
	return smooth_ui_enabled;
}

EXPORT_SYMBOL(smooth_ui);

static ssize_t smooth_ui_status_read(struct device * dev, struct device_attribute * attr, char * buf)
{
    return sprintf(buf, "%u\n", (smooth_ui_enabled ? 1 : 0));
}

static ssize_t smooth_ui_status_write(struct device * dev, struct device_attribute * attr, const char * buf, size_t size)
{
    unsigned int data;

    if(sscanf(buf, "%u\n", &data) == 1) 
	{
	    if (data == 1) 
		{
		    pr_info("%s: SMOOTH_UI enabled\n", __FUNCTION__);

		    smooth_ui_enabled = true;

		} 
	    else if (data == 0) 
		{
		    pr_info("%s: FSMOOTH_UI disabled\n", __FUNCTION__);

		    smooth_ui_enabled = false;
		} 
	    else 
		{
		    pr_info("%s: invalid input range %u\n", __FUNCTION__, data);
		}
	} 
    else 
	{
	    pr_info("%s: invalid input\n", __FUNCTION__);
	}

    return size;
}

static ssize_t smoothui_version(struct device * dev, struct device_attribute * attr, char * buf)
{
    return sprintf(buf, "%u\n", SMOOTHUI_VERSION);
}

static DEVICE_ATTR(smooth_ui_enabled, S_IRUGO | S_IWUGO, smooth_ui_status_read, smooth_ui_status_write);
static DEVICE_ATTR(version, S_IRUGO , smoothui_version, NULL);

static struct attribute *smoothui_attributes[] = 
    {
	&dev_attr_smooth_ui_enabled.attr,
	&dev_attr_version.attr,
	NULL
    };

static struct attribute_group smoothui_group = 
    {
	.attrs  = smoothui_attributes,
    };

static struct miscdevice smoothui_device = 
    {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "smoothui",
    };

static int __init smoothui_init(void)
{
    int ret;

    pr_info("%s misc_register(%s)\n", __FUNCTION__, smoothui_device.name);

    ret = misc_register(&smoothui_device);

    if (ret) 
	{
	    pr_err("%s misc_register(%s) fail\n", __FUNCTION__, smoothui_device.name);
	    return 1;
	}

    if (sysfs_create_group(&smoothui_device.this_device->kobj, &smoothui_group) < 0) 
	{
	    pr_err("%s sysfs_create_group fail\n", __FUNCTION__);
	    pr_err("Failed to create sysfs group for device (%s)!\n", smoothui_device.name);
	}

    return 0;
}

device_initcall(smoothui_init);
