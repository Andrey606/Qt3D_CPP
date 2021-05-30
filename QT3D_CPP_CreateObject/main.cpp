#include <QGuiApplication>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DRender/QCamera>
// в QTorusMesh и QTorusGeometry используються разные рендеры
// QTorusGeometry - если хотим самостоятельно задать положение всех вершин обьекта и использовать с ним уже построеный тор
// есть клас Geometry для определение своих обьектов
#include <Qt3DExtras/QTorusMesh>

// тут уже сущесвует набор материалов QPhongMaterial - уже сразу с бликом
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DCore/QTransform>

/*
    тут реализованы много вариантов источников света (точечный, направленный, прожектрный...)
    мы воспользумся точечным
*/
#include <Qt3DRender/QPointLight>
// для управление камерой
#include <Qt3DExtras/QOrbitCameraController>

#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QSceneLoader>
#include <Qt3DExtras/QPerVertexColorMaterial>
#include <QVector2D>
#include <Qt3DExtras/QDiffuseSpecularMaterial>

#include <stlparser.h>

#include <QtMath>
#include <QRandomGenerator>

void createScene(Qt3DCore::QEntity *rootEntity);
void createCustomMeshEntity(Qt3DCore::QEntity *rootEntity);
Qt3DCore::QEntity *createCustomMeshEntity2(Qt3DCore::QEntity *rootEntity, const QVector<triangle2> &triangles, QColor materialColor = QColor(0, 0, 255));
void createLight(Qt3DCore::QEntity *rootEntity, QVector3D translation);

static int randomBetween(int low, int high)
{
    return (qrand() % ((high + 1) - low) + low);
}

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    stlParser parser;
    parser.uploadFile();
    parser.overhangsThatShouldHaveSupports();

    // основное окно
    Qt3DExtras::Qt3DWindow view;

    /*
       в Qt3D сделана сиситема сущностей (Entity), все построенно на них, иерархия,
       корневая сущность может содержать другую сущность:

        - внутри сущности может находиться какойто обьект например (Mesh), материал,
        трансформ (так называемый преобразование для даного набора сущностей)
        каждая сущность может иметь свою компоновку (у каждой сущности есть свой родитель и каждая из них может иметь
        потомков)
    */
    Qt3DCore::QEntity *rootEntity = new Qt3DCore::QEntity; // главная корневая сущность (посути это вся наша сцена, верхний уровень)

    createLight(rootEntity, QVector3D(0.0f, 0.0f, -30.0f));
    createLight(rootEntity, QVector3D(0.0f, 0.0f, 30.0f));

    createLight(rootEntity, QVector3D(0.0f, -30.0f, 0.0f));
    createLight(rootEntity, QVector3D(0.0f, 30.0f, 0.0f));

    createLight(rootEntity, QVector3D(-30.0f, 0.0f, 0.0f));
    createLight(rootEntity, QVector3D(30.0f, 0.0f, 0.0f));

    //createScene(rootEntity); // тор
    //createCustomMeshEntity(rootEntity); // создание обьекта вручную
    //createCustomMeshEntity2(rootEntity);  // загрузка модели с файла
    //createCustomMeshEntity2(rootEntity, parser._triangles2, QColor(0,255,0));
    //createCustomMeshEntity2(rootEntity, parser._overhangsTriangles2);
    Qt3DCore::QEntity *modelEntity = new Qt3DCore::QEntity(rootEntity);
    Qt3DCore::QEntity *customMeshEntity1 = new Qt3DCore::QEntity(modelEntity);
    Qt3DCore::QEntity *customMeshEntity2 = new Qt3DCore::QEntity(modelEntity);

    auto var = [&]() {
        std::size_t const half_size = parser._triangles2.size() / 2;

        QVector<triangle2> split_lo;
        split_lo.resize(1);
        int i = 0;
        for(auto it = parser._triangles2.begin(); it!= parser._triangles2.end(); ++it)
        {
            //split_lo.append(*it);
            split_lo[0] = *it;
            if(parser.isOverhangsTriangle(i)){
                createCustomMeshEntity2(customMeshEntity1, split_lo, QColor(255, 0, 0));
            }
            else{
                createCustomMeshEntity2(customMeshEntity2, split_lo, QColor(0, 0, 255));
            }
            i++;
        }

        QVector<triangle2> split_hi;
        for(auto it = parser._triangles2.begin() + half_size; it!= parser._triangles2.end() + half_size; ++it)
        {
            //split_hi.append(*it);
        }

        //createCustomMeshEntity2(rootEntity, split_lo, QColor(255, 0, 0));
        //createCustomMeshEntity2(rootEntity, split_hi, QColor(0, 0, 255));
    };
    var();

    /*
        кроме самой сущности нужно настроить камеру, она уже сожержиться в Qt3DWindow, поэтому нам нужно росто к ней обратиться.
        сам клас камеры находиться в Qt3DRender::QCamera
    */
    Qt3DRender::QCamera *camera = view.camera();

    /*
        теперь какмеру нужно настроить
        lens - настройка линзы камеры
        setPerspectiveProjection - устанавливаем переспективную проекцию, можно еще ортогональную
        1. - угол наклона
        2. - аспект - ширина делить на высоту окна, чтобы было ближе дальше?
        3. - ближняя плоскость от сечения
        4. - дальняя плоскость от сечения
    */
    camera->lens()->setPerspectiveProjection(60.0f, (float)view.width()/(float)view.height(), 0.1f, 1000.0f);

    /*
        где камера сейчас находиться, по умолчанию в точке (0, 0, 0), я собираюсь в этой точке размещать обьект, поэтому камерц мы отодвинем
        X - направлена на право
        Y - направлена на вверх
        Z - направлена назад наблюдателя

        поэтому мы отодвинем камеру назад по оси Z на 40 юнитов
    */
    camera->setPosition(QVector3D(0.0f, 0.0f, -40.0f));

    // то куда камера смотрит, дальше будем использовать чтобы крутить камеру вокруг обьекта
    camera->setViewCenter(QVector3D(0.0f, 0.0f, 0.0f));

    /*
        теперь создадим контролер камеры чтобы можно было двигать камеру
        их существует два
        1) от первого лица
        2) орбитальный - позволяет крутить камеру вокруг центра ее
    */
    Qt3DExtras::QOrbitCameraController *cameraControler = new Qt3DExtras::QOrbitCameraController(rootEntity);
    cameraControler->setCamera(camera); // задаем контролеру с какой камерй работать
    cameraControler->setLookSpeed(500.0f); // меняем скорост вращение вокруг центра
    cameraControler->setLinearSpeed(200.0f); // скорость перемещение камеры влево вправо вверхвних (не кручение)

    view.setRootEntity(rootEntity); // задаем окну главную сущность
    view.show(); // отображаем содержимое
    return app.exec();
}

void createLight(Qt3DCore::QEntity *rootEntity, QVector3D translation)
{
    // создадим отделную сущность для света
    Qt3DCore::QEntity *lightEntity = new Qt3DCore::QEntity(rootEntity);

    // создадим для этого ентити трансформ чтобы можно было вигать источник света
    Qt3DCore::QTransform *lightTransform = new Qt3DCore::QTransform(lightEntity);
    lightTransform->setTranslation(translation); // передвигаем куда хотим

    // источник света уже сформирован (дефолтный но мы добавим свой точечный)
    Qt3DRender::QPointLight *pointLight = new Qt3DRender::QPointLight(lightEntity);
    //pointLight->setIntensity(0.8f);
    //pointLight->setColor(QColor(0, 0, 0));

    lightEntity->addComponent(lightTransform);
    lightEntity->addComponent(pointLight);
}

void createScene(Qt3DCore::QEntity *resultEntity)
{
    /*
        у всех этих класов в качестве родителей указывается QNode,
        чтобы удаление сущностей происходило правильно нужно указать правильно всем родителей!
        view->rootEntity->...
    */
    Qt3DExtras::QTorusMesh *torusMesh = new Qt3DExtras::QTorusMesh(resultEntity);
    torusMesh->setRadius(20.0f);
    torusMesh->setMinorRadius(3.0f);
    torusMesh->setSlices(26); //это выставляем гладкость, кол-во фрагментов по внутренему радиусу
    torusMesh->setRings(26); //это выставляем гладкость, кол-во фрагментов по внешнему радиусу

    // теперь нужно попределить материал
    Qt3DExtras::QPhongMaterial *torusMaterial = new Qt3DExtras::QPhongMaterial(resultEntity);

    Qt3DCore::QTransform *torusTransform = new Qt3DCore::QTransform(resultEntity);

    // теперь те класы которые компоненты (унаследованы от QComponent) нужно скомпоновать их в resultEntity
    resultEntity->addComponent(torusMesh);
    resultEntity->addComponent(torusMaterial);
    resultEntity->addComponent(torusTransform);

    //createLight(resultEntity);
}

// vertices {(x, y, z, nx, ny, nz, cx, cy, cz)}
Qt3DCore::QEntity *createCustomMeshEntity2(Qt3DCore::QEntity *rootEntity, const QVector<triangle2> &triangles, QColor materialColor)
{
    // Torus
    Qt3DCore::QEntity *customMeshEntity = new Qt3DCore::QEntity(rootEntity);

    // Material
    //Qt3DRender::QMaterial *material = new Qt3DExtras::QPerVertexColorMaterial(customMeshEntity);
    Qt3DExtras::QDiffuseSpecularMaterial *material = new Qt3DExtras::QDiffuseSpecularMaterial(customMeshEntity);
    material->setDiffuse(materialColor);

    // Transform
    Qt3DCore::QTransform *transform = new Qt3DCore::QTransform(customMeshEntity);
    transform->setScale(0.9f);
    transform->setTranslation({9.0f, -9.0f, -9.0f});
    transform->setRotationX(-90);
    transform->setRotationY(180);

    // Custom Mesh (Tetrahedron)
    Qt3DRender::QGeometryRenderer *customMeshRenderer = new Qt3DRender::QGeometryRenderer(customMeshEntity);
    Qt3DRender::QGeometry *customGeometry = new Qt3DRender::QGeometry(customMeshRenderer);

    Qt3DRender::QBuffer *vertexDataBuffer = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::VertexBuffer, customGeometry);
    Qt3DRender::QBuffer *indexDataBuffer = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::IndexBuffer, customGeometry);

    // vec3 for position
    // vec3 for colors
    // vec3 for normals

    /*          2
               /|\
              / | \
             / /3\ \
             0/___\ 1
    */

    int vertexesCount = (triangles.size()*3); // 136*3=408

    // 4 distinct vertices
    QByteArray vertexBufferData; // 1 точка = ((0.0f, 0.0f, 0.0f), (0.0f, 0.0f, 0.0f), (0.0f, 0.0f, 0.0f))
    vertexBufferData.resize(vertexesCount * (3 + 3 + 3) * sizeof(float));


    // 1 точка = ((0.0f, 0.0f, 0.0f), (0.0f, 0.0f, 0.0f), (0.0f, 0.0f, 0.0f))
    // тут 4 точки
    float *rawVertexArray = reinterpret_cast<float *>(vertexBufferData.data());
    QVector3D color(0.0f, 0.0f, 0.0f);

    // v1 (v.x, v.y, v.z, n.x, n.y, n.z, r.z, r.y, r.z) = 9 * float
    int idx = 0;
    for (const triangle2 &v : triangles) {
        rawVertexArray[idx++] = v.Vertex_1.x();
        rawVertexArray[idx++] = v.Vertex_1.y();
        rawVertexArray[idx++] = v.Vertex_1.z();
        rawVertexArray[idx++] = v.Normal_vector.x();
        rawVertexArray[idx++] = v.Normal_vector.y();
        rawVertexArray[idx++] = v.Normal_vector.z();
        rawVertexArray[idx++] = color.x();
        rawVertexArray[idx++] = color.y();
        rawVertexArray[idx++] = color.z();

        rawVertexArray[idx++] = v.Vertex_2.x();
        rawVertexArray[idx++] = v.Vertex_2.y();
        rawVertexArray[idx++] = v.Vertex_2.z();
        rawVertexArray[idx++] = v.Normal_vector.x();
        rawVertexArray[idx++] = v.Normal_vector.y();
        rawVertexArray[idx++] = v.Normal_vector.z();
        rawVertexArray[idx++] = color.x();
        rawVertexArray[idx++] = color.y();
        rawVertexArray[idx++] = color.z();

        rawVertexArray[idx++] = v.Vertex_3.x();
        rawVertexArray[idx++] = v.Vertex_3.y();
        rawVertexArray[idx++] = v.Vertex_3.z();
        rawVertexArray[idx++] = v.Normal_vector.x();
        rawVertexArray[idx++] = v.Normal_vector.y();
        rawVertexArray[idx++] = v.Normal_vector.z();
        rawVertexArray[idx++] = color.x();
        rawVertexArray[idx++] = color.y();
        rawVertexArray[idx++] = color.z();
    }

    // Indices (408)
    QByteArray indexBufferData;
    indexBufferData.resize(vertexesCount * 3 * sizeof(ushort));
    ushort *rawIndexArray = reinterpret_cast<ushort *>(indexBufferData.data());

    for(int i=0; i<(vertexesCount * 3); i++)
    {
        rawIndexArray[i] = i;
    }

    vertexDataBuffer->setData(vertexBufferData);
    indexDataBuffer->setData(indexBufferData);

    // Attributes
    Qt3DRender::QAttribute *positionAttribute = new Qt3DRender::QAttribute();
    positionAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    positionAttribute->setBuffer(vertexDataBuffer);
    positionAttribute->setDataType(Qt3DRender::QAttribute::Float);
    positionAttribute->setDataSize(3);
    positionAttribute->setByteOffset(0);
    positionAttribute->setByteStride(9 * sizeof(float));
    positionAttribute->setCount(vertexesCount);
    positionAttribute->setName(Qt3DRender::QAttribute::defaultPositionAttributeName());

    Qt3DRender::QAttribute *normalAttribute = new Qt3DRender::QAttribute();
    normalAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    normalAttribute->setBuffer(vertexDataBuffer);
    normalAttribute->setDataType(Qt3DRender::QAttribute::Float);
    normalAttribute->setDataSize(3);
    normalAttribute->setByteOffset(3 * sizeof(float));
    normalAttribute->setByteStride(9 * sizeof(float));
    normalAttribute->setCount(vertexesCount);
    normalAttribute->setName(Qt3DRender::QAttribute::defaultNormalAttributeName());

    Qt3DRender::QAttribute *indexAttribute = new Qt3DRender::QAttribute();
    indexAttribute->setAttributeType(Qt3DRender::QAttribute::IndexAttribute);
    indexAttribute->setBuffer(indexDataBuffer);
    indexAttribute->setDataType(Qt3DRender::QAttribute::UnsignedShort);
    indexAttribute->setDataSize(1);
    indexAttribute->setByteOffset(0);
    indexAttribute->setByteStride(0);
    indexAttribute->setCount((vertexesCount*3));

    customGeometry->addAttribute(positionAttribute);
    customGeometry->addAttribute(normalAttribute);
    customGeometry->addAttribute(indexAttribute);

    customMeshRenderer->setInstanceCount(1);
    customMeshRenderer->setFirstVertex(0);
    customMeshRenderer->setFirstInstance(0);
    customMeshRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::Triangles);
    customMeshRenderer->setGeometry(customGeometry);
    // 4 faces of 3 points
    customMeshRenderer->setVertexCount((vertexesCount*3));

    customMeshEntity->addComponent(customMeshRenderer);
    customMeshEntity->addComponent(transform);
    customMeshEntity->addComponent(material);

    return customMeshEntity;
}

void createCustomMeshEntity(Qt3DCore::QEntity *rootEntity)
{
    // Torus
    Qt3DCore::QEntity *customMeshEntity = new Qt3DCore::QEntity(rootEntity);

    // Material
    Qt3DRender::QMaterial *material = new Qt3DExtras::QPerVertexColorMaterial(customMeshEntity);
    //Qt3DExtras::QDiffuseSpecularMaterial *material = new Qt3DExtras::QDiffuseSpecularMaterial(customMeshEntity);
    //material->setDiffuse(QColor(255, 0, 255));
    //material->setAlphaBlendingEnabled(false);

    // Transform
    Qt3DCore::QTransform *transform = new Qt3DCore::QTransform(customMeshEntity);
    transform->setScale(8.0f);

    // Custom Mesh (Tetrahedron)
    Qt3DRender::QGeometryRenderer *customMeshRenderer = new Qt3DRender::QGeometryRenderer(customMeshEntity);
    Qt3DRender::QGeometry *customGeometry = new Qt3DRender::QGeometry(customMeshRenderer);

    Qt3DRender::QBuffer *vertexDataBuffer = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::VertexBuffer, customGeometry);
    Qt3DRender::QBuffer *indexDataBuffer = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::IndexBuffer, customGeometry);

    // vec3 for position
    // vec3 for colors
    // vec3 for normals

    /*          2
               /|\
              / | \
             / /3\ \
             0/___\ 1
    */

    // 4 distinct vertices
    QByteArray vertexBufferData; // 1 точка = ((0.0f, 0.0f, 0.0f), (0.0f, 0.0f, 0.0f), (0.0f, 0.0f, 0.0f))
    vertexBufferData.resize(4 * (3 + 3 + 3) * sizeof(float));


    // Vertices +
    QVector3D v0(-1.0f, -1.0f, 0.0f);
    QVector3D v1(1.0f, -1.0f, 0.0f);
    QVector3D v2(0.0f, 1.0f, 0.0f);
    QVector3D v3(0.0f, 0.0f, -1.0f);

    // Faces Normals +
    QVector3D n023 = QVector3D::normal(v0, v2, v3);
    QVector3D n012 = QVector3D::normal(v0, v1, v2);
    QVector3D n310 = QVector3D::normal(v3, v1, v0);
    QVector3D n132 = QVector3D::normal(v1, v3, v2);

    // vertexes Normals
    QVector3D n0 = QVector3D(n023 + n012 + n310).normalized();
    QVector3D n1 = QVector3D(n132 + n012 + n310).normalized();
    QVector3D n2 = QVector3D(n132 + n012 + n023).normalized();
    QVector3D n3 = QVector3D(n132 + n310 + n023).normalized();

    // Colors
    QVector3D red(1.0f, 0.0f, 0.0f);
    QVector3D green(1.0f, 0.0f, 0.0f);
    QVector3D blue(1.0f, 0.0f, 0.0f);
    QVector3D white(1.0f, 0.0f, 0.0f);

    QVector<QVector3D> vertices = QVector<QVector3D>()
            << v0 << n0 << red // p[0]
            << v1 << n1 << blue // p[1]
            << v2 << n2 << green // p[2]
            << v3 << n3 << white; // p[3]

    // 1 точка = ((0.0f, 0.0f, 0.0f), (0.0f, 0.0f, 0.0f), (0.0f, 0.0f, 0.0f))
    // тут 4 точки
    float *rawVertexArray = reinterpret_cast<float *>(vertexBufferData.data());
    int idx = 0;

    // записывем данные в vertexBufferData в такой последовательности (4 точки)
    // v1 (v.x, v.y, v.z, n.x, n.y, n.z, r.z, r.y, r.z) = 9 * float
    for (const QVector3D &v : vertices) {
        rawVertexArray[idx++] = v.x(); // ((v.x, 0.0f, 0.0f) -
        rawVertexArray[idx++] = v.y(); // ((0.0f, v.yf, 0.0f) -
        rawVertexArray[idx++] = v.z(); // ((0.0f, 0.0f, v.zf) -
    }

    // Indices (12)
    QByteArray indexBufferData;
    indexBufferData.resize(4 * 3 * sizeof(ushort));
    ushort *rawIndexArray = reinterpret_cast<ushort *>(indexBufferData.data());

    // Front
    rawIndexArray[0] = 0;
    rawIndexArray[1] = 1;
    rawIndexArray[2] = 2;
    // Bottom
    rawIndexArray[3] = 3;
    rawIndexArray[4] = 1;
    rawIndexArray[5] = 0;
    // Left
    rawIndexArray[6] = 0;
    rawIndexArray[7] = 2;
    rawIndexArray[8] = 3;
    // Right
    rawIndexArray[9] = 1;
    rawIndexArray[10] = 3;
    rawIndexArray[11] = 2;

    vertexDataBuffer->setData(vertexBufferData);
    indexDataBuffer->setData(indexBufferData);

    // Attributes
    Qt3DRender::QAttribute *positionAttribute = new Qt3DRender::QAttribute();
    positionAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    positionAttribute->setBuffer(vertexDataBuffer);
    positionAttribute->setDataType(Qt3DRender::QAttribute::Float);
    positionAttribute->setDataSize(3);
    positionAttribute->setByteOffset(0);
    positionAttribute->setByteStride(9 * sizeof(float));
    positionAttribute->setCount(4);
    positionAttribute->setName(Qt3DRender::QAttribute::defaultPositionAttributeName());

    Qt3DRender::QAttribute *normalAttribute = new Qt3DRender::QAttribute();
    normalAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    normalAttribute->setBuffer(vertexDataBuffer);
    normalAttribute->setDataType(Qt3DRender::QAttribute::Float);
    normalAttribute->setDataSize(3);
    normalAttribute->setByteOffset(3 * sizeof(float));
    normalAttribute->setByteStride(9 * sizeof(float));
    normalAttribute->setCount(4);
    normalAttribute->setName(Qt3DRender::QAttribute::defaultNormalAttributeName());

    Qt3DRender::QAttribute *colorAttribute = new Qt3DRender::QAttribute();
    colorAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    colorAttribute->setBuffer(vertexDataBuffer);
    colorAttribute->setDataType(Qt3DRender::QAttribute::Float);
    colorAttribute->setDataSize(3);
    colorAttribute->setByteOffset(6 * sizeof(float));
    colorAttribute->setByteStride(9 * sizeof(float));
    colorAttribute->setCount(4);
    colorAttribute->setName(Qt3DRender::QAttribute::defaultColorAttributeName());

    Qt3DRender::QAttribute *indexAttribute = new Qt3DRender::QAttribute();
    indexAttribute->setAttributeType(Qt3DRender::QAttribute::IndexAttribute);
    indexAttribute->setBuffer(indexDataBuffer);
    indexAttribute->setDataType(Qt3DRender::QAttribute::UnsignedShort);
    indexAttribute->setDataSize(1);
    indexAttribute->setByteOffset(0);
    indexAttribute->setByteStride(0);
    indexAttribute->setCount(12);

    customGeometry->addAttribute(positionAttribute);
    //customGeometry->addAttribute(normalAttribute);
    customGeometry->addAttribute(colorAttribute);
    customGeometry->addAttribute(indexAttribute);

    customMeshRenderer->setInstanceCount(1);
    customMeshRenderer->setFirstVertex(0);
    customMeshRenderer->setFirstInstance(0);
    customMeshRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::Triangles);
    customMeshRenderer->setGeometry(customGeometry);
    // 4 faces of 3 points
    customMeshRenderer->setVertexCount(12);

    customMeshEntity->addComponent(customMeshRenderer);
    customMeshEntity->addComponent(transform);
    customMeshEntity->addComponent(material);

}
