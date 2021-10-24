#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <linux/videodev2.h>

int device_open(char *dev_name)
{
	struct stat st;
	int fd = -1;

	if (-1 == stat(dev_name, &st)) {
		fprintf(stderr, "Cannot identify '%s': %d, %s\n", dev_name, errno, strerror(errno));
		return fd;
	}
	if (!S_ISCHR(st.st_mode)) {
		fprintf(stderr, "%s is no device\n", dev_name);
		return fd;
	}
	fd = open(dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);
	if (-1 == fd) {
		fprintf(stderr, "Cannot open '%s': %d, %s\n", dev_name, errno, strerror(errno));
		return fd;
	}

	printf("%s %d : Open %s successfully. fd = %d\n", __func__, __LINE__, dev_name, fd);
	return fd;
}

int device_close(int fd)
{
	if (-1 == close(fd)) {
		printf("\tdevice close failed.\n");
		return -1;
	} else {
		printf("%s %d : devices close successfully\n", __func__, __LINE__);
	}

	return 0;
}

int device_query(char *dev_name, int fd, struct v4l2_capability *cap)
{
	/* query v4l2-devices's capability */
	if (-1 == ioctl(fd, VIDIOC_QUERYCAP, cap)) {
		if (EINVAL == errno) {
			fprintf(stderr, "%s is no V4L2 device\n", dev_name);
			exit(EXIT_FAILURE);
		} else {
			printf("\tvideo ioctl_querycap failed.\n");
			exit(EXIT_FAILURE);
		}
	} else {
		printf("\n\tdriver name : %s\n\tcard name : %s\n\tbus info : %s\n\tdriver version : %u.%u.%u\n\n",
			cap->driver, cap->card, cap->bus_info,(cap->version >> 16) & 0XFF,
			(cap->version >> 8) & 0XFF, cap->version & 0XFF);
	}
	return 0;
}

char *dev_list[] = {
	"/dev/video0",	"/dev/video1",	"/dev/video2",	"/dev/video3",	"/dev/video4", // 00 - 04
	"/dev/video5",	"/dev/video6",	"/dev/video7",	"/dev/video8",	"/dev/video9", // 05 - 09
	"/dev/video10",	"/dev/video11",	"/dev/video12",	"/dev/video13",	"/dev/video14", // 10 - 14
};

char* get_device(char *name)
{
	int i = 0;
	int fd;
	struct v4l2_capability cap;

	memset(&cap, 0, sizeof(cap));
	for( i = 0; i < sizeof(dev_list)/sizeof(*dev_list); ++i) {
		int fd = device_open(dev_list[i]);

		/* no video device, break */
		if (fd == -1) {
			printf("no vide device, quit");
			return 0;
		}
		device_query(dev_list[i], fd, &cap);
		device_close(fd);
		if (strstr(cap.driver, name)) {
			printf("get device %s\n", dev_list[i]);
			return dev_list[i];
		}
	}
	return 0;
}
