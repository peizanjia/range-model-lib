#include <QtTest/QtTest>
#include <QFile>
#include <QTemporaryDir>

#include "rml_file_header.h"
#include "rml_file_io.h"
#include "rml_model.h"

namespace
{

RModel makeModel()
{
    RModel model;
    model.setNNodes(3);
    model.setNode(0,RNode(0.0,0.0,0.0));
    model.setNode(1,RNode(1.0,0.0,0.0));
    model.setNode(2,RNode(0.0,1.0,0.0));
    model.setNElements(1);

    RElement element(R_ELEMENT_TRUSS1);
    element.setNodeId(0,0);
    element.setNodeId(1,1);
    model.setElement(0,element);

    model.setNodePhysicalId(0,41);
    model.setNodePhysicalId(1,97);
    model.setNodePhysicalId(2,103);
    model.setElementPhysicalId(0,501);
    return model;
}

qint64 filePosition(RFile &file, RFile::Type type)
{
    return type == RFile::ASCII ? file.getTextStream().pos() : file.pos();
}

bool makeLegacyFile(const QString &currentFileName,
                    const QString &legacyFileName,
                    RFile::Type type)
{
    RFile input(currentFileName,type);
    const QIODevice::OpenMode mode = type == RFile::ASCII
        ? QIODevice::ReadOnly | QIODevice::Text
        : QIODevice::ReadOnly;
    if (!input.open(mode))
    {
        return false;
    }

    RFileHeader header;
    if (type == RFile::ASCII) RFileIO::readAscii(input,header);
    else RFileIO::readBinary(input,header);
    const qint64 bodyStart = filePosition(input,type);
    input.setVersion(header.getVersion());

    QString name;
    QString description;
    uint nNodes = 0;
    uint nElements = 0;
    if (type == RFile::ASCII)
    {
        RFileIO::readAscii(input,name);
        RFileIO::readAscii(input,description);
        RFileIO::readAscii(input,nNodes);
    }
    else
    {
        RFileIO::readBinary(input,name);
        RFileIO::readBinary(input,description);
        RFileIO::readBinary(input,nNodes);
    }
    RNode node;
    for (uint i=0;i<nNodes;i++)
    {
        if (type == RFile::ASCII) RFileIO::readAscii(input,node);
        else RFileIO::readBinary(input,node);
    }
    if (type == RFile::ASCII) RFileIO::readAscii(input,nElements);
    else RFileIO::readBinary(input,nElements);
    RElement element;
    for (uint i=0;i<nElements;i++)
    {
        if (type == RFile::ASCII) RFileIO::readAscii(input,element);
        else RFileIO::readBinary(input,element);
    }
    const qint64 elementEnd = filePosition(input,type);

    uint value = 0;
    for (uint i=0;i<nNodes+nElements+2;i++)
    {
        if (type == RFile::ASCII) RFileIO::readAscii(input,value);
        else RFileIO::readBinary(input,value);
    }
    const qint64 tableEnd = filePosition(input,type);
    input.close();
    if (bodyStart < 0 || elementEnd < bodyStart || tableEnd < elementEnd)
    {
        return false;
    }

    QFile rawFile(currentFileName);
    if (!rawFile.open(QIODevice::ReadOnly))
    {
        return false;
    }
    const QByteArray bytes = rawFile.readAll();
    rawFile.close();
    if (tableEnd > bytes.size())
    {
        return false;
    }

    RSaveFile output(legacyFileName,type == RFile::ASCII ? RSaveFile::ASCII : RSaveFile::BINARY);
    if (!output.open(QIODevice::WriteOnly))
    {
        return false;
    }
    const RFileHeader legacyHeader(R_FILE_TYPE_MODEL,RVersion(1,3,16));
    if (type == RFile::ASCII)
    {
        RFileIO::writeAscii(output,legacyHeader);
        output.getTextStream().flush();
    }
    else
    {
        RFileIO::writeBinary(output,legacyHeader);
    }
    output.write(bytes.mid(bodyStart,elementEnd-bodyStart));
    output.write(bytes.mid(tableEnd));
    return output.commit();
}

}

class TestModelPhysicalIds : public QObject
{
    Q_OBJECT

private slots:
    void identifiersRemainStableAcrossEdits();
    void currentAsciiAndBinaryFormatsRoundTripIdentifiers();
    void legacyAsciiAndBinaryModelsReceiveIdentifiers();
};

void TestModelPhysicalIds::identifiersRemainStableAcrossEdits()
{
    RModel model;
    model.setNNodes(3);
    model.setNode(0,RNode(0.0,0.0,0.0));
    model.setNode(1,RNode(1.0,0.0,0.0));
    model.setNode(2,RNode(2.0,0.0,0.0));
    QCOMPARE(model.getNodePhysicalId(0),1u);
    QCOMPARE(model.getNodePhysicalId(1),2u);
    QCOMPARE(model.getNodePhysicalId(2),3u);

    model.setNodePhysicalId(0,40);
    model.setNodePhysicalId(1,70);
    model.setNodePhysicalId(2,90);
    QVERIFY_EXCEPTION_THROWN(model.setNodePhysicalId(2,70),RError);
    QCOMPARE(model.findNodePositionByPhysicalId(70),1u);

    RNode replacement(4.0,0.0,0.0);
    model.setNode(0,replacement);
    QCOMPARE(model.getNodePhysicalId(0),40u);
    model.removeNode(1);
    QCOMPARE(model.findNodePositionByPhysicalId(70),RConstants::eod);
    QCOMPARE(model.getNodePhysicalId(1),90u);
    model.addNode(model.getNode(0));
    QCOMPARE(model.getNodePhysicalId(2),91u);

    model.setNElements(2);
    model.setElementPhysicalId(0,200);
    model.setElementPhysicalId(1,400);
    QVERIFY_EXCEPTION_THROWN(model.setElementPhysicalId(1,200),RError);
    model.setNElements(1);
    model.setNElements(2);
    QCOMPARE(model.getElementPhysicalId(0),200u);
    QCOMPARE(model.getElementPhysicalId(1),401u);

    RModel copy(model);
    QCOMPARE(copy.getNodePhysicalId(0),40u);
    QCOMPARE(copy.getNodePhysicalId(1),90u);
    QCOMPARE(copy.getElementPhysicalId(1),401u);
}

void TestModelPhysicalIds::currentAsciiAndBinaryFormatsRoundTripIdentifiers()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const RModel source = makeModel();
    QCOMPARE(RModel::version.toString(),QStringLiteral("1.3.17"));

    for (bool binary : {false,true})
    {
        const QString fileName = directory.filePath(binary ? QStringLiteral("ids.rbm") : QStringLiteral("ids.ram"));
        source.write(fileName,false);

        RModel restored;
        restored.read(fileName);
        QCOMPARE(restored.getNodePhysicalId(0),41u);
        QCOMPARE(restored.getNodePhysicalId(1),97u);
        QCOMPARE(restored.getNodePhysicalId(2),103u);
        QCOMPARE(restored.findNodePositionByPhysicalId(97),1u);
        QCOMPARE(restored.getElementPhysicalId(0),501u);

        restored.addNode(RNode(4.0,0.0,0.0));
        QCOMPARE(restored.getNodePhysicalId(3),104u);
        restored.setNElements(2);
        QCOMPARE(restored.getElementPhysicalId(1),502u);
    }
}

void TestModelPhysicalIds::legacyAsciiAndBinaryModelsReceiveIdentifiers()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const RModel source = makeModel();

    for (bool binary : {false,true})
    {
        const QString currentFileName = directory.filePath(binary ? QStringLiteral("current.rbm") : QStringLiteral("current.ram"));
        const QString legacyFileName = directory.filePath(binary ? QStringLiteral("legacy.rbm") : QStringLiteral("legacy.ram"));
        source.write(currentFileName,false);
        QVERIFY(makeLegacyFile(currentFileName,legacyFileName,binary ? RFile::BINARY : RFile::ASCII));

        RModel restored;
        restored.read(legacyFileName);
        QCOMPARE(restored.getNodePhysicalId(0),1u);
        QCOMPARE(restored.getNodePhysicalId(1),2u);
        QCOMPARE(restored.getNodePhysicalId(2),3u);
        QCOMPARE(restored.getElementPhysicalId(0),1u);
        restored.addNode(RNode(4.0,0.0,0.0));
        QCOMPARE(restored.getNodePhysicalId(3),4u);
    }
}

QTEST_APPLESS_MAIN(TestModelPhysicalIds)
#include "tst_model_physical_ids.moc"
