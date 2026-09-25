#include <QtTest/QtTest>
#include <QFile>
#include <QTemporaryDir>

#include <cstring>

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

bool corruptFirstNodePhysicalId(const QString &fileName, RFile::Type type)
{
    RFile input(fileName,type);
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
    if (nNodes < 2)
    {
        return false;
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

    if (type == RFile::ASCII)
    {
        input.getTextStream().skipWhiteSpace();
    }
    const qint64 firstIdPosition = filePosition(input,type);
    uint firstId = 0;
    if (type == RFile::ASCII) RFileIO::readAscii(input,firstId);
    else RFileIO::readBinary(input,firstId);
    if (type == RFile::ASCII)
    {
        input.getTextStream().skipWhiteSpace();
    }
    uint secondId = 0;
    if (type == RFile::ASCII) RFileIO::readAscii(input,secondId);
    else RFileIO::readBinary(input,secondId);
    input.close();

    if (firstId == secondId || firstIdPosition < 0)
    {
        return false;
    }

    QFile rawFile(fileName);
    if (!rawFile.open(QIODevice::ReadOnly))
    {
        return false;
    }
    QByteArray bytes = rawFile.readAll();
    rawFile.close();

    QByteArray oldValue;
    QByteArray newValue;
    if (type == RFile::ASCII)
    {
        oldValue = QByteArray::number(firstId);
        newValue = QByteArray::number(secondId);
        if (oldValue.size() != newValue.size() || bytes.mid(firstIdPosition,oldValue.size()) != oldValue)
        {
            return false;
        }
    }
    else
    {
        oldValue.resize(sizeof(firstId));
        newValue.resize(sizeof(secondId));
        std::memcpy(oldValue.data(),&firstId,sizeof(firstId));
        std::memcpy(newValue.data(),&secondId,sizeof(secondId));
        if (bytes.mid(firstIdPosition,oldValue.size()) != oldValue)
        {
            return false;
        }
    }
    bytes.replace(firstIdPosition,oldValue.size(),newValue);

    if (!rawFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        return false;
    }
    const bool written = rawFile.write(bytes) == bytes.size();
    rawFile.close();
    return written;
}

}

class TestModelPhysicalIds : public QObject
{
    Q_OBJECT

private slots:
    void identifiersRemainStableAcrossEdits();
    void resizeFailsAtomicallyWhenPhysicalIdentifiersAreExhausted();
    void currentAsciiAndBinaryFormatsRoundTripIdentifiers();
    void legacyAsciiAndBinaryModelsReceiveIdentifiers();
    void malformedCurrentIdentifiersDoNotReplaceExistingModel();
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

void TestModelPhysicalIds::resizeFailsAtomicallyWhenPhysicalIdentifiersAreExhausted()
{
    RModel nodes;
    nodes.setNNodes(1);
    nodes.setNode(0,RNode(3.0,4.0,5.0));
    nodes.setNodePhysicalId(0,RConstants::eod - 2);

    QVERIFY_EXCEPTION_THROWN(nodes.setNNodes(3),RError);
    QCOMPARE(nodes.getNNodes(),1u);
    QCOMPARE(nodes.RResults::getNNodes(),1u);
    QCOMPARE(nodes.getNodePhysicalId(0),RConstants::eod - 2);
    QCOMPARE(nodes.getNode(0).getX(),3.0);

    nodes.setNNodes(2);
    QCOMPARE(nodes.getNodePhysicalId(1),RConstants::eod - 1);
    QVERIFY_EXCEPTION_THROWN(nodes.setNNodes(3),RError);
    QCOMPARE(nodes.getNNodes(),2u);
    QCOMPARE(nodes.RResults::getNNodes(),2u);
    QCOMPARE(nodes.getNodePhysicalId(1),RConstants::eod - 1);

    RModel elements;
    elements.setNElements(1);
    RElement existingElement(R_ELEMENT_TRUSS1);
    elements.setElement(0,existingElement);
    elements.setElementPhysicalId(0,RConstants::eod - 2);

    QVERIFY_EXCEPTION_THROWN(elements.setNElements(3),RError);
    QCOMPARE(elements.getNElements(),1u);
    QCOMPARE(elements.RResults::getNElements(),1u);
    QCOMPARE(elements.getElementPhysicalId(0),RConstants::eod - 2);
    QCOMPARE(elements.getElement(0).getType(),R_ELEMENT_TRUSS1);

    elements.setNElements(2);
    QCOMPARE(elements.getElementPhysicalId(1),RConstants::eod - 1);
    QVERIFY_EXCEPTION_THROWN(elements.setNElements(3),RError);
    QCOMPARE(elements.getNElements(),2u);
    QCOMPARE(elements.RResults::getNElements(),2u);
    QCOMPARE(elements.getElementPhysicalId(1),RConstants::eod - 1);
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

void TestModelPhysicalIds::malformedCurrentIdentifiersDoNotReplaceExistingModel()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    for (bool binary : {false,true})
    {
        const RFile::Type type = binary ? RFile::BINARY : RFile::ASCII;
        const QString fileName = directory.filePath(binary ? QStringLiteral("malformed.rbm") : QStringLiteral("malformed.ram"));

        RModel incoming = makeModel();
        incoming.setName(QStringLiteral("replacement"));
        incoming.setNode(0,RNode(100.0,0.0,0.0));
        incoming.write(fileName,false);
        QVERIFY(corruptFirstNodePhysicalId(fileName,type));

        RModel existing = makeModel();
        existing.setName(QStringLiteral("preserved"));
        existing.setNode(0,RNode(-10.0,0.0,0.0));
        QVERIFY_EXCEPTION_THROWN(existing.read(fileName),RError);

        QCOMPARE(existing.getName(),QStringLiteral("preserved"));
        QCOMPARE(existing.getNNodes(),3u);
        QCOMPARE(existing.getNElements(),1u);
        QCOMPARE(existing.getNode(0).getX(),-10.0);
        QCOMPARE(existing.getNodePhysicalId(0),41u);
        QCOMPARE(existing.getNodePhysicalId(1),97u);
        QCOMPARE(existing.getElementPhysicalId(0),501u);
    }
}

QTEST_APPLESS_MAIN(TestModelPhysicalIds)
#include "tst_model_physical_ids.moc"
