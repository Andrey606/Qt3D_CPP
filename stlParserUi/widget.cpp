#include "widget.h"
#include <QDebug>

void Widget::paintEvent(QPaintEvent *event)
{
    DrawWireframeTriangle();
}

void Widget::DrawLine(QPoint v1, QPoint v2)
{
    QPainter painter(this);
    painter.drawLine(v1.x(), v1.y(), v2.x(), v2.y());
}

void Widget::DrawLine(QPointF v1, QPointF v2)
{
    QPainter painter(this);

    qDebug() << v1.x() << ", " << v1.y();
    qDebug() << v2.x() << ", " << v2.y();

    painter.drawLine(v1.x(), v1.y(), v2.x(), v2.y());
}

QPointF Widget::ViewportToCanvas(float x, float y)
{
    return {((x*Cw/Vw)+px), ((y*Ch/Vh)+py)};
}

QPointF Widget::ProjectVertex(QVector3D v)
{
      return ViewportToCanvas(v.x() * d / (v.z()+1), v.y() * d / (v.z()+1));
}

void Widget::Draw3DLine(QVector3D v1, QVector3D v2)
{
    QPointF P1 = {((v1.x()*d)/v1.z()), ((v1.y()*d)/v1.z())};
    QPointF P2 = {((v2.x()*d)/v2.z()), ((v1.y()*d)/v2.z())};

    qDebug() << P1;
    qDebug() << P2;

    DrawLine(P1, P2);
}

void Widget::DrawWireframeTriangle()
{
    triangle triangle;
    triangle.Vertex_1[0] = -1;
    triangle.Vertex_1[1] = 1;
    triangle.Vertex_1[2] = 1;

    triangle.Vertex_2[0] = 1;
    triangle.Vertex_2[1] = 1;
    triangle.Vertex_2[2] = 1;

    triangle.Vertex_3[0] = 1;
    triangle.Vertex_3[1] = -1;
    triangle.Vertex_3[2] = 1;

    QVector3D vAf = {-1, 1, 1};
    QVector3D vBf = {1, 1, 1};
    QVector3D vCf = {1, -1, 1};
    QVector3D vDf = {-1, -1, 1};

      QVector3D vAb = {-1, 1, 2};
      QVector3D vBb = {1, 1, 2};
      QVector3D vCb = {1, -1, 2};
      QVector3D vDb = {-1, -1, 2};



    //for(auto &triangle :_triangles)
    {
        DrawLine(ProjectVertex(vAf), ProjectVertex(vBf));
        DrawLine(ProjectVertex(vBf), ProjectVertex(vCf));
        DrawLine(ProjectVertex(vCf), ProjectVertex(vDf));
        DrawLine(ProjectVertex(vDf), ProjectVertex(vAf));

      DrawLine(ProjectVertex(vAb), ProjectVertex(vBb));
      DrawLine(ProjectVertex(vBb), ProjectVertex(vCb));
      DrawLine(ProjectVertex(vCb), ProjectVertex(vDb));
      DrawLine(ProjectVertex(vDb), ProjectVertex(vAb));

      DrawLine(ProjectVertex(vAf), ProjectVertex(vAb));
      DrawLine(ProjectVertex(vBf), ProjectVertex(vBb));
      DrawLine(ProjectVertex(vCf), ProjectVertex(vCb));
      DrawLine(ProjectVertex(vDf), ProjectVertex(vDb));

      qDebug() << "Triangles: ";

      for(auto &triangle :_triangles)
      {
          QVector3D v1 = {triangle.Vertex_1[0], triangle.Vertex_1[1], triangle.Vertex_1[2]};
          QVector3D v2 = {triangle.Vertex_2[0], triangle.Vertex_2[1], triangle.Vertex_2[2]};
          QVector3D v3 = {triangle.Vertex_3[0], triangle.Vertex_3[1], triangle.Vertex_3[2]};

          DrawLine(ProjectVertex(v1), ProjectVertex(v2));
          DrawLine(ProjectVertex(v2), ProjectVertex(v3));
          DrawLine(ProjectVertex(v3), ProjectVertex(v1));

          //break;
      }



        //break;
    }
}
