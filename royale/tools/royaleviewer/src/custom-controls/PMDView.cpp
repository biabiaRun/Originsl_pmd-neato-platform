#include "PMDView.hpp"
#include <QPainter>
#include <QVBoxLayout>
#include "DisplaySupport.hpp"

PMDView::PMDView (QWidget *parent, int rows, int columns)
    : QWidget (parent)
    , m_drawFrame (true)
{
    QVBoxLayout *l = new QVBoxLayout();
    setLayout (l);
    int s = static_cast<int> (DisplaySupport::sharedInstance()->pointsToPixels (76));
    setMaximumSize (s * columns, s * rows);
    setMinimumSize (s, s);
    setFixedSize (s * columns, s * rows);
    l->setSizeConstraint (QLayout::SetMaximumSize);
    setStyleSheet ("background-color:transparent");
    l->setContentsMargins (10, 10, 10, 10);

    hide();
}

PMDView::PMDView (QWidget *parent)
    : QWidget (parent)
    , m_drawFrame (false)
{
}

void PMDView::toggle()
{
    if (isVisible())
    {
        hide();
    }
    else
    {
        show();
    }
}

void PMDView::paintEvent (QPaintEvent *)
{
    QPainter painter (this);
    painter.fillRect (0, 0, size().width(), size().height(), QBrush (QColor (52, 139, 187)));
    if (m_drawFrame)
    {
        painter.setPen (QColor (255, 255, 255));
        painter.drawRect (0, 0, size().width() - 1, size().height() - 1);
    }
}
