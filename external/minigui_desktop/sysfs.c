/*
 * Copyright (c) 2018 rockchip
 *
 */

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define LOG_TAG         "sysfs "

static int _write_sysfs_int(const char *filename, const char *basedir, int val,
                            bool verify)
{
    int ret = 0;
    FILE *sysfsfp;
    int test;
    char *temp = (char *)malloc(strlen(basedir) + strlen(filename) + 2);

    if (!temp)
        return -ENOMEM;

    ret = sprintf(temp, "%s/%s", basedir, filename);
    if (ret < 0)
        goto error_free;

    sysfsfp = fopen(temp, "w");
    if (!sysfsfp)
    {
        ret = -errno;
        printf(LOG_TAG"failed to open %s", temp);
        goto error_free;
    }

    ret = fprintf(sysfsfp, "%d", val);
    if (ret < 0)
    {
        if (fclose(sysfsfp))
            printf(LOG_TAG"%s: Failed to close dir", __func__);

        goto error_free;
    }

    if (fclose(sysfsfp))
    {
        ret = -errno;
        goto error_free;
    }

    if (verify)
    {
        sysfsfp = fopen(temp, "r");
        if (!sysfsfp)
        {
            ret = -errno;
            printf(LOG_TAG"failed to open %s", temp);
            goto error_free;
        }

        if (fscanf(sysfsfp, "%d", &test) != 1)
        {
            ret = errno ? -errno : -ENODATA;
            if (fclose(sysfsfp))
                printf(LOG_TAG"%s: Failed to close dir", __func__);

            goto error_free;
        }

        if (fclose(sysfsfp))
        {
            ret = -errno;
            goto error_free;
        }

        if (test != val)
        {
            printf(LOG_TAG"Possible failure in int write %d to %s/%s",
                   val, basedir, filename);
            ret = -1;
        }
    }

error_free:
    free(temp);
    return ret;
}

int write_sysfs_int(const char *filename, const char *basedir, int val)
{
    return _write_sysfs_int(filename, basedir, val, false);
}

int write_sysfs_int_and_verify(const char *filename, const char *basedir,
                               int val)
{
    return _write_sysfs_int(filename, basedir, val, true);
}

static int _write_sysfs_string(const char *filename, const char *basedir,
                               const char *val, bool verify)
{
    int ret = 0;
    FILE  *sysfsfp;
    char *temp = (char *)malloc(strlen(basedir) + strlen(filename) + 2);

    if (!temp)
    {
        printf(LOG_TAG"Memory allocation failed");
        return -ENOMEM;
    }

    ret = sprintf(temp, "%s/%s", basedir, filename);
    if (ret < 0)
        goto error_free;

    sysfsfp = fopen(temp, "w");
    if (!sysfsfp)
    {
        ret = -errno;
        printf(LOG_TAG"Could not open %s", temp);
        goto error_free;
    }

    ret = fprintf(sysfsfp, "%s", val);
    if (ret < 0)
    {
        if (fclose(sysfsfp))
            printf(LOG_TAG"%s: Failed to close dir", __func__);

        goto error_free;
    }

    if (fclose(sysfsfp))
    {
        ret = -errno;
        goto error_free;
    }

    if (verify)
    {
        sysfsfp = fopen(temp, "r");
        if (!sysfsfp)
        {
            ret = -errno;
            printf(LOG_TAG"Could not open file to verify");
            goto error_free;
        }

        if (fscanf(sysfsfp, "%s", temp) != 1)
        {
            ret = errno ? -errno : -ENODATA;
            if (fclose(sysfsfp))
                printf(LOG_TAG"%s: Failed to close dir", __func__);

            goto error_free;
        }

        if (fclose(sysfsfp))
        {
            ret = -errno;
            goto error_free;
        }

        if (strcmp(temp, val) != 0)
        {
            printf(LOG_TAG"Possible failure in string write of %s "
                   "Should be %s written to %s/%s", temp, val,
                   basedir, filename);
            ret = -1;
        }
    }

error_free:
    free(temp);

    return ret;
}

int write_sysfs_string_and_verify(const char *filename, const char *basedir,
                                  const char *val)
{
    return _write_sysfs_string(filename, basedir, val, true);
}

int write_sysfs_string(const char *filename, const char *basedir,
                       const char *val)
{
    return _write_sysfs_string(filename, basedir, val, false);
}

int read_sysfs_posint(const char *filename, const char *basedir)
{
    int ret;
    FILE  *sysfsfp;
    char *temp = (char *)malloc(strlen(basedir) + strlen(filename) + 2);

    if (!temp)
    {
        printf(LOG_TAG"Memory allocation failed");
        return -ENOMEM;
    }

    ret = sprintf(temp, "%s/%s", basedir, filename);
    if (ret < 0)
        goto error_free;

    sysfsfp = fopen(temp, "r");
    if (!sysfsfp)
    {
        ret = -errno;
        goto error_free;
    }

    errno = 0;
    if (fscanf(sysfsfp, "%d\n", &ret) != 1)
    {
        ret = errno ? -errno : -ENODATA;
        if (fclose(sysfsfp))
            printf(LOG_TAG"%s: Failed to close dir", __func__);

        goto error_free;
    }

    if (fclose(sysfsfp))
        ret = -errno;

error_free:
    free(temp);

    return ret;
}

int read_sysfs_float(const char *filename, const char *basedir, float *val)
{
    int ret = 0;
    FILE  *sysfsfp;
    char *temp = (char *)malloc(strlen(basedir) + strlen(filename) + 2);

    if (!temp)
    {
        printf(LOG_TAG"Memory allocation failed");
        return -ENOMEM;
    }

    ret = sprintf(temp, "%s/%s", basedir, filename);
    if (ret < 0)
        goto error_free;

    sysfsfp = fopen(temp, "r");
    if (!sysfsfp)
    {
        ret = -errno;
        goto error_free;
    }

    errno = 0;
    if (fscanf(sysfsfp, "%f\n", val) != 1)
    {
        ret = errno ? -errno : -ENODATA;
        if (fclose(sysfsfp))
            printf(LOG_TAG"%s: Failed to close dir", __func__);

        goto error_free;
    }

    if (fclose(sysfsfp))
        ret = -errno;

error_free:
    free(temp);

    return ret;
}

int read_sysfs_string(const char *filename, const char *basedir, char *str)
{
    int ret = 0;
    FILE  *sysfsfp;
    char *temp = (char *)malloc(strlen(basedir) + strlen(filename) + 2);

    if (!temp)
    {
        printf(LOG_TAG"Memory allocation failed");
        return -ENOMEM;
    }

    ret = sprintf(temp, "%s/%s", basedir, filename);
    if (ret < 0)
        goto error_free;

    sysfsfp = fopen(temp, "r");
    if (!sysfsfp)
    {
        ret = -errno;
        goto error_free;
    }

    errno = 0;
    if (fscanf(sysfsfp, "%s\n", str) != 1)
    {
        ret = errno ? -errno : -ENODATA;
        if (fclose(sysfsfp))
            printf(LOG_TAG"%s: Failed to close dir", __func__);

        goto error_free;
    }

    if (fclose(sysfsfp))
        ret = -errno;

error_free:
    free(temp);

    return ret;
}

static int calc_digits(int num)
{
    int count = 0;

    while (num != 0)
    {
        num /= 10;
        count++;
    }

    return count;
}
