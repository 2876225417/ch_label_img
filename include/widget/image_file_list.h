#ifndef IMAGE_FILE_LIST_H
#define IMAGE_FILE_LIST_H

#include "utils/non-copyable.h"
#include "widget/image_file_item.h"
#include "widget_pch.h"
#include <qboxlayout.h>
#include <qscrollarea.h>
#include <qsize.h>
#include <qtmetamacros.h>
#include <qwidget.h>
#include <qwindowdefs.h>

class ImageFileList: public QWidget, private NonCopyable {
    Q_OBJECT
public:
    explicit ImageFileList(QWidget* parent = nullptr);
    ~ImageFileList();

    void load_directory(const QString&);
    void add_file(const QString&);

    void clear();

    [[nodiscard]] auto selected_items() const -> QList<ImageFileItem*>;

    void set_thumbnail_size(const QSize&);

signals:
    void item_clicked(ImageFileItem*);
    void item_double_clicked(ImageFileItem*);
    void selection_changed();

private slots:
    void on_item_clicked(ImageFileItem*);
    void on_item_double_clicked(ImageFileItem*);
private:
    void setup_ui();
    void clear_selection();
    [[nodiscard]] auto is_image_file(const QString&) const -> bool;

private:
    QScrollArea* m_scroll_area;
    QWidget* m_content_widget;
    QVBoxLayout* m_content_layout;

    QList<ImageFileItem*> m_items;
    QSize m_thumbnail_size;

    static const QStringList IMAGE_EXTENSIONS;
};

#endif // IMAGE_FILE_LIST_H