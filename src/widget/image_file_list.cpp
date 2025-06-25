#include "widget/image_file_item.h"
#include <cstdio>
#include <qapplication.h>
#include <qboxlayout.h>
#include <qcontainerfwd.h>
#include <qfileinfo.h>
#include <qlist.h>
#include <qnamespace.h>
#include <qscrollarea.h>
#include <qsize.h>
#include <qwidget.h>
#include <widget/image_file_list.h>

#include <qtils/logger.hpp>

const QStringList ImageFileList::IMAGE_EXTENSIONS = {
    "jpg", "jpeg", "png", "bmp", "gif", "webp", "tiff", "tif"
};

void ImageFileList::setup_ui() {
    auto* main_layout = new QVBoxLayout{this};
    main_layout->setContentsMargins(0, 0, 0, 0);

    m_scroll_area = new QScrollArea{this};
    m_scroll_area->setWidgetResizable(true);
    m_scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_content_widget = new QWidget;
    m_content_layout = new QVBoxLayout{m_content_widget};
    m_content_layout->setContentsMargins(0, 0, 0, 0);
    m_content_layout->setSpacing(1);
    m_content_layout->addStretch();
    
    m_scroll_area->setWidget(m_content_widget);
    main_layout->addWidget(m_scroll_area);
}

void ImageFileList::load_directory(const QString& dir_path) {
    clear();
 
    const std::string path = dir_path.toStdString().c_str();

    using namespace labelimg::qtils::logger;
    logg<LogLevel::INFO>("Dir path: {}", path);

    QDir directory{dir_path};
    if (!directory.exists()) return;

    QStringList filters;
    for (const QString& ext: IMAGE_EXTENSIONS) filters << QString("*.%1").arg(ext);

    QStringList files = directory.entryList(filters, QDir::Files, QDir::Name);

    if (files.size() == 0) 
        logg<LogLevel::ERROR>("No files in this dir");

    for (const QString& file: files) {
        const std::string file_name = file.toStdString();
        QString file_path = directory.filePath(file);
        logg<LogLevel::INFO>("file name: {}", file_name);
        add_file(file_path);
    }

}

void ImageFileList::add_file(const QString& file_path) {
    if (!is_image_file(file_path)) return;
    
    auto* item = new ImageFileItem(file_path, this);
    item->set_thumbnail_size(m_thumbnail_size);

    connect(item, &ImageFileItem::clicked, this, &ImageFileList::on_item_clicked);
    connect(item, &ImageFileItem::double_clicked, this, &ImageFileList::on_item_double_clicked);

    m_content_layout->insertWidget(m_content_layout->count() - 1, item);
    m_items.append(item);
}

void ImageFileList::clear() {
    for (ImageFileItem* item: m_items) {
        m_content_layout->removeWidget(item);
        item->deleteLater();
    }
    m_items.clear();
}

auto ImageFileList::selected_items() const -> QList<ImageFileItem*> { 
    QList<ImageFileItem*> selected;
    for (ImageFileItem* item: m_items) {
        if (item->is_selected()) selected.append(item);
    }
    return selected;
}

void ImageFileList::set_thumbnail_size(const QSize& size) {
    m_thumbnail_size = size;
    for (ImageFileItem* item: m_items) 
        item->set_thumbnail_size(size);
}

void ImageFileList::on_item_clicked(ImageFileItem* item) {
    if (!QApplication::keyboardModifiers().testFlag(Qt::ControlModifier))
        clear_selection();

    item->set_selected(!item->is_selected());
    emit item_clicked(item);
    emit selection_changed();
}

void ImageFileList::on_item_double_clicked(ImageFileItem* item) {
    emit item_double_clicked(item);
}

void ImageFileList::clear_selection() {
    for (ImageFileItem* item: m_items)
        item->set_selected(false);
}

auto ImageFileList::is_image_file(const QString& file_path) const -> bool {
    QFileInfo info{file_path};
    QString suffix = info.suffix().toLower();
    return IMAGE_EXTENSIONS.contains(suffix);
}






ImageFileList::ImageFileList(QWidget* parent)
    : QWidget{parent}
    , m_thumbnail_size{64, 64}
    {
    setup_ui();
}

ImageFileList::~ImageFileList() = default;