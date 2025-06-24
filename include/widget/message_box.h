#ifndef MESSAGE_BOX
#define MESSAGE_BOX

#include "widget_pch.h"
#include <qanimationgroup.h>
#include <qobject.h>
#include <qparallelanimationgroup.h>

class MessageBox: public QWidget, private NonCopyable {
    Q_OBJECT
    Q_PROPERTY(int slidePosition READ slidePosition WRITE setSlidePosition)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
public:
    enum class MessageType: std::int8_t {
        Info,
        Success,
        Warning,
        Error
    };

    explicit MessageBox( QString text
                       , MessageType type = MessageType::Info
                       , int duration = 3000
                       , QWidget* parent = nullptr);
    ~MessageBox();

    void show_message();
    
    void close();

    [[nodiscard]] auto 
    message_type() const -> MessageType { return m_type; }

    static void show_info(QString,    int duration = 3000);
    static void show_success(QString, int duration = 3000);
    static void show_warning(QString, int duration = 4000);
    static void show_error(QString,   int duration = 5000);

protected:
    void paintEvent(QPaintEvent*) override;
    void enterEvent(QEnterEvent*) override;
    void leaveEvent(QEvent*) override;
    void mousePressEvent(QMouseEvent*) override;

private:
    void setup_UI();
    void start_show_animation();
    void start_hide_animation();
    void update_position();

    [[nodiscard]] auto 
    slidePosition() const -> int { return m_slide_position; }

    void setSlidePosition(int pos);

    [[nodiscard]] auto 
    opacity() const -> qreal;

    void setOpacity(qreal);

    static void add_active_box(MessageBox*);
    static void remove_active_box(MessageBox*);
    static void update_active_box_positions();
private:
    QString m_text;
    MessageType m_type;
    int m_duration;
    int m_slide_position;

    QTimer* m_auto_close_timer;
    QPropertyAnimation* m_slide_animation;
    QPropertyAnimation* m_fade_animation;
    QGraphicsOpacityEffect* m_opacity_effect;

    bool m_is_hovered;

    static QList<MessageBox*> s_active_boxes;
    static constexpr int MARGIN = 20;
    static constexpr int SPACING = 10;
    static constexpr int BOX_HEIGHT = 60;
    static constexpr int BOX_WIDTH = 300;
};


#endif // MESSAGE_BOX