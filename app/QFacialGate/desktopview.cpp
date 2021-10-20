#include <stdio.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <arpa/inet.h>

#include <QtWidgets>
#include <QTouchEvent>
#include <QList>
#include <QTime>
#include <QMessageBox>

#include "dbserver.h"
#include <rkfacial/rkfacial.h>
#ifdef TWO_PLANE
#include <rkfacial/display.h>
#include <rkfacial/draw_rect.h>
#endif
#include "savethread.h"
#include "desktopview.h"

#define CAMERA_WIDTH 1280
#define CAMERA_HEIGHT 720
#define SAVE_FRAMES 30

DesktopView *DesktopView::desktopView = nullptr;

static int getLocalIp(char *interface, char *ip, int ip_len)
{
	int sd;
	struct sockaddr_in sin;
	struct ifreq ifr;

	memset(ip, 0, ip_len);
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (-1 == sd) {
		//qDebug("socket error: %s\n", strerror(errno));
		return -1;
	}

	strncpy(ifr.ifr_name, interface, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = 0;

	if (ioctl(sd, SIOCGIFADDR, &ifr) < 0) {
		//qDebug("ioctl error: %s\n", strerror(errno));
		close(sd);
		return -1;
	}

	memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
	sprintf(ip, "%s", inet_ntoa(sin.sin_addr));

	close(sd);
	return 0;
}

void DesktopView::initTimer()
{
	timer = new QTimer;
	timer->setSingleShot(false);
	timer->start(3000); //ms
	connect(timer, SIGNAL(timeout()), this, SLOT(timerTimeOut()));

	faceTimer = new QTimer;
	faceTimer->setSingleShot(false);
	faceTimer->start(1000); //ms
	connect(faceTimer, SIGNAL(timeout()), this, SLOT(faceTimerTimeOut()));
}

void DesktopView::timerTimeOut()
{
#if 0
	static QTime t_time;
	static int cnt = 1;

	if(cnt) {
		t_time.start();
		cnt--;
	}

	qDebug("%s: %ds", __func__, t_time.elapsed() / 1000);
#endif
	char ip[MAX_IP_LEN];

	getLocalIp("eth0", ip, MAX_IP_LEN);
	if(!strcmp(ip, videoItem->getIp()))
		return;

	videoItem->setIp(ip);
}

void DesktopView::faceTimerTimeOut()
{
	updateFace = true;

#ifdef TWO_PLANE
	if(videoItem->isVisible())
		updateUi(0, desktopRect.height()*4/5, desktopRect.width(), desktopRect.height()/5);
#endif
}

bool DesktopView::event(QEvent *event)
{
	switch(event->type()) {
#if 0 //when support mouse, the touch event is also converted to a mouse event
		case QEvent::TouchBegin:
		case QEvent::TouchUpdate:
		case QEvent::TouchEnd:
		{
			QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);
			QList<QTouchEvent::TouchPoint> touchPoints = touchEvent->touchPoints();
			if(touchPoints.count() != 1)
				break;

			const QTouchEvent::TouchPoint &touchPoint = touchPoints.first();
			switch (touchPoint.state()) {
				case Qt::TouchPointStationary:
				case Qt::TouchPointReleased:
					// don't do anything if this touch point hasn't moved or has been released
					break;
				default:
				{
					bool flag;
					QRectF rect = touchPoint.rect();
					if (rect.isEmpty())
						break;

					bool settingFlag = rect.y() > menuWidget->height()
						&& rect.y() < (desktopRect.height()*4/5 - 110);
#ifdef BUILD_TEST
					bool testFlag = rect.x() > testWidget->width()
						|| (rect.y() < testWidget->y()
						|| rect.y() > testWidget->y() + testWidget->height());

					if(testWidget->isVisible())
						flag = settingFlag && testFlag;
					else
#endif
						flag = settingFlag;

					if(flag) {
						if(menuWidget->isVisible())
							menuWidget->setVisible(false);
						else
							menuWidget->setVisible(true);
#ifdef BUILD_TEST
						if(testWidget->isVisible())
							testWidget->setVisible(false);
						else
							testWidget->setVisible(true);
#endif
					}
					break;
				}
			}
			break;
		}
#endif
		case QEvent::MouseButtonPress:
		{
			if(editWidget->isVisible())
				break;

			if(menuWidget->isVisible())
				menuWidget->setVisible(false);
			else
				menuWidget->setVisible(true);
#ifdef BUILD_TEST
			if(testWidget->isVisible())
				testWidget->setVisible(false);
			else
				testWidget->setVisible(true);

#endif
			break;
		}

		default:
			break;
	}

	return QGraphicsView::event(event);
}

bool DesktopView::eventFilter(QObject *obj, QEvent *e)
{
	if((obj == ipEdit || obj == netmaskEdit || obj == gatewayEdit)
		&& e->type() == QEvent::MouseButtonPress){
		if(keyBoard->isHidden())
			keyBoard->showPanel();

		keyBoard->focusLineEdit((QLineEdit*)obj);
	}
	return QGraphicsView::eventFilter(obj,e);
}

static void setButtonFormat(QBoxLayout *layout, QPushButton *btn, QRect rect)
{
	bool two_plane = false;

#ifdef TWO_PLANE
	two_plane = true;
#endif

	if(!two_plane) {
		btn->setFixedSize(rect.width()/4, rect.width()/10);
		btn->setStyleSheet("QPushButton{font-size:30px}");
	}
	layout->addWidget(btn);
}

#ifdef BUILD_TEST
void DesktopView::paintTestInfo(struct test_result *test)
{
	desktopView->videoItem->setTesIntfo(test);
	desktopView->collectBtn->setEnabled(true);
	desktopView->realBtn->setEnabled(true);
	desktopView->photoBtn->setEnabled(true);
	desktopView->testing = false;
}

void DesktopView::initTestUi()
{
	QVBoxLayout *vLayout = new QVBoxLayout;
	vLayout->setMargin(0);
	vLayout->setSpacing(0);

	collectBtn = new QPushButton(tr("开始采集"));
	setButtonFormat(vLayout, collectBtn, desktopRect);
	realBtn = new QPushButton(tr("测试真人"));
	setButtonFormat(vLayout, realBtn, desktopRect);
	photoBtn = new QPushButton(tr("测试照片"));
	setButtonFormat(vLayout, photoBtn, desktopRect);

	testWidget = new QWidget();
	testWidget->setLayout(vLayout);
	//testWidget->setObjectName("testWidget");
	//testWidget->setStyleSheet("#testWidget {background-color:rgba(10, 10, 10,100);}");
	testWidget->setWindowOpacity(0.8);
	testWidget->setGeometry(0, desktopRect.height()/3, 0, desktopRect.width()/10*3);

	connect(collectBtn, SIGNAL(clicked()), this, SLOT(saveAllSlots()));
	connect(realBtn, SIGNAL(clicked()), this, SLOT(saveFakeSlots()));
	connect(photoBtn, SIGNAL(clicked()), this, SLOT(saveRealSlots()));

	testing = false;
	register_get_test_callback(paintTestInfo);
}

static void setSaveIrFlag(bool real, bool fake)
{
	save_ir_real(real);
	save_ir_fake(fake);
}

void DesktopView::saveAllSlots()
{
	if(testing)
		return;

	bool equal = collectBtn->text().compare("开始采集", Qt::CaseSensitive);

	if(!equal) {
		setSaveIrFlag(true, true);
		collectBtn->setText(tr("结束采集"));
		realBtn->setEnabled(false);
		photoBtn->setEnabled(false);
	} else {
		setSaveIrFlag(false, false);
		collectBtn->setText(tr("开始采集"));
		realBtn->setEnabled(true);
		photoBtn->setEnabled(true);
	}
}

void DesktopView::saveRealSlots()
{
	setSaveIrFlag(true, false);
	rockface_start_test();

	testing = true;
	collectBtn->setEnabled(false);
	realBtn->setEnabled(false);
	photoBtn->setEnabled(false);
}

void DesktopView::saveFakeSlots()
{
	setSaveIrFlag(false, true);
	rockface_start_test();

	testing = true;
	collectBtn->setEnabled(false);
	realBtn->setEnabled(false);
	photoBtn->setEnabled(false);
}
#endif

void DesktopView::updateUi(int x, int y, int w, int h)
{
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
		printf("+++++ %s FPS: %2.2f (%u frames in %ds)\n",
				__func__, paint_frames * 1000.0 / paint_time.elapsed(),
				paint_frames, paint_time.elapsed() / 1000);
#endif

	emit itemDirty(x, y, w, h);
}

void DesktopView::cameraSwitch()
{
	if(cameraType == ISP) {
		videoItem->setBoxRect(0, 0, -1, -1);
		switchBtn->setText(tr("IR"));
		cameraType = CIF;

#ifdef TWO_PLANE
		display_switch(DISPLAY_VIDEO_IR);
#endif
	} else {
		switchBtn->setText(tr("RGB"));
		cameraType = ISP;

#ifdef TWO_PLANE
		display_switch(DISPLAY_VIDEO_RGB);
#endif
	}
}

void DesktopView::registerSlots()
{
	rkfacial_register();
}

void DesktopView::deleteSlots()
{
	rkfacial_delete();
	videoItem->clear();
}

void DesktopView::saveSlots()
{
	saving = true;
	saveBtn->setEnabled(false);
	switchBtn->setEnabled(false);
}

void DesktopView::updateScene(int x, int y, int w, int h)
{
	scene()->update(x, y, w, h);
	update(x, y, w, h);
}

void DesktopView::setSlots()
{
	if(videoItem->isVisible()) {
		videoItem->setVisible(false);
		setWidget->setVisible(false);
		menuWidget->setVisible(false);
		editWidget->setVisible(true);

#ifdef BUILD_TEST
		if(testWidget->isVisible())
			testWidget->setVisible(false);
#endif
	}
}

void DesktopView::closeSlots()
{
	if(editWidget->isVisible()) {
		videoItem->setVisible(true);
		setWidget->setVisible(true);
		editWidget->setVisible(false);
		keyBoard->hidePanel();
	}
}

static bool checkAddress(QString address, bool isNetmask)
{
	int p0, p1, p2, p3;
	int maxP0;

	QList<QString> list = address.split('.');

	if(list.count() != 4)
		return true;

	p0 = list[0].toInt();
	p1 = list[1].toInt();
	p2 = list[2].toInt();
	p3 = list[3].toInt();

	if(isNetmask)
		maxP0 = 255;
	else
		maxP0 = 233;

	if(p0 < 0 || p0 > maxP0
		|| p1 < 0 || p1 > 255
		|| p2 < 0 || p2 > 255
		|| p3 < 0 || p3 > 255)
		return true;

	return false;
}

void DesktopView::editSetSlots()
{
	bool check;

	if(checkAddress(ipEdit->text(), false)) {
		QMessageBox::warning(NULL, "warning", "Invalid IP");
		return;
	}

	if(checkAddress(netmaskEdit->text(), true)) {
		QMessageBox::warning(NULL, "warning", "Invalid Netmask");
		return;
	}

	if(checkAddress(gatewayEdit->text(), false)) {
		QMessageBox::warning(NULL, "warning", "Invalid Gateway");
		return;
	}

	dbserver_network_ipv4_set("eth0", "manual", qPrintable(ipEdit->text()),
		qPrintable(netmaskEdit->text()), qPrintable(gatewayEdit->text()));
}

void DesktopView::initUi()
{
	QHBoxLayout *hLayout = new QHBoxLayout;
	hLayout->setMargin(0);
	hLayout->setSpacing(0);

	registerBtn = new QPushButton(tr("Register"));
	setButtonFormat(hLayout, registerBtn, desktopRect);
	switchBtn = new QPushButton(tr("RGB"));
	setButtonFormat(hLayout, switchBtn, desktopRect);
#ifdef ONE_PLANE
	saveBtn = new QPushButton(tr("Capture"));
	setButtonFormat(hLayout, saveBtn, desktopRect);
#endif
	deleteBtn = new QPushButton(tr("Delete"));
	setButtonFormat(hLayout, deleteBtn, desktopRect);

	menuWidget = new QWidget();
	menuWidget->setLayout(hLayout);
	//menuWidget->setObjectName("menuWidget");
	//menuWidget->setStyleSheet("#menuWidget {background-color:rgba(10, 10, 10, 100);}");
	menuWidget->setWindowOpacity(0.8);
	menuWidget->setGeometry(0, 0, desktopRect.width(), desktopRect.width()/10);

	int btnSize = desktopRect.width()/15;
	QIcon setIcon;
	setIcon.addFile(":/images/icon_set.png");
	setBtn.setIcon(setIcon);
	setBtn.setIconSize(QSize(btnSize, btnSize));

	QHBoxLayout *setLayout = new QHBoxLayout;
	setLayout->setMargin(0);
	setLayout->setSpacing(0);
	setLayout->addWidget(&setBtn);
	setWidget = new QWidget;
	setWidget->setLayout(setLayout);
	setWidget->setGeometry(desktopRect.width() - btnSize * 2,
		desktopRect.height()*4/5 - btnSize * 2, btnSize, btnSize);

	initEditUi();
}

void DesktopView::initEditUi()
{
	QIcon setIcon;
	setIcon.addFile(":/images/icon_close.png");
	int btnSize = desktopRect.width()/15;
	closeBtn.setIcon(setIcon);
	closeBtn.setIconSize(QSize(btnSize, btnSize));
	closeBtn.setFixedSize(btnSize, btnSize);

	int editSize = desktopRect.width()/10;
	ipEdit = new QLineEdit();
	ipEdit->setFixedHeight(editSize);
	ipEdit->setPlaceholderText(tr("Please enter IP"));
	ipEdit->installEventFilter(this);

	netmaskEdit = new QLineEdit();
	netmaskEdit->setFixedHeight(editSize);
	netmaskEdit->setPlaceholderText(tr("Please enter Netmask"));
	netmaskEdit->installEventFilter(this);

	gatewayEdit = new QLineEdit();
	gatewayEdit->setFixedHeight(editSize);
	gatewayEdit->setPlaceholderText(tr("Please enter Gateway"));
	gatewayEdit->installEventFilter(this);

	int editWidgetWidth = (desktopRect.width() - desktopRect.width()/7);
	editSetBtn.setText(tr("Setting"));
	editSetBtn.setFixedSize(editWidgetWidth/3, editSize);

	QVBoxLayout *editLayout = new QVBoxLayout;
	editLayout->setContentsMargins(20, 0, 20, 0);
	editLayout->addWidget(&closeBtn, 0, Qt::AlignRight);
	editLayout->addWidget(ipEdit);
	editLayout->addWidget(netmaskEdit);
	editLayout->addWidget(gatewayEdit);
	editLayout->addWidget(&editSetBtn, 0, Qt::AlignHCenter);

	editWidget = new QWidget;
	editWidget->setLayout(editLayout);
	editWidget->setGeometry(desktopRect.width()/14, desktopRect.height()/6, editWidgetWidth, desktopRect.height()/3);
}

void DesktopView::iniSignalSlots()
{
	connect(switchBtn, SIGNAL(clicked()), this, SLOT(cameraSwitch()));
	connect(registerBtn, SIGNAL(clicked()), this, SLOT(registerSlots()));
	connect(deleteBtn, SIGNAL(clicked()), this, SLOT(deleteSlots()));
#ifdef ONE_PLANE
	connect(saveBtn, SIGNAL(clicked()), this, SLOT(saveSlots()));
#endif
	connect(this, SIGNAL(itemDirty(int, int, int, int)), this, SLOT(updateScene(int, int, int, int)));
	connect(&setBtn, SIGNAL(clicked()), this, SLOT(setSlots()));
	connect(&closeBtn, SIGNAL(clicked()), this, SLOT(closeSlots()));
	connect(&editSetBtn, SIGNAL(clicked()), this, SLOT(editSetSlots()));
}

static bool coordIsVaild(int left, int top, int right, int bottom)
{
	if(left < 0 || top < 0 || right < 0 || bottom < 0) {
		qDebug("%s: invalid rect(%d, %d, %d, %d)", __func__, left, top, right, bottom);
		return false;
	}

	if(left > right || top > bottom) {
		qDebug("%s: invalid rect(%d, %d, %d, %d)", __func__, left, top, right, bottom);
		return false;
	}

	return true;
}

void DesktopView::paintBox(int left, int top, int right, int bottom)
{
	bool ret;
	int mod = 0;
	static int refreshCount = 0;

	if(desktopView->cameraType == CIF) {
#ifdef TWO_PLANE
		display_paint_box(0, 0, 0, 0);
#endif
		return;
	}

	if(desktopView->refreshFrame) {
		mod = refreshCount % desktopView->refreshFrame;

		refreshCount++;
		if(refreshCount == 65535)
			refreshCount = 0;

		if(mod)
			return;
	}

	if(!coordIsVaild(left, top, right, bottom))
		return;

	if(!left && !top && !right && !bottom) {
		ret = desktopView->videoItem->setBoxRect(0, 0, -1, -1);
		goto update_paint;
	}

	ret = desktopView->videoItem->setBoxRect(left, top, right, bottom);

update_paint:
#ifdef TWO_PLANE
	if(ret) {
		display_paint_box(left, top, right, bottom);
		switch(desktopView->videoItem->facial.state) {
			case USER_STATE_REAL_UNREGISTERED:
				display_set_color(set_yuv_color(COLOR_B));
				break;
			case USER_STATE_REAL_REGISTERED_WHITE:
				display_set_color(set_yuv_color(COLOR_G));
				break;
			case USER_STATE_REAL_REGISTERED_BLACK:
				display_set_color(set_yuv_color(COLOR_BK));
				break;
			default:
				display_set_color(set_yuv_color(COLOR_R));
				break;
		}
	}
#endif
	return;
}

void DesktopView::paintInfo(struct user_info *info, bool real)
{
	if(desktopView->cameraType == CIF)
		return;

	desktopView->videoItem->setUserInfo(info, real);
	desktopView->updateUi(0, 0, desktopView->desktopRect.width(), desktopView->desktopRect.height());
}

void DesktopView::paintFace(void *ptr, int fmt, int width, int height, int x, int y, int w, int h)
{
	if(desktopView->updateFace) {
		desktopView->videoItem->setFaceInfo(ptr, fmt, width, height, x, y, w, h);
		desktopView->updateFace = false;
		desktopView->updateUi(0, 0, desktopView->desktopRect.width(), desktopView->desktopRect.height());
	}
}

void DesktopView::configRegion(int x, int y, int w, int h)
{
	desktopView->videoItem->setRegion(x, y, w, h);
	desktopView->updateUi(0, 0, desktopView->desktopRect.width(), desktopView->desktopRect.height());
}

void DesktopView::saveFile(uchar *buf, int len, uchar *flag)
{
	if(!saving)
		return;

	if(saveFrames) {
		SaveThread *thread = new SaveThread(buf, len, flag, saveFrames);
		thread->start();
		saveFrames--;
	} else {
		saveFrames = SAVE_FRAMES;
		saving = false;
		saveBtn->setEnabled(true);
		switchBtn->setEnabled(true);
	}
}

void DesktopView::displayRgb(void *src_ptr, int src_fd, int src_fmt, int src_w, int src_h, int rotation)
{
	if(desktopView->cameraType != ISP)
		return;

	if (src_fmt != RK_FORMAT_YCbCr_420_SP) {
		qDebug("%s: src_fmt = %d", __func__, src_fmt);
		return;
	}

	desktopView->saveFile((uchar *)src_ptr, src_w * src_h * 3 / 2, "rgb");

	//qDebug("%s, tid(%lu)\n", __func__, pthread_self());
	desktopView->videoItem->render((uchar *)src_ptr, src_fmt, rotation,
						src_w, src_h);
	desktopView->updateUi(0, 0, desktopView->desktopRect.width(), desktopView->desktopRect.height());
}

void DesktopView::displayIr(void *src_ptr, int src_fd, int src_fmt, int src_w, int src_h, int rotation)
{
	if(desktopView->cameraType != CIF)
		return;

	if (src_fmt != RK_FORMAT_YCbCr_420_SP) {
		qDebug("%s: src_fmt = %d", __func__, src_fmt);
		return;
	}

	desktopView->saveFile((uchar *)src_ptr, src_w * src_h * 3 / 2, "ir");

	desktopView->videoItem->render((uchar *)src_ptr, src_fmt, rotation,
						src_w, src_h);
	desktopView->updateUi(0, 0, desktopView->desktopRect.width(), desktopView->desktopRect.height());
}

static int DesktopView::initRkfacial(int faceCnt)
{
#ifdef TWO_PLANE
	set_rgb_param(CAMERA_WIDTH, CAMERA_HEIGHT, NULL, true);
	set_ir_param(CAMERA_WIDTH, CAMERA_HEIGHT, NULL);
	set_rgb_rotation(90);

	display_switch(DISPLAY_VIDEO_RGB);
	if (display_init(desktopRect.width(), desktopRect.height())) {
		qDebug("%s: display_init failed", __func__);
		return -1;
	}
#else
	set_rgb_param(CAMERA_WIDTH, CAMERA_HEIGHT, displayRgb, true);
	set_ir_param(CAMERA_WIDTH, CAMERA_HEIGHT, displayIr);
#endif

	set_face_param(CAMERA_WIDTH, CAMERA_HEIGHT, faceCnt);

	register_rkfacial_paint_box(paintBox);
	register_rkfacial_paint_info(paintInfo);
	register_rkfacial_paint_face(paintFace);
	register_get_face_config_region(configRegion);

	if(rkfacial_init() < 0) {
		qDebug("%s: rkfacial_init failed", __func__);
		return -1;
	}
	return 0;
}

void DesktopView::deinitRkfacial()
{
	rkfacial_exit();

#ifdef TWO_PLANE
	display_exit();
#endif
}

DesktopView::DesktopView(int faceCnt, int refresh, QWidget *parent)
	: QGraphicsView(parent)
{
	desktopView = this;
	cameraType = ISP;
	saveFrames = SAVE_FRAMES;
	saving = false;
	updateFace = false;
	refreshFrame = refresh;

#ifdef TWO_PLANE
	this->setStyleSheet("background: transparent");
#endif

	desktopRect = QApplication::desktop()->availableGeometry();
	qDebug("DesktopView Rect(%d, %d, %d, %d)", desktopRect.x(), desktopRect.y(),
		desktopRect.width(), desktopRect.height());

	resize(desktopRect.width(), desktopRect.height());
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	setAttribute(Qt::WA_AcceptTouchEvents, true);
	//qApp->setAttribute(Qt::AA_SynthesizeTouchForUnhandledMouseEvents, false);
	//qApp->setAttribute(Qt::AA_SynthesizeMouseForUnhandledTouchEvents, false);

	videoItem = new VideoItem(desktopRect);
	videoItem->setZValue(0);

	keyBoard = QKeyBoard::getInstance();

	initUi();
	iniSignalSlots();

	QGraphicsScene *scene = new QGraphicsScene(this);
	scene->setItemIndexMethod(QGraphicsScene::NoIndex);
	scene->addItem(videoItem);
	scene->addWidget(menuWidget);
	scene->addWidget(setWidget);
	scene->addWidget(editWidget);
	scene->addWidget(keyBoard);
	menuWidget->setVisible(false);
	editWidget->setVisible(false);

#ifdef BUILD_TEST
	initTestUi();
	scene->addWidget(testWidget);
	testWidget->setVisible(false);
#endif

	scene->setSceneRect(scene->itemsBoundingRect());
	setScene(scene);

	initTimer();
	initRkfacial(faceCnt);
}

DesktopView::~DesktopView()
{
	deinitRkfacial();
	timer->stop();
	faceTimer->stop();

	if(keyBoard)
		delete keyBoard;
}
