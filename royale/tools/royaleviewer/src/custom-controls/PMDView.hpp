#ifndef BASETOOL_H
#define BASETOOL_H

#include <QWidget>

class PMDView : public QWidget
{
    Q_OBJECT

public:
    explicit PMDView (QWidget *parent = 0);
    explicit PMDView (QWidget *parent, int rows, int columns);

protected:
    void paintEvent (QPaintEvent *) Q_DECL_OVERRIDE;

public slots:
    void toggle();

signals:
    void hidden();

private:
    bool m_drawFrame;
};

#endif // BASETOOL_H
