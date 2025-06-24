#include <qabstractanimation.h>
#include <qapplication.h>
#include <qcolor.h>
#include <qcoreevent.h>
#include <qevent.h>
#include <qguiapplication.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qpainter.h>
#include <qparallelanimationgroup.h>
#include <qscreen.h>
#include <qtpreprocessorsupport.h>
#include <qtypes.h>
#include <qvariant.h>
#include <qwidget.h>
#include <qwindowdefs.h>
#include <widget/message_box.h>

QList<MessageBox*> MessageBox::s_active_boxes;

void MessageBox::setup_UI() {
    setFixedSize(BOX_WIDTH, BOX_HEIGHT);

    auto* shadow = new QGraphicsDropShadowEffect{this};
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 80));
    shadow->setOffset(0, 2);
    setGraphicsEffect(shadow);

    m_opacity_effect = new QGraphicsOpacityEffect{this};
    m_opacity_effect->setOpacity(0.f);

    m_auto_close_timer = new QTimer{this};
    m_auto_close_timer->setSingleShot(true);
    connect(m_auto_close_timer, &QTimer::timeout, this, &MessageBox::close);
    
    m_slide_animation = new QPropertyAnimation{this, "slidePosition"};
    m_slide_animation->setDuration(300);
    m_slide_animation->setEasingCurve(QEasingCurve::OutCubic);

    m_fade_animation = new QPropertyAnimation(m_opacity_effect, "opacity");
    m_fade_animation->setDuration(300);
}

void MessageBox::show_message() {
    add_active_box(this);
    
    update_position();

    show();
    raise();

    start_show_animation();
    if (m_duration > 0) m_auto_close_timer->start(m_duration);
}

void MessageBox::close() {
    if (m_slide_animation->state() == QAbstractAnimation::Running) return;

    if (m_auto_close_timer) m_auto_close_timer->stop();
    start_hide_animation();
}

void MessageBox::start_show_animation() {
    m_slide_position = width();

    m_slide_animation->setStartValue(width());
    m_slide_animation->setEndValue(0);

    m_fade_animation->setStartValue(0.f);
    m_fade_animation->setEndValue(1.f);

    auto* group = new QParallelAnimationGroup{this};
    group->addAnimation(m_slide_animation);
    group->addAnimation(m_fade_animation);

    setGraphicsEffect(m_opacity_effect);
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void MessageBox::start_hide_animation() {
    m_slide_animation->setStartValue(0);
    m_slide_animation->setEndValue(width());

    m_fade_animation->setStartValue(1.f);
    m_fade_animation->setEndValue(0.f);

    auto* group = new QParallelAnimationGroup{this};
    group->addAnimation(m_slide_animation);
    group->addAnimation(m_fade_animation);

    connect(group, &QParallelAnimationGroup::finished, this, &QWidget::close);
    
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void MessageBox::update_position() {
    if (QApplication::instance()) {
        QScreen* screen = QApplication::primaryScreen();
        QRect screen_geometry = screen->availableGeometry();

        int index = s_active_boxes.indexOf(this);
        int x = screen_geometry.right() - width() - MARGIN + m_slide_position;
        int y = screen_geometry.bottom() - MARGIN - (index + 1) * (BOX_HEIGHT + SPACING);

        move(x, y);
    } else if (parent()) {
        QWidget* p = qobject_cast<QWidget*>(parent());
        int index = s_active_boxes.indexOf(this);
        int x = p->width() - width() - MARGIN + m_slide_position;
        int y = p->height() - MARGIN - (index + 1) * (BOX_HEIGHT + SPACING);

        move(x, y);
    }
}

void MessageBox::setSlidePosition(int pos) {
    m_slide_position = pos;
    update_position();
}

auto MessageBox::opacity() const -> qreal {
    return m_opacity_effect ? m_opacity_effect->opacity() : 1.f;
}

void MessageBox::setOpacity(qreal opacity) {
    if (m_opacity_effect) m_opacity_effect->setOpacity(opacity);
}

void MessageBox::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter{this};
    painter.setRenderHint(QPainter::Antialiasing);

    QColor bg_color;
    QColor border_color;
    QColor text_color = Qt::white;

    switch (m_type) {
        case MessageType::Info:
            bg_color = QColor(52, 152, 219);
            border_color = QColor(41, 128, 185);
            break;
        case MessageType::Success:
            bg_color = QColor(46, 204, 113);
            border_color = QColor(39, 174, 96);
            break;
        case MessageType::Warning:
            bg_color = QColor(243, 156, 18);
            border_color = QColor(230, 126, 34);
            break;
        case MessageType::Error:
            bg_color = QColor(231, 76, 60);
            border_color = QColor(192, 57, 43);
            break;
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(bg_color);
    painter.drawRoundedRect(rect(), 8, 8);

    painter.setPen(QPen(border_color, 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(rect().adjusted(0, 0, -1, -1), 8, 8);

    QString icon;
    switch (m_type) {
        case MessageType::Info:    icon = "ℹ"; break;
        case MessageType::Success: icon = "✓"; break;
        case MessageType::Warning: icon = "⚠"; break;
        case MessageType::Error:   icon = "✕"; break;
    }

    painter.setPen(text_color);
    painter.setFont(QFont("Arial", 16, QFont::Bold));
    painter.drawText(QRect(15, 0, 30, height()), Qt::AlignCenter, icon);

    painter.setFont(QFont("Arial", 11));
    painter.drawText(QRect(50, 0, width() - 70, height()), Qt::AlignCenter | Qt::TextWordWrap, m_text);

    if (m_is_hovered) {
        painter.setPen(QPen(text_color, 2));
        painter.drawText(QRect(width() - 30, 0, 20, height()), Qt::AlignHCenter, "×");
    }
}

void MessageBox::enterEvent(QEnterEvent* event) {
    Q_UNUSED(event);
    m_is_hovered = true;
    m_auto_close_timer->stop();
    update();
}

void MessageBox::leaveEvent(QEvent* event) {
    Q_UNUSED(event);
    m_is_hovered = false;
    if (m_duration > 0) m_auto_close_timer->start(1000);
    update();
}

void MessageBox::mousePressEvent(QMouseEvent* event) {
    if (event->x() > width() - 30) close();
}



void MessageBox::add_active_box(MessageBox* box) {
    s_active_boxes.append(box);
    update_active_box_positions();
}

void MessageBox::remove_active_box(MessageBox* box) {
    s_active_boxes.removeAll(box);
    if (qApp) update_active_box_positions();
}

void MessageBox::update_active_box_positions() {
    for (int i = 0; i < s_active_boxes.size(); ++i) 
        s_active_boxes[i]->update_position();
}



void MessageBox::show_info(QString text, int duration) {
    auto* box = new MessageBox(std::move(text), MessageType::Info, duration);
    box->show_message();
}

void MessageBox::show_success(QString text, int duration) {
    auto* box = new MessageBox(std::move(text), MessageType::Success, duration);
    box->show_message();
}

void MessageBox::show_warning(QString text, int duration) {
    auto* box = new MessageBox(std::move(text), MessageType::Warning, duration);
    box->show_message();
}

void MessageBox::show_error(QString text, int duration) {
    auto* box = new MessageBox(std::move(text), MessageType::Error, duration);
    box->show_message();
}

MessageBox::~MessageBox() {
    remove_active_box(this);
}

MessageBox::MessageBox( QString text
                      , MessageType type
                      , int duration
                      , QWidget* parent
    ): QWidget{parent}
     , m_text{std::move(text)}
     , m_type{type}
     , m_duration{duration}
     , m_slide_position{0}
     , m_is_hovered{false}
     {
    setWindowFlags( Qt::FramelessWindowHint
                  | Qt::WindowStaysOnTopHint 
                  | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);

    setup_UI();
}


