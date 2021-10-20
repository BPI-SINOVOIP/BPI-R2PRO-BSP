#ifndef SAVE_THREAD_H
#define SAVE_THREAD_H

#include <QThread>

class SaveThread : public QThread
{
	Q_OBJECT

public:
	SaveThread(uchar *buf, int bufLen, uchar *cameraFlag, int saveFrames);
	void run() override;

private:
	uchar *videoBuf;
	uchar flag[5];
	int frame;
	int len;
};

#endif // SAVE_THREAD_H
