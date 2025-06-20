#ifndef IMAGE_VIEWER_H
#define IMAGE_VIEWER_H

#include "utils/non-copyable.h"
#include <qevent.h>
#include <qpixmap.h>
#include <qtmetamacros.h>
#include <qwidget.h>
class ImageViewer: public QWidget, private NonCopyable {
    Q_OBJECT
public:
    explicit ImageViewer(QWidget* parent=  nullptr);
    ~ImageViewer() override;

public slots:
    void set_pixmap(const QPixmap& pixmap);

protected:
    void paintEvent(QPaintEvent* event) override;
private:
    QPixmap m_pixmap;
};
#endif // IMAGE_VIEWER_H