#include <stdio.h>
#include <QString>
#include <QDir>
#include <QFile>
#include <QDateTime>

#include "savethread.h"

SaveThread::SaveThread(uchar *buf, int bufLen, uchar *cameraFlag, int saveFrames)
{
	memset(flag, 0, 5);
	if(cameraFlag) {
		int flagLen = strlen(cameraFlag) > 5 ? 5 : strlen(cameraFlag);
		memcpy(flag, cameraFlag, flagLen);
	}

	frame = saveFrames;
	videoBuf = buf;
	len = bufLen;
}

static bool createDir(const QString &path)
{
	QDir dir(path);
	if(dir.exists())
		return true;
	else
		return dir.mkpath(path);
}

void SaveThread::run()
{
	QString path = "/data/";
	QString dir = QString(QLatin1String(flag));
	path += dir;
	if(!createDir(path))
		return;

	QDateTime time = QDateTime::currentDateTime();
	QString date = time.toString("/yyyy.MM.dd-hh:mm:ss.zzz");
	path += date;

	QFile file(path);
	if(file.open(QIODevice::WriteOnly)) {
		file.write(videoBuf, len);
		file.close();
	}
}