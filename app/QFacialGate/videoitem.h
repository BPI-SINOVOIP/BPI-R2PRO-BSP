#ifndef VIDEOITEM_H
#define VIDEOITEM_H

#include <QGraphicsObject>
#include <QMutex>
#include <QImage>

#include <rga/rga.h>
#include <rga/RgaApi.h>
#include <rkfacial/rkfacial.h>
#include "snapshotthread.h"

#ifndef NAME_LEN
#define NAME_LEN 256
#endif

#define MAX_IP_LEN 20

struct VideoInfo
{
	uchar *buf;
	RgaSURF_FORMAT format;
	int rotate;
	int width;
	int height;
};

struct FacialInfo
{
	QRect boxRect;
	char fullName[NAME_LEN];
	bool real;
	enum user_state state;

	//for setFaceInfo
	uchar *faceBuf;
	QRect faceRect;
	int faceFormat;
};

struct InfoBox
{
	QRectF infoRect;
	QRectF titleRect;
	QRectF ipRect;
	QRectF timeRect;
	QRectF nameRect;
	QRectF snapshotRect;
	QString title;
};

#ifdef BUILD_TEST
struct TestBox
{
	QRectF irDetectRect;
	QRectF irLivenessRect;
	QRectF rgbAlignRect;
	QRectF rgbExtractRect;
	QRectF rgbLandmarkRect;
	QRectF rgbSearchRect;
};

struct TestInfo
{
	bool valid;
	struct TestBox testBox;
	struct test_result testResult;
};
#endif

class VideoItem : public QGraphicsObject
{
	Q_OBJECT

public:
	struct FacialInfo facial;

	VideoItem(const QRect &rect, QGraphicsItem *parent = 0);
	virtual ~VideoItem();

	QRectF boundingRect() const override;

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0) override;


	void render(uchar *buf, RgaSURF_FORMAT format, int rotate,
			int width, int height);

	bool setBoxRect(int left, int top, int right, int bottom);

	void setUserInfo(struct user_info *info, bool real);

	void setFaceInfo(void *ptr, int fmt, int width, int height, int x, int y, int w, int h);

	void setRegion(int x, int y, int w, int h);

	void setIp(char *current_ip);
	char *getIp();

	void clear();

#ifdef BUILD_TEST
	void setTesIntfo(struct test_result *testResult);
#endif

private:
	QRect displayRect;
	QRect regionRect;
	struct VideoInfo video;
	struct InfoBox infoBox;
	int *infoBoxBuf;
	char ip[MAX_IP_LEN];

	RgaSURF_FORMAT rgaFormat;
	unsigned int blend;

	QImage defaultSnapshot;
	SnapshotThread *snapshotThread;

	QMutex mutex;
	QMutex faceMutex;

#ifdef BUILD_TEST
	struct TestInfo testInfo;
	void initTestInfo();
	void drawTestInfo(QPainter *painter);
#endif

	void drawInfoBox(QPainter *painter, QImage *image);
	void drawBox(QPainter *painter);
	void drawSnapshot(QPainter *painter, QImage *image);
	void drawRegion(QPainter *painter);
};

#endif // VIDEOITEM_H
