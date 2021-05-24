#include <QCoreApplication>
#include <stlparser.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    stlParser parser;
    parser.uploadFile();
    parser.overhangsThatShouldHaveSupports();

    return a.exec();
}
