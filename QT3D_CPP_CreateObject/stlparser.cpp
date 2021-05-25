#include "stlparser.h"
#include <QFile>
#include <QDebug>
#include <QChar>
#include <QDataStream>
#include <iostream>
#include <fstream>
#include <QtMath>


stlParser::stlParser()
{

}


void stlParser::uploadFile()
{
    std::fstream input("C:\\Users\\akuluiev\\Documents\\Qt3D_CPP\\QT3D_CPP_CreateObject\\xyzCalibration_cube.stl", std::ios_base::in | std::ios_base::binary);

    input.read((char *)&_header, sizeof(_header));
    std::cout << "_header: " << _header.Header << std::endl;
    std::cout << "Number_of_triangles: " << _header.Number_of_triangles << std::endl;

    _triangles.reserve(_header.Number_of_triangles+1);
    _triangles.resize(_header.Number_of_triangles+1);

    _triangles2.reserve(_header.Number_of_triangles+1);
    _triangles2.resize(_header.Number_of_triangles+1);

    int i=1;
    while (input.read(reinterpret_cast<char *>(&_triangles[i-1]), 50))
    {
        _triangles2[i-1].Vertex_1 = {_triangles.at(i-1).Vertex_1[0], _triangles.at(i-1).Vertex_1[1], _triangles.at(i-1).Vertex_1[2]};
        _triangles2[i-1].Vertex_2 = {_triangles.at(i-1).Vertex_2[0], _triangles.at(i-1).Vertex_2[1], _triangles.at(i-1).Vertex_2[2]};
        _triangles2[i-1].Vertex_3 = {_triangles.at(i-1).Vertex_3[0], _triangles.at(i-1).Vertex_3[1], _triangles.at(i-1).Vertex_3[2]};
        _triangles2[i-1].Normal_vector = {_triangles.at(i-1).Normal_vector[0], _triangles.at(i-1).Normal_vector[1], _triangles.at(i-1).Normal_vector[2]};

        std::cout << i << ") Normal : (" << _triangles.at(i-1).Normal_vector[0] << ", " << _triangles.at(i-1).Normal_vector[1] << ", " << _triangles.at(i-1).Normal_vector[2] << "), ";
        std::cout << "Vertex_1 : (" << _triangles.at(i-1).Vertex_1[0] << ", " << _triangles.at(i-1).Vertex_1[1] << ", " << _triangles.at(i-1).Vertex_1[2] << "), ";
        std::cout << "Vertex_2 : (" << _triangles.at(i-1).Vertex_2[0] << ", " << _triangles.at(i-1).Vertex_2[1] << ", " << _triangles.at(i-1).Vertex_2[2] << "), ";
        std::cout << "Vertex_3 : (" << _triangles.at(i-1).Vertex_3[0] << ", " << _triangles.at(i-1).Vertex_3[1] << ", " << _triangles.at(i-1).Vertex_3[2] << "), ";
        std::cout << "Attribute_byte_count : " << _triangles.at(i-1).Attribute_byte_count << std::endl;
        i++;
    }

    std::cout << std::endl << std::endl;
}


void stlParser::overhangsThatShouldHaveSupports()
{
    std::cout << "Overhangs that should have supports:" << std::endl << std::endl;

    int i = 1;
    for(triangle &obj : _triangles)
    {
        float angle = qRadiansToDegrees(qAcos(obj.Normal_vector[2] /
                qSqrt(  qPow(obj.Normal_vector[0], 2) +
                qPow(obj.Normal_vector[1], 2) +
                qPow(obj.Normal_vector[2], 2))));

        if(angle >= 100) // 135
        {
            _overhangsTriangles.append({(i-1), angle});

            _overhangsTriangles2.append({
                                            {_triangles.at(i-1).Vertex_1[0], _triangles.at(i-1).Vertex_1[1], _triangles.at(i-1).Vertex_1[2]}, // vertex1
                                            {_triangles.at(i-1).Vertex_2[0], _triangles.at(i-1).Vertex_2[1], _triangles.at(i-1).Vertex_2[2]}, // vertex2
                                            {_triangles.at(i-1).Vertex_3[0], _triangles.at(i-1).Vertex_3[1], _triangles.at(i-1).Vertex_3[2]}, // vertex3
                                            {_triangles.at(i-1).Normal_vector[0], _triangles.at(i-1).Normal_vector[1], _triangles.at(i-1).Normal_vector[2]}  // normal vector
                                        });

            std::cout   << i << ") " << obj.Normal_vector[0] << ", "
                    << obj.Normal_vector[1] << ", "
                    << obj.Normal_vector[2] << " - "
                    << "angle: " << angle << std::endl;
        }
        i++;
    }
}

bool stlParser::isOverhangsTriangle(int index)
{
    for(auto &v : _overhangsTriangles)
    {
        if(index == v._positionInTrinalesArray)
        {
            return true;
        }
    }

    return false;
}
