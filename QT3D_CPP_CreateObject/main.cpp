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

Qt3DCore::QEntity *createScene();
void createLight(Qt3DCore::QEntity *rootEntity);



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
    Qt3DCore::QEntity *rootEntity = createScene(); // главная корневая сущность (посути это вся наша сцена, верхний уровень)

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
    cameraControler->setLookSpeed(100.0f); // меняем скорост вращение вокруг центра
    cameraControler->setLinearSpeed(50.0f); // скорость перемещение камеры влево вправо вверхвних (не кручение)

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
    pointLight->setColor(QColor(255, 0, 255));

    lightEntity->addComponent(lightTransform);
    lightEntity->addComponent(pointLight);
}

Qt3DCore::QEntity *createScene()
{
    Qt3DCore::QEntity *resultEntity = new Qt3DCore::QEntity;

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

    return resultEntity;
}
