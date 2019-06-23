/**
 * NT Fullscreen Capture Overlay
 * @author denBot@github
 * @date 23/06/2019
 */

#include <iostream>
#include <QtGui>
#include <QApplication>
#include <QWidget>
#include <QDesktopWidget>
#include <QPushButton>

using namespace std;

/** Represents the region of screen selected by user */
struct magnifier { int x, y, wh, zoom; };
struct mag_coordinates { int x, y; QString text; Qt::AlignmentFlag alignment; };


/**
 * Implementation of a full-screen overlay used for screen-capturing purposes.
 *
 * The overlay is updated/redrawn on various mouse-driven events, such as when the mouse is moved, pressed and released.
 * On mouse release, this overlay will close and print x,y,w,h to the console.
 */
class Overlay : public QWidget
{
private:
    QRect res;
    QRect currScreen;
    QRect region;
    QPixmap desktop;
    int cursorX;
    int cursorY;
    int startX;
    int startY;
    struct mag_coordinates coordinates;
    struct magnifier mag = { 0, 0, 160, 20 };
    bool pressed = false;
public:
    void start() {

        res = {0,0,0,0};
        cursorX = startX = QCursor::pos().x();
        cursorY = startY = QCursor::pos().y();
        currScreen = QApplication::screenAt(QCursor::pos())->geometry();

        for (QScreen* screen : QApplication::screens()) {
            QRect sg = screen->geometry();
            if (sg.x() < res.x()) res.setX(sg.x());
            if (sg.y() < res.y()) res.setY(sg.y());
            if (sg.x()+sg.width() > res.width()) res.setWidth(sg.x()+sg.width());
            if (sg.y()+sg.height() > res.height()) res.setHeight(sg.y()+sg.height());
        }

        setGeometry(res);

        printf("Overlay Resolution: %d %d %d %d\n", res.x(), res.y(), res.width(), res.height());

        desktop = QPixmap::grabWindow(QApplication::desktop()->winId());

        setMouseTracking(true);
        setCursor(Qt::CrossCursor);
        setWindowFlags(Qt::WindowStaysOnTopHint | Qt::BypassWindowManagerHint);

        update();
        show();

    }
protected:
    void drawMagnifier(QPainter* painter) {
        coordinates.x = mag.x;
        coordinates.y = cursorY+5;
        coordinates.alignment = Qt::AlignRight;
        mag.x = cursorX-mag.wh-20;
        mag.y = cursorY+20;

        if (pressed) {
            coordinates.text.sprintf("X:%d Y:%d W:%d H:%d", cursorX, cursorY, region.width(), region.height());
        } else {
            coordinates.text.sprintf("X:%d Y:%d", cursorX, cursorY);
        }

        if (mag.x < currScreen.x()) {
            mag.x = cursorX+20;
            coordinates.x = mag.x;
            coordinates.alignment = Qt::AlignLeft;
        }

        if (mag.y + mag.wh > currScreen.y()+currScreen.height()) {
            mag.y = cursorY-20-mag.wh;
            coordinates.y = cursorY-15;
        }

        // Draw coordinates text under magnifying glass
        painter->setPen(QColor(255, 255, 255, 200));
        painter->drawText(coordinates.x, coordinates.y, mag.wh, 20, coordinates.alignment, coordinates.text);

        // Draw outline of pixmap
        painter->setBrush(QColor(30,30,30));
        painter->drawRect(mag.x-1, mag.y-1, mag.wh+1, mag.wh+1);

        // Draw zoomed pixmap with cross grid
        painter->drawPixmap(mag.x, mag.y, mag.wh, mag.wh, desktop, cursorX-(mag.zoom/2), cursorY-(mag.zoom/2), mag.zoom, mag.zoom);
        painter->setBrush(QBrush(QColor(0, 0, 0, 80), Qt::CrossPattern));
        painter->drawRect(mag.x-1, mag.y-1, mag.wh+1, mag.wh+1);

        // Draw white centre lines and dot
        painter->setPen(QPen(QColor(255, 255, 255, 60), 3));
        painter->drawLine(mag.x+(mag.wh/2), mag.y, mag.x+(mag.wh/2), mag.y+mag.wh);
        painter->drawLine(mag.x, mag.y+(mag.wh/2), mag.x+mag.wh, mag.y+(mag.wh/2));
        painter->drawRect(mag.x+(mag.wh/2)-2, mag.y+(mag.wh/2)-2, 4, 4);
    }
    void paintEvent(QPaintEvent *) override {
        QPainter painter(this);

        currScreen = QApplication::screenAt(QCursor::pos())->geometry();

        //painter.setRenderHint(QPainter::Antialiasing);
        painter.setFont(QFont("Arial", 9));
        painter.setPen(Qt::transparent);
        painter.drawPixmap(0,0,desktop);

        if (pressed) {
            // calculate region co-ordinates of section selected by user

            region = {
                    startX > cursorX ? cursorX : startX,
                    startY > cursorY ? cursorY : startY,
                    startX > cursorX ? startX - cursorX : cursorX - startX,
                    startY > cursorY ? startY - cursorY : cursorY - startY
            };

            // Draw darkened area around selection
            painter.setBrush(QColor(0, 0, 0, 100));
            painter.drawRect(0, 0, region.x(), height());
            painter.drawRect(region.x(), 0, region.width(), region.y());
            painter.drawRect(region.x(), region.y()+region.height(), region.width(), height());
            painter.drawRect(region.x()+region.width(), 0, width()-region.x(), height());

            painter.setBrush(Qt::transparent);
            painter.setPen(QColor(255, 255, 255, 50));
            painter.drawRect(region.x(), region.y(), region.width(), region.height());

            painter.drawLine(0, startY, width(), startY);
            painter.drawLine(startX, 0, startX, height());
        } else {
            // Darkened screen overlay
            painter.setBrush(QColor(0, 0, 0, 100));
            painter.drawRect(0, 0, this->width(), this->height());
        }

        painter.setPen(QColor(255, 255, 255, 200));
        painter.drawLine(0, cursorY, this->width(), cursorY);
        painter.drawLine(cursorX, 0, cursorX, this->height());

        drawMagnifier(&painter);
    }
    void keyPressEvent(QKeyEvent* event) override {
        if (event->key() == Qt::Key_Escape) {
            puts("closing");
            close();
        }
        if (event->key() == Qt::Key_Space) {
            printf("0,0,%i,%i", width(), height());
            close();
        }
    }
    void mouseMoveEvent(QMouseEvent* mouseEvent) override {
        /**
         * On Mouse Moved Event
         *
         * When the cursor is moved by the user, Overlay fields cursorX and cursorY are updated with the
         * mouses current x and y positions. The painter is then updated, redrawing the entire overlay
         * only when the user moves the mouse.
         *
         * @param mouseEvent - The QMouseEvent for the given mouse moved event
         * @return void
         */
        cursorX = mouseEvent->pos().x();
        cursorY = mouseEvent->pos().y();
        update();
    }
    void mousePressEvent(QMouseEvent* mouseEvent) override {
        /**
         * On Mouse Pressed Event
         *
         * When the cursor is pressed by the user, Overlay fields startX and startY are updated with the
         * mouses current x and y positions. startX and Y signify the starting x and y coordinates of the region
         * selected by the user. These values are used to calculate the selection region { x,y,w,h }.
         *
         * The pressed boolean is then set to true to notify the paint event of the mouse being pressed.
         *
         * @param mouseEvent - The QMouseEvent for the given mouse moved event
         * @return void
         */
        pressed = true;
        startX = mouseEvent->pos().x();
        startY = mouseEvent->pos().y();
    }
    void mouseReleaseEvent(QMouseEvent* mouseEvent) override {
        /**
         * On Mouse Release Event
         *
         * When the cursor is released by the user, Overlay fields startX and startY are reset to zero
         * and the pressed boolean is reset to false and the overlay is then closed.
         *
         * The calculated region information is then printed to the console in the format x,y,width,height
         *
         * @param mouseEvent - The QMouseEvent for the given mouse moved event. (un-used)
         * @return void
         */
        pressed = false;
        startX = startY = 0;

        if (region.width() > 0 && region.height() > 0) {
            printf("%i,%i,%i,%i", region.x(), region.y(), region.width(), region.height());
        } else {
            printf("%i,%i,0,0", cursorX, cursorY);
        }

        close();
    }
    void wheelEvent(QWheelEvent *event) override {
        if (event->modifiers() & Qt::ShiftModifier) {
            if (event->delta() > 0) {
                if (mag.wh < 300) mag.wh+=2;
            } else {
                if (mag.wh > 2) mag.wh-=2;
            }
        } else {
            if (event->delta() > 0) {
                if (mag.zoom > 8) mag.zoom-=2;
            } else {
                if (mag.zoom < 80) mag.zoom+=2;
            }
        }
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    Overlay widget;
    widget.start();

    app.exec();
    return 0;
}
