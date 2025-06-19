#ifndef REGION_CROPPER_H
#define REGION_CROPPER_H

#include <QWidget>
#include <QRect>
#include <QPoint>
#include <qpaintdevice.h>
#include <qpoint.h>
#include <qwidget.h>

class RegionCropper: public QWidget {
    Q_OBJECT
public:
    explicit RegionCropper(QWidget* parent = nullptr);
    ~RegionCropper() override;

    const QRect& get_selection_rect() const;

protected:
    void mousePressEvent(QMouseEvent*)   override;
    void mouseMoveEvent(QMouseEvent*)    override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void paintEvent(QPaintEvent*)        override;
private:
    bool m_is_selecting;
    QPoint m_start_point;
    QRect m_selection_rect;
};

#endif // REGION_CROPPER_H