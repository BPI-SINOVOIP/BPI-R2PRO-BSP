#ifndef DESKTOPVIEW_H
#define DESKTOPVIEW_H

#include <QGraphicsView>
#include <QGroupBox>
#include <QPushButton>
#include <QTouchEvent>
#include <QLineEdit>
#include <rkfacial/rkfacial.h>

#include "videoitem.h"
#include "qtkeyboard.h"

typedef enum {
	ISP,
	CIF
} CAMERA_TYPE;

class DesktopView : public QGraphicsView
{
	Q_OBJECT

public:
	static DesktopView *desktopView;

	explicit DesktopView(int faceCnt, int refresh, QWidget *parent = 0);
	virtual ~DesktopView();

protected:
	bool event(QEvent *event) override;
	bool eventFilter(QObject *obj, QEvent *e) override;

private:
	QWidget *menuWidget;
	QWidget *setWidget;
	QWidget *editWidget;

	QPushButton *switchBtn;
	QPushButton *registerBtn;
	QPushButton *deleteBtn;
	QPushButton *saveBtn;
	QPushButton setBtn;
	QPushButton closeBtn;
	QPushButton editSetBtn;

	QLineEdit *ipEdit;
	QLineEdit *netmaskEdit;
	QLineEdit *gatewayEdit;

	int refreshFrame;
	int saveFrames;
	bool saving;
	bool updateFace;

	QRect desktopRect;
	VideoItem *videoItem;

	CAMERA_TYPE cameraType;
	QTimer *timer;
	QTimer *faceTimer;

	QKeyBoard *keyBoard;

#ifdef BUILD_TEST
	QWidget *testWidget;
	QPushButton *collectBtn;
	QPushButton *realBtn;
	QPushButton *photoBtn;

	bool testing;
	void initTestUi();
	static void paintTestInfo(struct test_result *test);
#endif

	void initUi();
	void initEditUi();
	void initTimer();
	void iniSignalSlots();

	int initRkfacial(int faceCnt);
	void deinitRkfacial();

	void saveFile(uchar *buf, int len, uchar *flag);
	void updateUi(int x, int y, int w, int h);

	static void paintBox(int left, int top, int right, int bottom);
	static void paintInfo(struct user_info *info, bool real);
	static void paintFace(void *ptr, int fmt, int width, int height, int x, int y, int w, int h);
	static void configRegion(int x, int y, int w, int h);

	static void displayRgb(void *src_ptr, int src_fd, int src_fmt, int src_w, int src_h, int rotation);
	static void displayIr(void *src_ptr, int src_fd, int src_fmt, int src_w, int src_h, int rotation);

signals:
	void itemDirty(int x, int y, int w, int h);

private slots:
	void timerTimeOut();
	void faceTimerTimeOut();

	void cameraSwitch();
	void registerSlots();
	void deleteSlots();
	void saveSlots();
	void updateScene(int x, int y, int w, int h);
	void setSlots();
	void closeSlots();
	void editSetSlots();


#ifdef BUILD_TEST
	void saveAllSlots();
	void saveRealSlots();
	void saveFakeSlots();
#endif
};

#endif // DESKTOPVIEW_H
