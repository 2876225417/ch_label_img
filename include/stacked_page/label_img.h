#ifndef LABEL_IMG_PAGE
#define LABEL_IMG_PAGE

#include "stacked_page/base_page.h"
#include "widget/labeling_canvas.h"


class LabelImagePage: public BasePage {
    Q_OBJECT
public:
    explicit LabelImagePage(QWidget* parent = nullptr);
    ~LabelImagePage() override;
signals:

private:
    void set_layout() override;
    void set_connections() override;


    LabelingCanvas* m_canvas;
};

#endif // LABEL_IMG_PAGE