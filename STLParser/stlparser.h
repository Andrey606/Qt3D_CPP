#ifndef STLPARSER_H
#define STLPARSER_H

#include <QObject>
#include <QVector>

struct stl {
    char Header[80];
    uint32_t Number_of_triangles;
};

struct triangle {
    float Normal_vector[3]; // 12
    float Vertex_1[3];
    float Vertex_2[3];
    float Vertex_3[3];
    uint16_t Attribute_byte_count;
};

struct overhangsTriangle
{
    int _positionInTrinalesArray;
    float _angle;
};

class stlParser
{
public:
    stlParser();
    virtual ~stlParser() = default;

   void uploadFile();
   void overhangsThatShouldHaveSupports();

public:
    stl _header;
    QVector<triangle> _triangles;
    QVector<overhangsTriangle> _overhangsTriangles;
};

#endif // STLPARSER_H
