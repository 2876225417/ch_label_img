#ifndef IMAGE_FILE_ITEM_H
#define IMAGE_FILE_ITEM_H

#include "utils/non-copyable.h"
#include "widget_pch.h"
#include <qboxlayout.h>
#include <qcoreevent.h>
#include <qevent.h>
#include <qfileinfo.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qsize.h>
#include <qtmetamacros.h>
#include <qwidget.h>


class ImageFileItem: public QWidget, private NonCopyable {
    Q_OBJECT
public:
    explicit ImageFileItem(QString file_path, QWidget* parent = nullptr);
    ~ImageFileItem();

    enum class AnnotationStatus: std::int8_t {
        NotAnnotated,
        InProgress,
        Completed,
        Reviewed
    };

    [[nodiscard]] auto file_path() const -> const QString& { return std::move(m_file_path); }
    [[nodiscard]] auto file_name() const -> const QString& { return std::move(m_file_name); }
    [[nodiscard]] auto annotation_status() const -> AnnotationStatus { return m_status; }
    [[nodiscard]] auto is_selected() const -> bool { return m_selected; }
    
    void set_annotation_status(AnnotationStatus);
    void set_selected(bool);
    void set_thumbnail_size(const QSize&);

    void refresh_thumbnail();
signals:
    void clicked(ImageFileItem*);
    void double_clicked(ImageFileItem*);
    void status_changed(ImageFileItem*, AnnotationStatus);

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseDoubleClickEvent(QMouseEvent*) override;
    void enterEvent(QEnterEvent*) override;
    void leaveEvent(QEvent*) override;

private:
    void setup_ui();
    void load_thumbnail();
    void update_status_display();

    [[nodiscard]] auto get_status_color() const -> QColor;
    [[nodiscard]] auto get_status_text()  const -> QString;
private:
    QString m_file_path;
    QString m_file_name;
    QFileInfo m_file_info;

    AnnotationStatus m_status;
    bool m_selected;
    bool m_hovered;

    QHBoxLayout* m_main_layout;
    QLabel* m_thumbnail_label;
    QLabel* m_status_label;
    QLabel* m_filename_label;

    QSize m_thumbnail_size;
    QPixmap m_thumbnail;

    static constexpr int DEFAULT_THUMBNAIL_SIZE = 64;
    static constexpr int ITEM_HEIGHT = 80;
    static constexpr int MARGIN = 8;
    static constexpr int SPACING = 12;
    static constexpr int STATUS_INDICATOR_SIZE = 12;
};
#endif // IMAGE_FILE_ITEM_H