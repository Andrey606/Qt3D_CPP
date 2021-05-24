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

struct VertexData
{
    VertexData()
    {}
    VertexData(QVector3D p, QVector2D t, QVector3D n) : position(p), textCoord(t), normal(n)
    {}

    QVector3D position;
    QVector2D textCoord;
    QVector3D normal;
    QVector3D tangent;
    QVector3D bitangent;
};

void createScene(Qt3DCore::QEntity *rootEntity);
void createCustomMeshEntity(Qt3DCore::QEntity *rootEntity);
void createCustomMeshEntity2(Qt3DCore::QEntity *rootEntity);
void createLight(Qt3DCore::QEntity *rootEntity);

Qt3DCore::QEntity *addObject(const QVector<VertexData> &vertexes,
                             const QVector<uint> &indexses,
                             Qt3DRender::QMaterial *material,
                             Qt3DCore::QNode *parent = Q_NULLPTR);




int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

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

    //createScene(rootEntity); // тор
    createCustomMeshEntity(rootEntity); // создание обьекта вручную
    //createCustomMeshEntity2(rootEntity);  // загрузка модели с файла

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
    camera->setPosition(QVector3D(0.0f, 0.0f, 40.0f));

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

void createLight(Qt3DCore::QEntity *rootEntity)
{
    // создадим отделную сущность для света
    Qt3DCore::QEntity *lightEntity = new Qt3DCore::QEntity(rootEntity);

    // создадим для этого ентити трансформ чтобы можно было вигать источник света
    Qt3DCore::QTransform *lightTransform = new Qt3DCore::QTransform(lightEntity);
    lightTransform->setTranslation(QVector3D(0.0f, 0.0f, 30.0f)); // передвигаем куда хотим

    // источник света уже сформирован (дефолтный но мы добавим свой точечный)
    Qt3DRender::QPointLight *pointLight = new Qt3DRender::QPointLight(lightEntity);
    pointLight->setColor(QColor(0, 0, 255));

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
    torusMesh->setRadius(15.0f);
    torusMesh->setMinorRadius(6.0f);
    torusMesh->setSlices(6); //это выставляем гладкость, кол-во фрагментов по внутренему радиусу
    torusMesh->setRings(6); //это выставляем гладкость, кол-во фрагментов по внешнему радиусу

    // теперь нужно попределить материал
    Qt3DExtras::QPhongMaterial *torusMaterial = new Qt3DExtras::QPhongMaterial(resultEntity);

    Qt3DCore::QTransform *torusTransform = new Qt3DCore::QTransform(resultEntity);

    // теперь те класы которые компоненты (унаследованы от QComponent) нужно скомпоновать их в resultEntity
    resultEntity->addComponent(torusMesh);
    resultEntity->addComponent(torusMaterial);
    resultEntity->addComponent(torusTransform);

    createLight(resultEntity);
}

// не работает?
void createCustomMeshEntity2(Qt3DCore::QEntity *rootEntity)
{
    Qt3DCore::QEntity *objectEntity = new Qt3DCore::QEntity(rootEntity);
    Qt3DRender::QSceneLoader *sceneLoader = new Qt3DRender::QSceneLoader(objectEntity);
    //sceneLoader->setSource(QUrl("qrc:/obj/bunny.3mf"));
    QUrl url = QUrl::fromLocalFile(":/obj/monkey.obj");
    sceneLoader->setSource(url);
    objectEntity->addComponent(sceneLoader);

    Qt3DCore::QTransform *objectTransform = new Qt3DCore::QTransform(objectEntity);
    objectTransform->setScale(5.0f);
    //objectTransform->setTranslation(QVector3D(0.0f, 0.0f, 30.0f));
    objectEntity->addComponent(objectTransform);
}

void createCustomMeshEntity(Qt3DCore::QEntity *rootEntity)
{
    // Torus
    Qt3DCore::QEntity *customMeshEntity = new Qt3DCore::QEntity(rootEntity);
    Qt3DCore::QEntity *customMeshEntity2 = new Qt3DCore::QEntity(rootEntity);

    // Material
    //Qt3DRender::QMaterial *material = new Qt3DExtras::QPerVertexColorMaterial(customMeshEntity);
    Qt3DExtras::QDiffuseSpecularMaterial *material = new Qt3DExtras::QDiffuseSpecularMaterial(customMeshEntity);
    material->setDiffuse(QColor(0, 255, 0));

    Qt3DExtras::QDiffuseSpecularMaterial *material2 = new Qt3DExtras::QDiffuseSpecularMaterial(customMeshEntity2);
    material2->setDiffuse(QColor(0, 0, 255));


    //createLight(customMeshEntity);

    // Transform
    Qt3DCore::QTransform *transform = new Qt3DCore::QTransform(customMeshEntity);
    transform->setScale(8.0f);

    Qt3DCore::QTransform *transform2 = new Qt3DCore::QTransform(customMeshEntity2);
    transform->setScale(8.0f);

    // Custom Mesh (Tetrahedron)
    Qt3DRender::QGeometryRenderer *customMeshRenderer = new Qt3DRender::QGeometryRenderer(customMeshEntity);
    Qt3DRender::QGeometry *customGeometry = new Qt3DRender::QGeometry(customMeshRenderer);

    Qt3DRender::QGeometryRenderer *customMeshRenderer2 = new Qt3DRender::QGeometryRenderer(customMeshEntity2);
    Qt3DRender::QGeometry *customGeometry2 = new Qt3DRender::QGeometry(customMeshRenderer2);

    Qt3DRender::QBuffer *vertexDataBuffer = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::VertexBuffer, customGeometry);
    Qt3DRender::QBuffer *indexDataBuffer = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::IndexBuffer, customGeometry);

    Qt3DRender::QBuffer *vertexDataBuffer2 = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::VertexBuffer, customGeometry2);
    Qt3DRender::QBuffer *indexDataBuffer2 = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::IndexBuffer, customGeometry2);

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

    // 2 distinct vertices
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
    QVector3D green(0.0f, 1.0f, 0.0f);
    QVector3D blue(0.0f, 0.0f, 1.0f);
    QVector3D white(1.0f, 1.0f, 1.0f);

    QVector<QVector3D> vertices = QVector<QVector3D>()
            << v0 << n0 << red
            << v1 << n1 << blue
            << v2 << n2 << green
            << v3 << n3 << white;

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
    customGeometry->addAttribute(normalAttribute);
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

    customMeshEntity2->addComponent(customMeshRenderer2);
    customMeshEntity2->addComponent(transform2);
    customMeshEntity2->addComponent(material2);

}

Qt3DCore::QEntity *addObject(const QVector<VertexData> &vertexes,
                             const QVector<uint> &indexses,
                             Qt3DRender::QMaterial *material,
                             Qt3DCore::QNode *parent)
{
    Qt3DCore::QEntity *object = new Qt3DCore::QEntity(parent);
    Qt3DRender::QGeometryRenderer *renderer = new Qt3DRender::QGeometryRenderer(object);
    Qt3DRender::QGeometry *geometry = new Qt3DRender::QGeometry(renderer);

    Qt3DRender::QBuffer *vertexBuffer = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::VertexBuffer, geometry);
    Qt3DRender::QBuffer *indexBuffer = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::IndexBuffer, geometry);

    Qt3DRender::QAttribute *positionAttribute = new Qt3DRender::QAttribute(geometry);
    Qt3DRender::QAttribute *normalAttribute = new Qt3DRender::QAttribute(geometry);
    Qt3DRender::QAttribute *texcoordAttribute = new Qt3DRender::QAttribute(geometry);
    Qt3DRender::QAttribute *indexAttribute = new Qt3DRender::QAttribute(geometry);

    const quint32 stride = (3+3+2) * sizeof (float);

    // заполняем атрибуты как для шейдеров
    positionAttribute->setName(Qt3DRender::QAttribute::defaultPositionAttributeName());
    positionAttribute->setDataType(Qt3DRender::QAttribute::Float); // 3 координаты со значением float
    positionAttribute->setDataSize(3);
    positionAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    positionAttribute->setBuffer(vertexBuffer);
    positionAttribute->setByteStride(stride);
    positionAttribute->setByteOffset(0);

    normalAttribute->setName(Qt3DRender::QAttribute::defaultNormalAttributeName());
    normalAttribute->setDataType(Qt3DRender::QAttribute::Float);
    normalAttribute->setDataSize(3);
    normalAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    normalAttribute->setBuffer(vertexBuffer);
    normalAttribute->setByteStride(stride);
    normalAttribute->setByteOffset(3*sizeof(float));

    texcoordAttribute->setName(Qt3DRender::QAttribute::defaultTextureCoordinateAttributeName());
    texcoordAttribute->setDataType(Qt3DRender::QAttribute::Float);
    texcoordAttribute->setDataSize(2);
    texcoordAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    texcoordAttribute->setBuffer(vertexBuffer);
    texcoordAttribute->setByteStride(stride);
    texcoordAttribute->setByteOffset((3+3)*sizeof(float));

    indexAttribute->setAttributeType(Qt3DRender::QAttribute::IndexAttribute);
    indexAttribute->setDataType(Qt3DRender::QAttribute::UnsignedInt);
    indexAttribute->setBuffer(indexBuffer);
    indexAttribute->setCount(indexses.size());

    // Torus
    //Qt3DCore::QEntity *customMeshEntity = new Qt3DCore::QEntity(rootEntity);

    // Material
    //Qt3DRender::QMaterial *material = new Qt3DExtras::QPerVertexColorMaterial(customMeshEntity);

    // Transform
    /*Qt3DCore::QTransform *transform = new Qt3DCore::QTransform(customMeshEntity);
    transform->setScale(8.0f);

    // Custom Mesh (Tetrahedron)
    Qt3DRender::QGeometryRenderer *customMeshRenderer = new Qt3DRender::QGeometryRenderer(customMeshEntity);
    Qt3DRender::QGeometry *customGeometry = new Qt3DRender::QGeometry(customMeshRenderer);

    */

    return object;
}
