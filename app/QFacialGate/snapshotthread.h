#ifndef SNAPSHOT_THREAD_H
#define SNAPSHOT_THREAD_H

#include <QThread>
#include <QMutex>

#include <rga/rga.h>

#ifndef NAME_LEN
#define NAME_LEN 256
#endif

class SnapshotThread : public QThread
{
	Q_OBJECT

public:
	SnapshotThread();
	void run() override;
	bool setName(char *name);
	int snapshotWidth();
	int snapshotHeight();
	int snapshotBytesPerLine();
	RgaSURF_FORMAT snapshotFormat();
	char *snapshotBuf();
	void clear();

private:
	char *snapshot;
	char fullName[NAME_LEN];
	int width;
	int height;
	int bytes;
	QMutex mutex;
};

#endif // SNAPSHOT_THREAD_H