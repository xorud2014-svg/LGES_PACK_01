/*
 * MBUFF kernel virtual memory driver for Linux,
 * Proc filesystem interface
 *
 * Copyright (C) 1999, David Edwards <wavefrnt@tampabay.rr.com>
 *
 *
 * Much of this code is based on linux/drivers/block/ide-proc.c
 *
 * Copyright (C) 1997-1998, Mark Lord
 *
 */

#include <linux/proc_fs.h>
#include "mbuff.h"
#include "kernel_compat.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0)

static struct proc_dir_entry *root, *regions;

/*
 * Helper function to compute buffer pointers and sizes
 */
static int proc_mbuff_read_return(char *page, char **start, off_t off,
				  int count, int *eof, int len)
{
    len -= off;
    if (len < count)
    {
	*eof = 1;
	if (len <= 0)
	    return 0;
    }
    else
	len = count;
    *start = page + off;
    return len;
}

/*
 *  Read proc to display mbuff version
 */
static int proc_mbuff_read_version(char *page, char **start, off_t off,
				   int count, int *eof, void *data)
{
    char *out = page;
    int len;

    out += sprintf(out, "%s\n", MBUFF_VERSION);
    len = out - page;
    return proc_mbuff_read_return(page, start, off, count, eof, len);
}

/*
 *  Read proc to display size for a region
 */
static int proc_mbuff_read_size(char *page, char **start, off_t off,
				int count, int *eof, void *data)
{
    char *out = page;
    int len;
    struct mbuff *mbuff = (struct mbuff *) data;

    out += sprintf(out, "%ld\n", mbuff->size);
    len = out - page;
    return proc_mbuff_read_return(page, start, off, count, eof, len);
}

/*
 *  Read proc to display user space allocation count for a region
 */
static int proc_mbuff_read_count(char *page, char **start, off_t off,
				 int count, int *eof, void *data)
{
    char *out = page;
    int len;
    struct mbuff *mbuff = (struct mbuff *) data;

    out += sprintf(out, "%d\n", mbuff->count);
    len = out - page;
    return proc_mbuff_read_return(page, start, off, count, eof, len);
}

/*
 *  Read proc to display kernal space allocation count for a region
 */
static int proc_mbuff_read_kcount(char *page, char **start, off_t off,
				  int count, int *eof, void *data)
{
    char *out = page;
    int len;
    struct mbuff *mbuff = (struct mbuff *) data;

    out += sprintf(out, "%d\n", mbuff->kcount);
    len = out - page;
    return proc_mbuff_read_return(page, start, off, count, eof, len);
}

/*
 *  Read proc to display open count for a region
 */
static int proc_mbuff_read_opencnt(char *page, char **start, off_t off,
				   int count, int *eof, void *data)
{
    char *out = page;
    int len;
    struct mbuff *mbuff = (struct mbuff *) data;

    out += sprintf(out, "%d\n", mbuff->open_cnt);
    len = out - page;
    return proc_mbuff_read_return(page, start, off, count, eof, len);
}

/*
 *  Read proc to display open mode for a region
 */
static int proc_mbuff_read_openmode(char *page, char **start, off_t off,
				    int count, int *eof, void *data)
{
    char *out = page;
    int len;
    struct mbuff *mbuff = (struct mbuff *) data;

    out += sprintf(out, "%d\n", mbuff->open_mode);
    len = out - page;
    return proc_mbuff_read_return(page, start, off, count, eof, len);
}

/*
 *  Create an mbuff region's proc entry, called from mbuff_list_add
 */
void proc_mbuff_add_mbuff(struct mbuff *mbuff)
{
    struct proc_dir_entry *region, *ent;

    region = create_proc_entry(mbuff->name, S_IFDIR, regions);

    ent = create_proc_entry("size", 0, region);
    if (!ent)
	return;
    ent->read_proc = proc_mbuff_read_size;
    ent->data = mbuff;

    ent = create_proc_entry("count", 0, region);
    if (!ent)
	return;
    ent->read_proc = proc_mbuff_read_count;
    ent->data = mbuff;

    ent = create_proc_entry("kcount", 0, region);
    if (!ent)
	return;
    ent->read_proc = proc_mbuff_read_kcount;
    ent->data = mbuff;

    ent = create_proc_entry("open_cnt", 0, region);
    if (!ent)
	return;
    ent->read_proc = proc_mbuff_read_opencnt;
    ent->data = mbuff;

    ent = create_proc_entry("open_mode", 0, region);
    if (!ent)
	return;
    ent->read_proc = proc_mbuff_read_openmode;
    ent->data = mbuff;
}

/*
 *  Remove an mbuff region's proc entry, called from mbuff_list_remove
 */
void proc_mbuff_remove_mbuff(struct mbuff *mbuff)
{
    remove_proc_entry(mbuff->name, regions);
}

/*
 *  Create a "directory" in the /proc file system for mbuff
 */
void proc_mbuff_create(void)
{
    struct proc_dir_entry *ent;

    root = create_proc_entry("mbuff", S_IFDIR, 0);
    if (!root)
	return;

    /* regions is a directory containing a directory for each region */
    regions = create_proc_entry("regions", S_IFDIR, root);

    /* regions is a directory containing a directory for each region */
    ent = create_proc_entry("version", 0, root);
    if (!ent)
	return;
    ent->read_proc = proc_mbuff_read_version;

}

/*
 *  Remove the mbuff hierarchy from the /proc file system for mbuff
 */
void proc_mbuff_destroy(void)
{
    remove_proc_entry("mbuff/regions", 0);
    remove_proc_entry("mbuff/version", 0);
    remove_proc_entry("mbuff", 0);
}

#else
void proc_mbuff_create(void)
{
}
void proc_mbuff_destroy(void)
{
}
void proc_mbuff_add_mbuff(struct mbuff *mbuff)
{
}
void proc_mbuff_remove_mbuff(struct mbuff *mbuff)
{
}

#endif
