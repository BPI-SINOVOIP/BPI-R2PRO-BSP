#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <QtWidgets>
#include <QPainter>
#include <QPaintEngine>
#include <QDateTime>
#include "videoitem.h"

VideoItem::VideoItem(const QRect &rect, QGraphicsItem *parent)
	: QGraphicsObject(parent)
{
#ifdef QT_FB_DRM_RGB565
	rgaFormat = RK_FORMAT_RGB_565;
#else
	rgaFormat = RK_FORMAT_BGRA_8888;
#endif

	//blend = 0xFF0105;
	blend = 0xFF0405;
	displayRect = rect;
	regionRect.setCoords(0, 0, -1, -1);
	infoBox.infoRect.setRect(displayRect.x(), displayRect.height()*4/5, displayRect.width(), displayRect.height()/5);
	infoBox.titleRect = infoBox.infoRect.adjusted(displayRect.width()/100, displayRect.height()/100, 0, 0);
	infoBox.ipRect = infoBox.titleRect.adjusted(displayRect.width()/20, displayRect.height()/20, 0, 0);
	infoBox.timeRect = infoBox.ipRect.adjusted(0, displayRect.height()/25, 0, 0);
	infoBox.nameRect = infoBox.timeRect.adjusted(0, displayRect.height()/20, 0, 0);

	int size = displayRect.width()/5;
	infoBox.snapshotRect = QRectF(displayRect.width() - size - 20,
		infoBox.infoRect.y() + (infoBox.infoRect.height() - size)/2, size, size);

	infoBox.title = tr("人脸识别");

	memset(ip, 0, MAX_IP_LEN);
	memset(&facial, 0, sizeof(struct FacialInfo));
	memset(&video, 0, sizeof(struct VideoInfo));
	facial.boxRect.setCoords(0, 0, -1, -1);
	facial.faceRect.setCoords(0, 0, -1, -1);
	facial.faceBuf = (uchar *)malloc(displayRect.width() * displayRect.height() * 3);
	if(!facial.faceBuf)
		qDebug("faceBuf malloc failed");

	int len = infoBox.infoRect.width() * infoBox.infoRect.height();
#ifdef QT_FB_DRM_ARGB32
	int r = 153, g = 51, b = 250, a = 170;
#else
	int r = 153, g = 51, b = 250, a = 100;
#endif
	int bgra = ((a & 0xFF) << 24) | ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF);
	infoBoxBuf = (int *)malloc(len * sizeof(int));
	if(infoBoxBuf) {
		for(int i = 0; i < len; i++)
			infoBoxBuf[i] = bgra;
	}

	defaultSnapshot.load(":/images/default_user_face.png");
	snapshotThread = new SnapshotThread();

#ifdef BUILD_TEST
	initTestInfo();
#endif
}

VideoItem::~VideoItem()
{
	c_RkRgaDeInit();
	if(infoBoxBuf) {
		free(infoBoxBuf);
		infoBoxBuf = NULL;
	}

	if(facial.faceBuf) {
		free(facial.faceBuf);
		facial.faceBuf = NULL;
	}
}

static void findName(char *fullName, char *name, int nameLen)
{
	char *end, *begin;

	memset(name, 0, nameLen);
	if(strlen(fullName)) {
		begin = strrchr(fullName, '/');
		end = strrchr(fullName, '.');

		if (begin && end && end > begin)
			memcpy(name, begin + 1, end - begin - 1);
		else
			strcpy(name, "unknown_user");
	}
}

static int rgaPrepareInfo(uchar *buf, RgaSURF_FORMAT format, QRectF rect,
				int sw, int sh, rga_info_t *info)
{
	memset(info, 0, sizeof(rga_info_t));

	info->fd = -1;
	info->virAddr = buf;
	info->mmuFlag = 1;

	return rga_set_rect(&info->rect, rect.x(), rect.y(), rect.width(), rect.height(), sw, sh, format);
}

static int rgaDrawImage(uchar *src, RgaSURF_FORMAT src_format, QRectF srcRect,
				int src_sw, int src_sh, uchar *dst, RgaSURF_FORMAT dst_format,
				QRectF dstRect, int dst_sw, int dst_sh, int rotate, unsigned int blend)
{
	static int rgaSupported = 1;
	static int rgaInited = 0;
	rga_info_t srcInfo;
	rga_info_t dstInfo;

	memset(&srcInfo, 0, sizeof(rga_info_t));
	memset(&dstInfo, 0, sizeof(rga_info_t));

	if (!rgaSupported)
		return -1;

	if (!rgaInited) {
		if (c_RkRgaInit() < 0) {
			rgaSupported = 0;
			return -1;
		}

		rgaInited = 1;
	}

	if (rgaPrepareInfo(src, src_format, srcRect, src_sw, src_sh, &srcInfo) < 0)
		return -1;

	if (rgaPrepareInfo(dst, dst_format, dstRect, dst_sw, dst_sh, &dstInfo) < 0)
		return -1;

	srcInfo.rotation = rotate;
	if(blend)
		srcInfo.blend = blend;

	return c_RkRgaBlit(&srcInfo, &dstInfo, NULL);
}

#ifdef BUILD_TEST
void VideoItem::initTestInfo()
{
	testInfo.valid = false;
	memset(&testInfo.testResult, 0, sizeof(struct test_result));

	testInfo.testBox.irDetectRect.setRect(displayRect.width()*2/3 - 70, displayRect.height()/6,
		displayRect.width()/3 + 70, 35);
	testInfo.testBox.irLivenessRect.setRect(testInfo.testBox.irDetectRect.x(),
		testInfo.testBox.irDetectRect.y() + testInfo.testBox.irDetectRect.height() + 10,
		testInfo.testBox.irDetectRect.width(), testInfo.testBox.irDetectRect.height());
	testInfo.testBox.rgbLandmarkRect.setRect(testInfo.testBox.irLivenessRect.x(),
		testInfo.testBox.irLivenessRect.y() + testInfo.testBox.irLivenessRect.height() + 10,
		testInfo.testBox.irLivenessRect.width(), testInfo.testBox.irLivenessRect.height());
	testInfo.testBox.rgbAlignRect.setRect(testInfo.testBox.rgbLandmarkRect.x(),
		testInfo.testBox.rgbLandmarkRect.y() + testInfo.testBox.rgbLandmarkRect.height() + 10,
		testInfo.testBox.rgbLandmarkRect.width(), testInfo.testBox.rgbLandmarkRect.height());
	testInfo.testBox.rgbExtractRect.setRect(testInfo.testBox.rgbAlignRect.x(),
		testInfo.testBox.rgbAlignRect.y() + testInfo.testBox.rgbAlignRect.height() + 10,
		testInfo.testBox.rgbAlignRect.width(), testInfo.testBox.rgbAlignRect.height());
	testInfo.testBox.rgbSearchRect.setRect(testInfo.testBox.rgbExtractRect.x(),
		testInfo.testBox.rgbExtractRect.y() + testInfo.testBox.rgbExtractRect.height() + 10,
		testInfo.testBox.rgbExtractRect.width(), testInfo.testBox.rgbExtractRect.height());
}

void VideoItem::setTesIntfo(struct test_result *testResult)
{
	if(!testResult)
		return;

	mutex.lock();
	memcpy(&testInfo.testResult, testResult, sizeof(struct test_result));

#if 0
	qDebug("ir_detect: %d/%d", testInfo.testResult.ir_detect_ok, testInfo.testResult.ir_detect_total);
	qDebug("ir_liveness: %d/%d", testInfo.testResult.ir_liveness_ok, testInfo.testResult.ir_liveness_total);
	qDebug("rgb_align: %d/%d", testInfo.testResult.rgb_align_ok, testInfo.testResult.rgb_align_total);
	qDebug("rgb_detect: %d/%d", testInfo.testResult.rgb_detect_ok, testInfo.testResult.rgb_detect_total);
	qDebug("rgb_extract: %d/%d", testInfo.testResult.rgb_extract_ok, testInfo.testResult.rgb_extract_total);
	qDebug("rgb_landmark: %d/%d", testInfo.testResult.rgb_landmark_ok, testInfo.testResult.rgb_landmark_total);
	qDebug("rgb_search: %d/%d", testInfo.testResult.rgb_search_ok,testInfo.testResult.rgb_search_total);
#endif

	testInfo.valid = true;
	mutex.unlock();
}

void VideoItem::drawTestInfo(QPainter *painter)
{
	if(!testInfo.valid)
		return;

	QString text;
	QFont font;

	int flags = Qt::AlignTop | Qt::AlignLeft | Qt::TextWordWrap;
	font.setPixelSize(25);

	painter->setPen(QPen(Qt::red, 2));
	painter->setFont(font);

	text = "ir_detect: " + QString::number(testInfo.testResult.ir_detect_ok)
		+ "/" + QString::number(testInfo.testResult.ir_detect_total);
	painter->drawText(testInfo.testBox.irDetectRect, flags, text);

	text = "ir_liveness: " + QString::number(testInfo.testResult.ir_liveness_ok)
		+ "/" + QString::number(testInfo.testResult.ir_liveness_total);
	painter->drawText(testInfo.testBox.irLivenessRect, flags, text);

	text = "rgb_landmark: " + QString::number(testInfo.testResult.rgb_landmark_ok)
		+ "/" + QString::number(testInfo.testResult.rgb_landmark_total);
	painter->drawText(testInfo.testBox.rgbLandmarkRect, flags, text);

	text = "rgb_align: " + QString::number(testInfo.testResult.rgb_align_ok)
		+ "/" + QString::number(testInfo.testResult.rgb_align_total);
	painter->drawText(testInfo.testBox.rgbAlignRect, flags, text);

	text = "rgb_extract: " + QString::number(testInfo.testResult.rgb_extract_ok)
		+ "/" + QString::number(testInfo.testResult.rgb_extract_total);
	painter->drawText(testInfo.testBox.rgbExtractRect, flags, text);

	text = "rgb_search: " + QString::number(testInfo.testResult.rgb_search_ok)
		+ "/" + QString::number(testInfo.testResult.rgb_search_total);
	painter->drawText(testInfo.testBox.rgbSearchRect, flags, text);
}
#endif

void VideoItem::render(uchar *buf, RgaSURF_FORMAT format, int rotate, int width, int height)
{
	mutex.lock();

#if 0
	//printf fps
	static QTime cb_time;
	static int cb_frames = 0;

	if (!cb_frames)
		cb_time.start();

	if(cb_time.elapsed()/1000 >= 10) {
		cb_frames = 0;
		cb_time.restart();
	}

	if(!(++cb_frames % 50))
		printf("+++++ %s FPS: %2.2f (%u frames in %ds)\n",
				__func__, cb_frames * 1000.0 / cb_time.elapsed(),
				cb_frames, cb_time.elapsed() / 1000);
#endif

	video.buf = buf;
	video.format = format;
	video.rotate = rotate;
	video.width = width;
	video.height = height;

	mutex.unlock();
}

QRectF VideoItem::boundingRect() const
{
	return QRectF(displayRect.x(), displayRect.y(), displayRect.width(), displayRect.height());
}

bool VideoItem::setBoxRect(int left, int top, int right, int bottom)
{
	bool ret = false;

	mutex.lock();

	if(facial.boxRect.left() != left || facial.boxRect.top() != top
		|| facial.boxRect.right() != right || facial.boxRect.bottom() != bottom) {
		facial.boxRect.setCoords(left, top, right, bottom);

		if(facial.boxRect.isEmpty())
			clear();

		ret = true;
	}

	mutex.unlock();
	return ret;
}

void VideoItem::setUserInfo(struct user_info *info, bool real)
{
	int len;

	mutex.lock();

	if(info && info->sPicturePath && strlen(info->sPicturePath)) {
		len = strlen(info->sPicturePath) > (NAME_LEN - 1) ? (NAME_LEN - 1) : strlen(info->sPicturePath);
		if(strncmp(facial.fullName, info->sPicturePath, len)) {
			memset(facial.fullName, 0, NAME_LEN);
			strncpy(facial.fullName, info->sPicturePath, len);
		}

		if(info->state == USER_STATE_REAL_REGISTERED_WHITE
			|| info->state == USER_STATE_REAL_REGISTERED_BLACK) {
			if(snapshotThread->setName(info->sPicturePath))
				snapshotThread->start();
		}
	} else {
		if(strlen(facial.fullName))
			memset(facial.fullName, 0, sizeof(NAME_LEN));
	}

	if(info)
		facial.state = info->state;

	facial.real = real;
	mutex.unlock();
}

void VideoItem::setFaceInfo(void *ptr, int fmt, int width, int height, int x, int y, int w, int h)
{
	if(!ptr)
		return;

	if(fmt > RK_FORMAT_YCrCb_420_SP_10B) {
		qDebug("invalid fmt: %d\n", fmt);
		return;
	}

	faceMutex.lock();

	QRectF srcRect(x, y, w, h);
	facial.faceRect.setRect(0, 0, w, h);
	facial.faceFormat = fmt;
	rgaDrawImage(ptr, fmt, srcRect, width, height,
					facial.faceBuf, fmt, facial.faceRect,
					facial.faceRect.width(), facial.faceRect.height(), 0, 0);

	faceMutex.unlock();
}

void VideoItem::setRegion(int x, int y, int w, int h)
{
	mutex.lock();

	if(x == 0 && y == 0 && w == displayRect.width() && h == displayRect.height())
		regionRect.setCoords(0, 0, -1, -1);
	else
		regionRect.setRect(x, y, w, h);

	mutex.unlock();
}

void VideoItem::setIp(char *current_ip)
{
	if(current_ip)
		strcpy(ip, current_ip);
}

char *VideoItem::getIp()
{
	return ip;
}

void VideoItem::clear()
{
	if(snapshotThread)
		snapshotThread->clear();

	memset(facial.fullName, 0, NAME_LEN);
	faceMutex.lock();
	facial.faceRect.setCoords(0, 0, -1, -1);
	faceMutex.unlock();
}

void VideoItem::drawSnapshot(QPainter *painter, QImage *image)
{
	float scale;
	int width;

	if((facial.state == USER_STATE_FAKE) || facial.boxRect.isEmpty())
		goto draw_default;

	if(facial.state == USER_STATE_REAL_UNREGISTERED) {
		faceMutex.lock();

		if(!facial.faceBuf || facial.faceRect.isEmpty()) {
			faceMutex.unlock();
			goto draw_default;
		}

		scale = facial.faceRect.height()/infoBox.snapshotRect.height();
		width = facial.faceRect.width() / scale;
		if(infoBox.snapshotRect.x() + width > displayRect.width())
			width = displayRect.width() - infoBox.snapshotRect.x();

		QRectF dstRect(infoBox.snapshotRect.x(), infoBox.snapshotRect.y(), width, infoBox.snapshotRect.height());
		rgaDrawImage((uchar *)facial.faceBuf, facial.faceFormat, facial.faceRect,
						facial.faceRect.width(), facial.faceRect.height(),
						image->bits(), rgaFormat, dstRect,
						image->width(), image->height(), 0, blend);

		faceMutex.unlock();
		return;
	}

	if(!snapshotThread->snapshotBuf()) {
		if(strlen(facial.fullName)) {
			if(!snapshotThread->isRunning())
				snapshotThread->start();
		}

		goto draw_default;
	}

	scale = snapshotThread->snapshotHeight()/infoBox.snapshotRect.height();
	width = snapshotThread->snapshotWidth() / scale;
	if(infoBox.snapshotRect.x() + width > displayRect.width())
		width = displayRect.width() - infoBox.snapshotRect.x();
	QRectF srcRect(0, 0, snapshotThread->snapshotWidth(), snapshotThread->snapshotHeight());
	QRectF dstRect(infoBox.snapshotRect.x(), infoBox.snapshotRect.y(), width, infoBox.snapshotRect.height());
	rgaDrawImage((uchar *)snapshotThread->snapshotBuf(), snapshotThread->snapshotFormat(), srcRect,
					snapshotThread->snapshotWidth(), snapshotThread->snapshotHeight(),
					image->bits(), rgaFormat, dstRect,
					image->width(), image->height(), 0, blend);
	return;

draw_default:
	painter->drawImage(infoBox.snapshotRect, defaultSnapshot);
}

void VideoItem::drawInfoBox(QPainter *painter, QImage *image)
{
	int flags;
	QFont font;
	char name[NAME_LEN];

	flags = Qt::AlignTop | Qt::AlignLeft | Qt::TextWordWrap;
	painter->setPen(QPen(Qt::white, 2));

#ifdef QT_FB_DRM_RGB565
	painter->setBrush(QColor(153, 51, 250, 100));
	painter->setClipRect(boundingRect());
	painter->drawRect(infoBox.infoRect);
#else
	if(infoBoxBuf) {
		QRectF srcInfo(0, 0, infoBox.infoRect.width(), infoBox.infoRect.height());
		rgaDrawImage((uchar *)infoBoxBuf, rgaFormat, srcInfo,
						infoBox.infoRect.width(), infoBox.infoRect.height(),
						image->bits(), rgaFormat, infoBox.infoRect,
						image->width(), image->height(), 0, blend);
	}
#endif

	if(strlen(ip)) {
		font.setPixelSize(20);
		painter->setFont(font);
		painter->drawText(infoBox.ipRect, flags, QString(ip));
	}

	font.setPixelSize(35);
	font.setBold(true);
	painter->setFont(font);
	painter->drawText(infoBox.titleRect, flags, infoBox.title);

	QDateTime time = QDateTime::currentDateTime();
	QString date = time.toString("yyyy.MM.dd hh:mm:ss");
	//QString date = time.toString("yyyy.MM.dd hh:mm:ss ddd");
	font.setPixelSize(28);
	font.setBold(false);
	painter->setFont(font);
	painter->drawText(infoBox.timeRect, flags, date);

	findName(facial.fullName, name, NAME_LEN);
	if(strlen(name) && !facial.boxRect.isEmpty())
		painter->drawText(infoBox.nameRect, flags, QString(name));

	drawSnapshot(painter, image);
}

void VideoItem::drawBox(QPainter *painter)
{
	int w, h;

	if(facial.boxRect.isEmpty())
		return;

	w = facial.boxRect.right() - facial.boxRect.left();
	h = facial.boxRect.bottom() - facial.boxRect.top();

	switch(facial.state) {
		case USER_STATE_FAKE:
			painter->setPen(QPen(Qt::red, 4));
			break;

		case USER_STATE_REAL_UNREGISTERED:
			painter->setPen(QPen(Qt::blue, 4));
			break;

		case USER_STATE_REAL_REGISTERED_WHITE:
			painter->setPen(QPen(Qt::green, 4));
			break;

		case USER_STATE_REAL_REGISTERED_BLACK:
			painter->setPen(QPen(Qt::black, 4));
			break;
	}

	painter->setBrush(QColor(255, 255, 255, 0));
	painter->drawRect(facial.boxRect.left(), facial.boxRect.top(), w, h);
}

void VideoItem::drawRegion(QPainter *painter)
{
	if(regionRect.isEmpty())
		return;

	QFont font;
	QString title = tr("人脸ROI区域");
	QRectF titleRect = regionRect.adjusted(10, 10, 0, 0);
	int flags = Qt::AlignTop | Qt::AlignLeft | Qt::TextWordWrap;
	font.setPixelSize(40);
	painter->setFont(font);

	painter->setPen(QPen(Qt::yellow, 2));
	painter->drawText(titleRect, flags, title);
	painter->setBrush(QColor(255, 255, 255, 0));
	painter->drawRect(regionRect);
}

void VideoItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
						QWidget *widget)
{
	static int printf_cnt = 1;

	if(!painter->paintEngine())
		return;

	Q_UNUSED(option);
	Q_UNUSED(widget);

	mutex.lock();

#if 0
	//printf fps
	static QTime paint_time;
	static int paint_frames = 0;
	if (!paint_frames)
		paint_time.start();

	if(paint_time.elapsed()/1000 >= 10) {
		paint_frames = 0;
		paint_time.restart();
	}

	if(!(++paint_frames % 50))
		printf("----- %s FPS: %2.2f (%u frames in %ds)\n",
				__func__, paint_frames * 1000.0 / paint_time.elapsed(),
				paint_frames, paint_time.elapsed() / 1000);
#endif

	QImage *image = static_cast<QImage *>(painter->paintEngine()->paintDevice());
	if (image->isNull()) {
		mutex.unlock();
		return;
	}

	// TODO: Parse dst format
	if(printf_cnt) {
		qDebug("image->format: %d", image->format());
		printf_cnt--;
	}

	QRectF srcRect(0, 0, video.width, video.height);
	QRectF dstRect(0, 0, image->width(), image->height());

#ifdef ONE_PLANE
	rgaDrawImage(video.buf, video.format, srcRect, video.width, video.height,
					image->bits(), rgaFormat, dstRect, image->width(),
					image->height(), video.rotate, 0);
#endif

	const QVector<QRect> rects = painter->paintEngine()->systemClip().rects();
	for (QVector<QRect>::const_iterator it = rects.begin(); it != rects.end(); ++it) {
		if((it->y() + it->height()) > infoBox.infoRect.y()) {
			drawInfoBox(painter, image);
		}
	}

	drawRegion(painter);

#ifdef BUILD_TEST
	drawTestInfo(painter);
#endif

	video.buf = NULL;
	mutex.unlock();
}
