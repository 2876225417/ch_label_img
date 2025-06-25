#include "utils/action_types.h"
#include <qcolor.h>
#include <qcoreevent.h>
#include <qevent.h>
#include <qlabel.h>
#include <qnamespace.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpixmapcache.h>
#include <qsize.h>
#include <qwidget.h>
#include <widget/image_file_item.h>

void ImageFileItem::setup_ui() {
    setFixedHeight(ITEM_HEIGHT);
    setMinimumWidth(300);
    setCursor(Qt::PointingHandCursor);

    m_main_layout = new QHBoxLayout{this};
    m_main_layout->setContentsMargins(MARGIN, MARGIN, MARGIN, MARGIN);
    m_main_layout->setSpacing(SPACING);

    // 缩略图
    m_thumbnail_label = new QLabel{this};
    m_thumbnail_label->setFixedSize(m_thumbnail_size);
    m_thumbnail_label->setAlignment(Qt::AlignCenter);
    m_thumbnail_label->setStyleSheet(
        "QLabel {"
        "   border: 1px solid #ddd;"
        "   border-radius: 4px;"
        "   background-color: #f5f5f5"
        "}"
    );
    m_main_layout->addWidget(m_thumbnail_label);

    // 标注状态
    m_status_label = new QLabel{this};
    m_status_label->setFixedHeight(80);
    m_status_label->setAlignment(Qt::AlignCenter);
    m_status_label->setStyleSheet(
        "QLabel {"
        "   border-radius: 4px;"
        "   padding: 4px 8px;"
        "   font-weight: bold;"
        "   color: white;"
        "}"
    );
    update_status_display();
    m_main_layout->addWidget(m_status_label);

    // 文件信息
    m_filename_label = new QLabel{m_file_name, this};
    m_filename_label->setStyleSheet(
        "QLabel {"
        "   color: #333;"
        "   font-size: 12px;"
        "}"
    );
    m_filename_label->setWordWrap(true);
    m_main_layout->addWidget(m_filename_label, 1);

    m_main_layout->addStretch();
}

void ImageFileItem::load_thumbnail() {
    QString cache_key = QString("thumb_%1_%2x%3")
                        .arg(m_file_path)
                        .arg(m_thumbnail_size.width())
                        .arg(m_thumbnail_size.height());

    if (QPixmapCache::find(cache_key, &m_thumbnail)) {
        m_thumbnail_label->setPixmap(m_thumbnail);
        return;
    }

    QImageReader reader{m_file_path};
    if (!reader.canRead()) {
        m_thumbnail_label->setText("No\nPreview");
        m_thumbnail_label->setStyleSheet(
            m_thumbnail_label->styleSheet() +
            "QLabel { color: #999; font-size: 10px; }"
        );
        return;
    }
    
    QSize image_size = reader.size();
    if (image_size.width() > m_thumbnail_size.width() * 2) {
        image_size.scale(m_thumbnail_size * 2, Qt::KeepAspectRatio);
        reader.setScaledSize(image_size);
    }

    QImage image = reader.read();
    if (image.isNull()) {
        m_thumbnail_label->setText("Load\nError");
        return;
    }

    m_thumbnail = QPixmap::fromImage(
        image.scaled(m_thumbnail_size, Qt::KeepAspectRatio, Qt::SmoothTransformation)
    );

    QPixmapCache::insert(cache_key, m_thumbnail);

    m_thumbnail_label->setPixmap(m_thumbnail);
}

void ImageFileItem::set_annotation_status(AnnotationStatus status) {
    if (m_status != status) {
        m_status = status;
        update_status_display();
        emit status_changed(this, status);
        update();
    }
}

void ImageFileItem::set_selected(bool selected) {
    if (m_selected != selected) {
        m_selected = selected;
        update();
    }
}

void ImageFileItem::set_thumbnail_size(const QSize& size) {
    m_thumbnail_size = size;
    m_thumbnail_label->setFixedSize(size);
    refresh_thumbnail();
}

void ImageFileItem::refresh_thumbnail() {
    QString cache_key = QString("thumb_%1_%2x%3")
                        .arg(m_file_path)
                        .arg(m_thumbnail_size.width())
                        .arg(m_thumbnail_size.height());

    QPixmapCache::remove(cache_key);
    load_thumbnail();
}

void ImageFileItem::update_status_display() {
    QColor color = get_status_color();
    QString text = get_status_text();

    m_status_label->setText(text);
    m_status_label->setStyleSheet(
        QString("QLabel {"
                "   background-color: %1;"
                "   border-radius: 4px;"
                "   padding: 4px 8px;"
                "   font-weight: bold;"
                "   color: white;"
                "   font-size: 10px;"
                "}").arg(color.name())
    );
}

auto ImageFileItem::get_status_color() const -> QColor {
    switch (m_status) {
        case AnnotationStatus::NotAnnotated: return QColor("#e74c3c");
        case AnnotationStatus::InProgress:   return QColor("#f39c12");
        case AnnotationStatus::Completed:    return QColor("#27ae60");
        case AnnotationStatus::Reviewed:     return QColor("#3498db");
    }
    return QColor("#95a5a6");
}

auto ImageFileItem::get_status_text() const -> QString {
switch (m_status) {
        case AnnotationStatus::NotAnnotated: return "未标注";
        case AnnotationStatus::InProgress:   return "标注中";
        case AnnotationStatus::Completed:    return "已完成";
        case AnnotationStatus::Reviewed:     return "已审核";
    }
    return "未知";
}

void ImageFileItem::paintEvent(QPaintEvent* event) {
    QPainter painter{this};
    painter.setRenderHint(QPainter::Antialiasing);

    QColor bg_color;
    if (m_selected)     bg_color = QColor("#e3f2fd");
    else if (m_hovered) bg_color = QColor("#f5f5f5");
    else                bg_color = QColor("#ffffff");

    painter.fillRect(rect(), bg_color);

    if (m_selected) {
        QPen pen(QColor("#2196f3"), 2);
        painter.setPen(pen);
        painter.drawRect(rect().adjusted(1, 1, -1, -1));
    }

    QWidget::paintEvent(event);
}

void ImageFileItem::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) emit clicked(this);
    QWidget::mousePressEvent(event);
}

void ImageFileItem::mouseDoubleClickEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) emit double_clicked(this);
    QWidget::mouseDoubleClickEvent(event);
}

void ImageFileItem::enterEvent(QEnterEvent* event) {
    m_hovered = true;
    update();
    QWidget::enterEvent(event);
}

void ImageFileItem::leaveEvent(QEvent* event) {
    m_hovered = false;
    update();
    QWidget::leaveEvent(event);
}



















ImageFileItem::ImageFileItem(QString file_path, QWidget* parent)
    : QWidget{parent}
    , m_file_path{std::move(file_path)}
    , m_file_info{file_path}
    , m_status{AnnotationStatus::NotAnnotated}
    , m_selected{false}
    , m_hovered{false}
    , m_thumbnail_size{DEFAULT_THUMBNAIL_SIZE, DEFAULT_THUMBNAIL_SIZE}
    {
    m_file_name = m_file_info.baseName();
    setup_ui();
    load_thumbnail();
}

ImageFileItem::~ImageFileItem() = default;