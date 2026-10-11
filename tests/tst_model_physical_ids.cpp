#include <QtTest/QtTest>
#include <QFile>
#include <QTemporaryDir>

#include <algorithm>
#include <cstring>

#include "rml_file_header.h"
#include "rml_file_io.h"
#include "rml_model.h"

namespace
{

class PhysicalIdCounterTestModel : public RModel
{
public:
    void setNextElementGroupPhysicalId(uint physicalId)
    {
        this->nextElementGroupPhysicalId = physicalId;
    }
};

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

RModel makeGroupModel()
{
    RModel model = makeModel();
    RPoint point;
    RLine line;
    RSurface surface;
    RVolume volume;
    point.setName(QStringLiteral("point"));
    line.setName(QStringLiteral("line"));
    surface.setName(QStringLiteral("surface"));
    volume.setName(QStringLiteral("volume"));
    model.addPoint(point);
    model.addLine(line);
    model.addSurface(surface);
    model.addVolume(volume);
    return model;
}

QString legacyV17FixturePath(bool binary)
{
    return QStringLiteral(RANGE_MODEL_TEST_FIXTURE_DIR)
         + (binary ? QStringLiteral("/model-v17.rbm") : QStringLiteral("/model-v17.ram"));
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

bool corruptElementGroupPhysicalId(const QString &fileName,
                                   RFile::Type type,
                                   REntityGroupType targetType,
                                   uint replacementId)
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
    if (header.getVersion() < RVersion(1,3,18))
    {
        return false;
    }

    const auto read = [&input,type](auto &value)
    {
        if (type == RFile::ASCII) RFileIO::readAscii(input,value);
        else RFileIO::readBinary(input,value);
    };

    QString name;
    QString description;
    uint nNodes = 0;
    uint nElements = 0;
    read(name);
    read(description);
    read(nNodes);
    RNode node;
    for (uint i=0;i<nNodes;i++) read(node);
    read(nElements);
    RElement element;
    for (uint i=0;i<nElements;i++) read(element);

    uint ignoredId = 0;
    for (uint i=0;i<nNodes+nElements+2;i++) read(ignoredId);

    uint nPoints = 0;
    read(nPoints);
    qint64 idPosition = -1;
    uint originalId = 0;
    if (targetType == R_ENTITY_GROUP_POINT)
    {
        if (nPoints == 0) return false;
        if (type == RFile::ASCII) input.getTextStream().skipWhiteSpace();
        idPosition = filePosition(input,type);
        RPoint point;
        read(point);
        originalId = point.getPhysicalId();
    }
    else
    {
        RPoint point;
        for (uint i=0;i<nPoints;i++) read(point);
        uint nLines = 0;
        read(nLines);
        if (targetType != R_ENTITY_GROUP_LINE || nLines == 0) return false;
        if (type == RFile::ASCII) input.getTextStream().skipWhiteSpace();
        idPosition = filePosition(input,type);
        RLine line;
        read(line);
        originalId = line.getPhysicalId();
    }
    input.close();

    if (idPosition < 0)
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
        oldValue = QByteArray::number(originalId);
        newValue = QByteArray::number(replacementId);
        if (bytes.mid(idPosition,oldValue.size()) != oldValue)
        {
            return false;
        }
    }
    else
    {
        oldValue.resize(sizeof(originalId));
        newValue.resize(sizeof(replacementId));
        std::memcpy(oldValue.data(),&originalId,sizeof(originalId));
        std::memcpy(newValue.data(),&replacementId,sizeof(replacementId));
        if (bytes.mid(idPosition,oldValue.size()) != oldValue)
        {
            return false;
        }
    }
    bytes.replace(idPosition,oldValue.size(),newValue);

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
    void elementGroupPhysicalIdsRemainStableAcrossEdits();
    void elementGroupResizeFailsAtomicallyWhenIdentifiersAreExhausted();
    void resizeFailsAtomicallyWhenPhysicalIdentifiersAreExhausted();
    void currentAsciiAndBinaryFormatsRoundTripIdentifiers();
    void asciiAndBinaryResultValuesRoundTripExactly();
    void legacyV17FixturesReceiveDeterministicElementGroupIdentifiers();
    void legacyAsciiAndBinaryModelsReceiveIdentifiers();
    void malformedCurrentIdentifiersDoNotReplaceExistingModel();
    void malformedElementGroupIdentifiersAreRejected();
    void futureModelVersionsAreRejectedBeforeReadingBody();
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

void TestModelPhysicalIds::elementGroupPhysicalIdsRemainStableAcrossEdits()
{
    RModel model;
    model.setNPoints(2);
    model.setNLines(1);
    model.setNSurfaces(1);
    model.setNVolumes(1);
    QCOMPARE(model.getPoint(0).getPhysicalId(),1u);
    QCOMPARE(model.getPoint(1).getPhysicalId(),2u);
    QCOMPARE(model.getLine(0).getPhysicalId(),3u);
    QCOMPARE(model.getSurface(0).getPhysicalId(),4u);
    QCOMPARE(model.getVolume(0).getPhysicalId(),5u);

    RPoint replacement(model.getPoint(1));
    replacement.setName(QStringLiteral("replacement"));
    model.setPoint(0,replacement);
    QCOMPARE(model.getPoint(0).getPhysicalId(),1u);
    QCOMPARE(model.getPoint(0).getName(),QStringLiteral("replacement"));

    RLine replacementLine(model.getLine(0));
    replacementLine.setName(QStringLiteral("replacement line"));
    model.setLine(0,replacementLine);
    QCOMPARE(model.getLine(0).getPhysicalId(),3u);
    RSurface replacementSurface(model.getSurface(0));
    replacementSurface.setName(QStringLiteral("replacement surface"));
    model.setSurface(0,replacementSurface);
    QCOMPARE(model.getSurface(0).getPhysicalId(),4u);
    RVolume replacementVolume(model.getVolume(0));
    replacementVolume.setName(QStringLiteral("replacement volume"));
    model.setVolume(0,replacementVolume);
    QCOMPARE(model.getVolume(0).getPhysicalId(),5u);

    std::swap(model.getPoint(0),model.getPoint(1));
    model.getPoint(0).setName(QStringLiteral("renamed after swap"));
    QCOMPARE(model.getPoint(0).getPhysicalId(),2u);
    QCOMPARE(model.getPoint(1).getPhysicalId(),1u);
    QCOMPARE(model.getPoint(0).getName(),QStringLiteral("renamed after swap"));

    REntityGroupType groupType = R_ENTITY_GROUP_NONE;
    uint localIndex = RConstants::eod;
    QVERIFY(model.findElementGroupByPhysicalId(2,&groupType,&localIndex));
    QCOMPARE(int(groupType),int(R_ENTITY_GROUP_POINT));
    QCOMPARE(localIndex,0u);
    QVERIFY(model.findElementGroupByPhysicalId(3,&groupType,&localIndex));
    QCOMPARE(int(groupType),int(R_ENTITY_GROUP_LINE));
    QCOMPARE(localIndex,0u);
    QVERIFY(!model.findElementGroupByPhysicalId(0,&groupType,&localIndex));
    QCOMPARE(int(groupType),int(R_ENTITY_GROUP_NONE));
    QCOMPARE(localIndex,RConstants::eod);

    RModel copy(model);
    QCOMPARE(copy.getPoint(0).getPhysicalId(),2u);
    copy.removePoint(1);
    copy.addPoint(model.getPoint(0));
    QCOMPARE(copy.getPoint(1).getPhysicalId(),6u);
    copy.setNLines(0);
    copy.addLine(model.getLine(0));
    QCOMPARE(copy.getLine(0).getPhysicalId(),7u);

    RModel assigned;
    assigned = model;
    QCOMPARE(assigned.getPoint(0).getPhysicalId(),2u);
    assigned.addPoint(model.getPoint(0));
    QCOMPARE(assigned.getPoint(2).getPhysicalId(),6u);

    RModel otherModel;
    otherModel.addPoint(model.getPoint(0));
    QCOMPARE(otherModel.getPoint(0).getPhysicalId(),1u);
    QVERIFY(otherModel.getPoint(0).getPhysicalId() != model.getPoint(0).getPhysicalId());

    RModel resized;
    resized.setNPoints(2);
    resized.setNPoints(1);
    resized.setNPoints(2);
    QCOMPARE(resized.getPoint(1).getPhysicalId(),3u);

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath(QStringLiteral("group-counter.ram"));
    model.removeVolume(0);
    QCOMPARE(model.getNElementGroups(),4u);
    QCOMPARE(model.write(path,false),path);
    RModel restored;
    restored.read(path);
    restored.addVolume(RVolume());
    QCOMPARE(restored.getVolume(0).getPhysicalId(),6u);
}

void TestModelPhysicalIds::elementGroupResizeFailsAtomicallyWhenIdentifiersAreExhausted()
{
    PhysicalIdCounterTestModel model;
    model.setNPoints(1);
    model.setNextElementGroupPhysicalId(RConstants::eod - 2);

    QVERIFY_EXCEPTION_THROWN(model.setNPoints(4),RError);
    QCOMPARE(model.getNPoints(),1u);
    QCOMPARE(model.getPoint(0).getPhysicalId(),1u);

    model.setNPoints(2);
    QCOMPARE(model.getPoint(1).getPhysicalId(),RConstants::eod - 2);
    model.setNPoints(3);
    QCOMPARE(model.getPoint(2).getPhysicalId(),RConstants::eod - 1);
    QVERIFY_EXCEPTION_THROWN(model.setNPoints(4),RError);
    QCOMPARE(model.getNPoints(),3u);
    QCOMPARE(model.getPoint(0).getPhysicalId(),1u);
    QCOMPARE(model.getPoint(1).getPhysicalId(),RConstants::eod - 2);
    QCOMPARE(model.getPoint(2).getPhysicalId(),RConstants::eod - 1);
    QVERIFY_EXCEPTION_THROWN(model.addLine(RLine()),RError);
    QCOMPARE(model.getNLines(),0u);
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
    const RModel source = makeGroupModel();
    QCOMPARE(RModel::version.toString(),QStringLiteral("1.3.21"));

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
        QCOMPARE(restored.getPoint(0).getPhysicalId(),source.getPoint(0).getPhysicalId());
        QCOMPARE(restored.getLine(0).getPhysicalId(),source.getLine(0).getPhysicalId());
        QCOMPARE(restored.getLine(1).getPhysicalId(),source.getLine(1).getPhysicalId());
        QCOMPARE(restored.getSurface(0).getPhysicalId(),source.getSurface(0).getPhysicalId());
        QCOMPARE(restored.getVolume(0).getPhysicalId(),source.getVolume(0).getPhysicalId());

        REntityGroupType groupType = R_ENTITY_GROUP_NONE;
        uint localIndex = RConstants::eod;
        QVERIFY(restored.findElementGroupByPhysicalId(source.getSurface(0).getPhysicalId(),&groupType,&localIndex));
        QCOMPARE(int(groupType),int(R_ENTITY_GROUP_SURFACE));
        QCOMPARE(localIndex,0u);

        restored.addNode(RNode(4.0,0.0,0.0));
        QCOMPARE(restored.getNodePhysicalId(3),104u);
        restored.setNElements(2);
        QCOMPARE(restored.getElementPhysicalId(1),502u);
        restored.addPoint(source.getPoint(0));
        QCOMPARE(restored.getPoint(1).getPhysicalId(),6u);
        QVERIFY(restored.getPoint(1).getPhysicalId() != source.getPoint(0).getPhysicalId());
    }
}

void TestModelPhysicalIds::asciiAndBinaryResultValuesRoundTripExactly()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    RModel source = makeModel();
    const double expected = -0.012345678901234567;
    const uint variablePosition = source.addVariable(RVariable(R_VARIABLE_VELOCITY));
    RVariable &velocity = source.getVariable(variablePosition);
    velocity.setApplyType(R_VARIABLE_APPLY_NODE);
    velocity.resize(3,source.getNNodes());
    velocity.setValue(0,1,expected);

    for (bool binary : {false,true})
    {
        const QString path = directory.filePath(binary ? QStringLiteral("values.rbm")
                                                     : QStringLiteral("values.ram"));
        QCOMPARE(source.write(path,false),path);
        RModel restored;
        restored.read(path);
        const uint restoredPosition = restored.findVariable(R_VARIABLE_VELOCITY);
        QVERIFY(restoredPosition != RConstants::eod);
        QCOMPARE(restored.getVariable(restoredPosition).getValue(0,1),expected);
    }
}

void TestModelPhysicalIds::legacyV17FixturesReceiveDeterministicElementGroupIdentifiers()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    for (bool binary : {false,true})
    {
        const QString fixturePath = legacyV17FixturePath(binary);
        QVERIFY(QFile::exists(fixturePath));

        RFile fixture(fixturePath,binary ? RFile::BINARY : RFile::ASCII);
        QVERIFY(fixture.open(QIODevice::ReadOnly));
        RFileHeader header;
        if (binary) RFileIO::readBinary(fixture,header);
        else RFileIO::readAscii(fixture,header);
        QCOMPARE(header.getVersion().toString(),QStringLiteral("1.3.17"));
        fixture.close();

        RModel restored;
        restored.read(fixturePath);
        uint expectedGroupId = 1;
        for (uint i=0;i<restored.getNPoints();i++)
        {
            QCOMPARE(restored.getPoint(i).getPhysicalId(),expectedGroupId++);
        }
        for (uint i=0;i<restored.getNLines();i++)
        {
            QCOMPARE(restored.getLine(i).getPhysicalId(),expectedGroupId++);
        }
        for (uint i=0;i<restored.getNSurfaces();i++)
        {
            QCOMPARE(restored.getSurface(i).getPhysicalId(),expectedGroupId++);
        }
        for (uint i=0;i<restored.getNVolumes();i++)
        {
            QCOMPARE(restored.getVolume(i).getPhysicalId(),expectedGroupId++);
        }
        RPoint addedPoint;
        restored.addPoint(addedPoint);
        QCOMPARE(restored.getPoint(restored.getNPoints()-1).getPhysicalId(),expectedGroupId);
    }
}

void TestModelPhysicalIds::legacyAsciiAndBinaryModelsReceiveIdentifiers()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    for (bool binary : {false,true})
    {
        const QString fixturePath = legacyV17FixturePath(binary);
        const QString legacyFileName = directory.filePath(binary ? QStringLiteral("legacy-v16.rbm") : QStringLiteral("legacy-v16.ram"));
        QVERIFY(makeLegacyFile(fixturePath,legacyFileName,binary ? RFile::BINARY : RFile::ASCII));

        RModel restored;
        restored.read(legacyFileName);
        for (uint i=0;i<restored.getNNodes();i++)
        {
            QCOMPARE(restored.getNodePhysicalId(i),i+1);
        }
        for (uint i=0;i<restored.getNElements();i++)
        {
            QCOMPARE(restored.getElementPhysicalId(i),i+1);
        }
        uint expectedGroupId = 1;
        for (uint i=0;i<restored.getNPoints();i++)
        {
            QCOMPARE(restored.getPoint(i).getPhysicalId(),expectedGroupId++);
        }
        for (uint i=0;i<restored.getNLines();i++)
        {
            QCOMPARE(restored.getLine(i).getPhysicalId(),expectedGroupId++);
        }
        for (uint i=0;i<restored.getNSurfaces();i++)
        {
            QCOMPARE(restored.getSurface(i).getPhysicalId(),expectedGroupId++);
        }
        for (uint i=0;i<restored.getNVolumes();i++)
        {
            QCOMPARE(restored.getVolume(i).getPhysicalId(),expectedGroupId++);
        }
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

void TestModelPhysicalIds::malformedElementGroupIdentifiersAreRejected()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const RModel source = makeGroupModel();
    for (bool binary : {false,true})
    {
        const RFile::Type type = binary ? RFile::BINARY : RFile::ASCII;
        for (uint corruption=0;corruption<3;corruption++)
        {
            const QString fileName = directory.filePath(
                QStringLiteral("malformed-group-%1-%2.%3")
                    .arg(binary ? QStringLiteral("binary") : QStringLiteral("ascii"))
                    .arg(corruption)
                    .arg(binary ? QStringLiteral("rbm") : QStringLiteral("ram")));
            source.write(fileName,false);

            const REntityGroupType targetType = corruption == 2
                ? R_ENTITY_GROUP_LINE : R_ENTITY_GROUP_POINT;
            const uint replacementId = corruption == 0 ? 0
                                     : corruption == 1 ? RConstants::eod
                                     : source.getPoint(0).getPhysicalId();
            QVERIFY(corruptElementGroupPhysicalId(fileName,type,targetType,replacementId));

            RModel existing = makeGroupModel();
            existing.setName(QStringLiteral("preserved"));
            QVERIFY_EXCEPTION_THROWN(existing.read(fileName),RError);
            QCOMPARE(existing.getName(),QStringLiteral("preserved"));
            QCOMPARE(existing.getPoint(0).getPhysicalId(),source.getPoint(0).getPhysicalId());
            QCOMPARE(existing.getLine(0).getPhysicalId(),source.getLine(0).getPhysicalId());
            QCOMPARE(existing.getSurface(0).getPhysicalId(),source.getSurface(0).getPhysicalId());
            QCOMPARE(existing.getVolume(0).getPhysicalId(),source.getVolume(0).getPhysicalId());
        }
    }

    RModel duplicate = makeGroupModel();
    duplicate.getLine(0).RElementGroup::operator=(duplicate.getPoint(0));
    REntityGroupType groupType = R_ENTITY_GROUP_NONE;
    uint localIndex = RConstants::eod;
    QVERIFY_EXCEPTION_THROWN(duplicate.findElementGroupByPhysicalId(source.getPoint(0).getPhysicalId(),&groupType,&localIndex),RError);
    const QString duplicateWritePath = directory.filePath(QStringLiteral("duplicate-in-memory.ram"));
    QVERIFY_EXCEPTION_THROWN(duplicate.write(duplicateWritePath,false),RError);

    RModel zero = makeGroupModel();
    zero.getPoint(0) = RPoint();
    QVERIFY_EXCEPTION_THROWN(zero.findElementGroupByPhysicalId(source.getPoint(0).getPhysicalId(),&groupType,&localIndex),RError);
    QVERIFY_EXCEPTION_THROWN(zero.write(duplicateWritePath,false),RError);
}

void TestModelPhysicalIds::futureModelVersionsAreRejectedBeforeReadingBody()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    for (bool binary : {false,true})
    {
        const QString fileName = directory.filePath(binary ? QStringLiteral("future.rbm")
                                                           : QStringLiteral("future.ram"));
        RSaveFile output(fileName,binary ? RSaveFile::BINARY : RSaveFile::ASCII);
        QVERIFY(output.open(QIODevice::WriteOnly));
        const RFileHeader futureHeader(R_FILE_TYPE_MODEL,
            RVersion(RModel::version.getMajor(),RModel::version.getMinor(),
                     RModel::version.getRelease()+1));
        if (binary) RFileIO::writeBinary(output,futureHeader);
        else RFileIO::writeAscii(output,futureHeader);
        QVERIFY(output.commit());

        RModel existing = makeModel();
        existing.setName(QStringLiteral("preserved"));
        bool rejected = false;
        try
        {
            existing.read(fileName);
        }
        catch (const RError &error)
        {
            rejected = true;
            QVERIFY(error.getType() == RError::Type::InvalidFileFormat);
            QVERIFY(error.getMessage().contains(QStringLiteral("newer than supported version")));
        }
        QVERIFY(rejected);
        QCOMPARE(existing.getName(),QStringLiteral("preserved"));
    }
}

QTEST_APPLESS_MAIN(TestModelPhysicalIds)
#include "tst_model_physical_ids.moc"
