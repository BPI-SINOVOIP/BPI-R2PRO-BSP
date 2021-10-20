#ifndef QKEYBOARD_H
#define QKEYBOARD_H

//#include "basewidget.h"

#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>

namespace Ui
{
class QKeyBoard;
}

class QKeyBoard : public QWidget
{
    Q_OBJECT
public:
    explicit QKeyBoard(QWidget *parent = nullptr);
    ~QKeyBoard();

    static QKeyBoard* getInstance()
    {
        if (!_instance) {
            _instance = new QKeyBoard;
        }
        return _instance;
    }

    void focusLineEdit(QLineEdit *edit);
    void showPanel();
    void hidePanel();

private:
    Ui::QKeyBoard *ui;
    static QKeyBoard* _instance;
    bool isShiftOn;
    QLineEdit *lineEdit;
    QPoint mousePoint;
    bool mousePressed;
    QRect availableGeometry;

    void changeInputType(bool caps);
    void insertValue(const QString &value);

protected:
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);

private slots:
    void slot_onApplicationFocusChanged(QWidget *, QWidget *);
    void btn_clicked();
};

#endif // QKEYBOARD_H
