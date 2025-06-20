#ifndef LABELING_CANVAS_H
#define LABELING_CANVAS_H

#include <qevent.h>
#include <qpixmap.h>
#include <qtmetamacros.h>
#include <qwidget.h>
#include <utils/non-copyable.h>
#include <widget/image_viewer.h>
#include <widget/region_cropper.h>

class LabelingCanvas: public QWidget, private NonCopyable {
    Q_OBJECT
public:
    explicit LabelingCanvas(QWidget* parent = nullptr);
    ~LabelingCanvas() override;

public slots:
    void set_pixmap(const QPixmap& pixmap);

signals:

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    ImageViewer* m_image_viewer;
    RegionCropper* m_region_cropper;
};
#endif // LABELING_CANVAS_H