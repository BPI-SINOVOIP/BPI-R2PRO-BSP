#include <rkfacial/turbojpeg_decode.h>
#include "snapshotthread.h"

SnapshotThread::SnapshotThread()
{
	memset(fullName, 0, NAME_LEN);
	snapshot = NULL;
	width = 0;
	height = 0;
	bytes = 0;
}

bool SnapshotThread::setName(char *name)
{
	if(!name)
		return false;

	int len = strlen(name) > (NAME_LEN - 1) ? (NAME_LEN - 1) : strlen(name);
	if(strncmp(fullName, name, len)) {
		mutex.lock();

		if(snapshot) {
			turbojpeg_decode_put(snapshot);
			snapshot = NULL;
		}

		memset(fullName, 0, NAME_LEN);
		strncpy(fullName, name, len);

		mutex.unlock();
		return true;
	}

	return false;
}

int SnapshotThread::snapshotWidth(){
	return width;
}

int SnapshotThread::snapshotHeight()
{
	return height;
}

int SnapshotThread::snapshotBytesPerLine()
{
	return bytes;
}

RgaSURF_FORMAT SnapshotThread::snapshotFormat()
{
	if(bytes == 2)
		return RK_FORMAT_RGB_565;
	else if(bytes == 3)
		return RK_FORMAT_RGB_888;
	else
		return RK_FORMAT_UNKNOWN;
}

char *SnapshotThread::snapshotBuf()
{
	if(isRunning())
		return NULL;

	return snapshot;
}

void SnapshotThread::clear()
{
	mutex.lock();

	if(snapshot) {
		turbojpeg_decode_put(snapshot);
		snapshot = NULL;
	}
	memset(fullName, 0, NAME_LEN);

	mutex.unlock();
}

void SnapshotThread::run()
{
	mutex.lock();

	if(snapshot) {
		turbojpeg_decode_put(snapshot);
		snapshot = NULL;
	}

	snapshot = (char *)turbojpeg_decode_get(fullName, &width, &height, &bytes);
	if(!snapshot)
		qDebug("SnapshotThread::run: turbojpeg_decode_get failed");

	mutex.unlock();
}
