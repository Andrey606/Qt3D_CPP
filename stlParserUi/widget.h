#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QPainter>
#include <stlparser.h>
#include <QVector3D>

class Widget : public QWidget
{
    Q_OBJECT
public:
    explicit Widget(int width, int height, QWidget *parent = nullptr):
        Cw(width), Ch(height), scale(0.2)
    {
        this->setFixedSize(width, height);

        Vw = (width);
        Vh = (height);

        px = Cw/2; // смещение на холсте по x
        py = Ch/2; // смещение на холсте по y
    }

    stl _header;
    QVector<triangle> _triangles;
    QVector<overhangsTriangle> _overhangsTriangles;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void DrawWireframeTriangle();
    void DrawLine(QPoint v1, QPoint v2);
    void DrawLine(QPointF v1, QPointF v2);
    void Draw3DLine(QVector3D v1, QVector3D v2);

    QPointF ViewportToCanvas(float x, float y);
    QPointF ProjectVertex(QVector3D v);



    int Cw=0;
    int Ch=0;
    float Vw=0;
    float Vh=0;
    float scale = 1;

    int d = 100*scale; // растояние от камеры до холста
    int px = Cw/2; // смещение на холсте по x
    int py = Ch/2; // смещение на холсте по y

signals:

};

#endif // WIDGET_H
